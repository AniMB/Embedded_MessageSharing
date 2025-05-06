// Kepler_intro.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include<stdio.h>
# include<thread>
#include<queue>
# include<mutex>
#include <cstring>


#include "message.hpp"


/*
* --------------------------------------------------Assumption----------------------------------------------------------
*
*The assumption is that there is a queue for the messages for every thread. Same for locks.This will allow communication.
*
*Instead of using maps, arrays will be used. This is to only encourage static allocations.
* 
*/


/* 
* ---------------------------------------------Global Vars----------------------------------------------------------
*/

std::queue<message_t*> thread_queues[MAX_SIZE];
std::mutex locks[MAX_SIZE];
size_t created_message_counter{};




/*
----------------------------------------------CODE--------------------------------------------------------------
*/

/**
 * @brief Allocates a new message struct for the calling thread.
 * 
 * Threads call this function to obtain a fresh message structure from the message library.
 * If the message pool exceeds the maximum allowed (MAX_MSGS), or if memory allocation fails, 
 * the function returns nullptr.
 * 
 * @return message_t* on success; nullptr on failure.
 */
message_t* new_message() {
    try {
        if (created_message_counter < MAX_MSGS) {
            auto* m = new message_t;
            created_message_counter++;
            std::memset(m, 0, sizeof(message_t));
            return m;
        } else {
            return nullptr;
        }
    } catch (const std::bad_alloc&) {
        return nullptr;                      
    }
}

/**
 * @brief Frees a message struct and returns it back to the library.
 * 
 * This function is used by threads to indicate they are done using a message structure.
 * It deletes the memory and decrements the counter of created messages.
 * 
 * @param msg Pointer to the message struct to be deleted.
 * 
 * @return void
 */
void delete_message(message_t* msg) {
    delete msg;
    created_message_counter--;
}

/**
 * @brief Sends a message to the specified thread ID.
 * 
 * Threads call this function to deliver a message to another thread via its ID.
 * The message is enqueued into the destination thread's message queue.
 * 
 * @param destination_id ID of the thread to receive the message.
 * @param msg Pointer to the message to be sent.
 * 
 * @return int 0 on success; -1 on error 
 */
int send(uint8_t destination_id, message_t* msg) {
    if (!msg || destination_id >= MAX_SIZE) { return -1; }
    try {
        std::lock_guard<std::mutex> l(locks[destination_id]);
        thread_queues[destination_id].push(msg);
        return 0;
    } catch (...) {
        return -1;
    }
}

/**
 * @brief Receives a pending incoming message for the given thread ID.
 * 
 * Threads call this function to check their message queue and retrieve any available message.
 * The function will set the provided msg pointer to the contents of the received message.
 * 
 * @param receiver_id The ID of the thread wishing to receive a message.
 * @param msg Output pointer to a message_t structure to populate with received data.
 * 
 * @return int 0 on success; -1 on error 
 */
int recv(uint8_t receiver_id, message_t* msg) {
    if (!msg || receiver_id >= MAX_SIZE) { return -1; }
    try {
        std::lock_guard<std::mutex> l(locks[receiver_id]);
        if (thread_queues[receiver_id].empty()) { return -1; }
        *msg = *thread_queues[receiver_id].front();
        delete_message(thread_queues[receiver_id].front());
        thread_queues[receiver_id].pop();
        return 0;
    } catch (...) {
        return -1;
    }
}


/*

----------------------------------------------TESTING-------------------------------------------------------------------

*/

/**
 * @brief Test 1: Basic Send and Receive
 *
 * Tests that a message can be sent and then successfully received by the same thread ID.
 */
void test_basic_send_receive() {
    std::cout << "==== Test 1: Basic Send/Receive ====\n";

    message_t* msg = new_message();
    msg->len = 3;
    msg->data[0] = 'A';
    msg->data[1] = 'B';
    msg->data[2] = 'C';

    if (send(0, msg) != 0) std::cerr << "[FAIL] send failed\n";

    message_t result;
    if (recv(0, &result) == 0) {
        std::cout << "[PASS] Basic Receive: ";
        for (int i = 0; i < result.len; ++i) std::cout << result.data[i];
        std::cout << "\n";
    }
    else {
        std::cout << "[FAIL] recv failed\n";
    }
}

/**
 * @brief Test 2: Empty Queue Receive
 *
 * Tests that receiving from an empty message queue fails as expected.
 */
void test_recv_empty() {
    std::cout << "==== Test 2: Receive on Empty Queue ====\n";

    message_t result;
    if (recv(1, &result) != 0) {
        std::cout << "[PASS] Correctly returned error when no message available\n";
    }
    else {
        std::cout << "[FAIL] Received a message when queue was empty\n";
    }
}

/**
 * @brief Test 3: FIFO Order Verification
 *
 * Tests that messages sent to a thread are received in the same order (FIFO).
 */
void test_fifo_order() {
    std::cout << "==== Test 3: FIFO Message Order ====\n";

    char base = 'X';
    for (int i = 0; i < 3; ++i) {
        message_t* m = new_message();
        m->len = 1;
        m->data[0] = base + i;
        send(2, m);
    }

    for (int i = 0; i < 3; ++i) {
        message_t result;
        if (recv(2, &result) == 0 && result.data[0] == (base + i)) {
            std::cout << "[PASS] FIFO " << result.data[0] << '\n';
        }
        else {
            std::cout << "[FAIL] FIFO order broken\n";
        }
    }
}

/**
 * @brief Test 4: Exhaust Message Pool
 *
 * Tests that the system enforces a cap on total allocated messages and refuses allocation beyond MAX_MSGS.
 */
void test_pool_exhaustion() {
    std::cout << "==== Test 4: Message Pool Exhaustion ====\n";

    std::vector<message_t*> allocated;
    for (int i = 0; i < MAX_MSGS; ++i) {
        message_t* m = new_message();
        if (!m) {
            std::cerr << "[FAIL] Pool exhausted too early at " << i << "\n";
            break;
        }
        allocated.push_back(m);
    }

    message_t* fail_msg = new_message();
    if (!fail_msg) {
        std::cout << "[PASS] Pool correctly exhausted at MAX_MSGS\n";
    }
    else {
        std::cout << "[FAIL] Pool over-allocated beyond MAX_MSGS\n";
        delete_message(fail_msg);
    }

    for (auto* m : allocated) delete_message(m);
}

/**
 * @brief Test 5: Invalid Thread ID
 *
 * Tests that sending a message to an invalid (out-of-bounds) thread ID fails.
 */
void test_invalid_thread_id() {
    std::cout << "==== Test 5: Invalid Thread ID ====\n";

    message_t* m = new_message();
    m->len = 1;
    m->data[0] = 'E';

    if (send(MAX_SIZE + 1, m) != 0) {
        std::cout << "[PASS] send rejected invalid thread ID\n";
        delete_message(m);
    }
    else {
        std::cout << "[FAIL] send accepted invalid thread ID\n";
    }
}

/**
 * @brief Main function to run all message-passing test cases.
 *
 * Executes a series of tests to validate the correctness and robustness of the message passing library.
 *
 * @return int 0 on success.
 */
int main() {
    std::cout << "\n\n=== Running Message Passing Library Tests ===\n";
    test_basic_send_receive();
    test_recv_empty();
    test_fifo_order();
    test_pool_exhaustion();
    test_invalid_thread_id();
    std::cout << "\n=== Tests Complete ===\n";
    return 0;
}


