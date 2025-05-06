# Message Passing Library

This C++ library is to be used in embedded systems which has been created for 
the purpose of passing messages among threads.

## Features

- A queue for every thread, all statically held in a single array 
of length 254 defined by a MACRO.
- New meesage creation limited to 2048 to ensure memory safe code
- Comprehensive tests in `main()` encapsulating all edge cases
- Detailed API consisting of functions such as ``send()`` and `recv()`.

## Assumptions
- Each thread has a unique ID from `0` to `MAX_SIZE - 1`
- Messages are allocated dynamically but capped with a hard upper limit (`MAX_MSGS`)
- No `std::map` or `unordered_map` is used to ensure embedded-friendliness

## Design Tradeoffs
- Used *Dynamic Allocation* with ``new``/``delete`` for simplicty
- Used an individual lock instead of a global lock which may not be suitable for extremely small systems.

## Build Instructions 

To compile using `g++`:
```bash
g++ -std=c++17 -Wall -pthread message.cpp -o message_test
```
To execute, Run: 
```bash
./message_test
```
