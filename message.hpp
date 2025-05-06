#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>

/* MACRO DEFINITIONS*/
constexpr auto MAX_SIZE = 254;
constexpr auto MAX_MSGS = 2048;

/* STRUCTS*/


/**
* @brief Struct message_t contains two params 
* 
* @param len: uint8_t type element
* 
* @param data[255]: static array of type uint8_t
* 
*/
struct message_t {
    uint8_t len;
    uint8_t data[255];
};

/* Function Declarations */
message_t* new_message();
void       delete_message(message_t* msg);
int        send(uint8_t destination_id, message_t* msg);
int        recv(uint8_t receiver_id, message_t* msg);

#endif 