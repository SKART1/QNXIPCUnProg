/*
 * Client.hpp
 *
 *  Created on: 28.04.2014
 *      Author: Art
 */

#ifndef CLIENT_HPP_
#define CLIENT_HPP_
#include <fcntl.h>	   //for shared memory
#include <sys/mman.h> //for shared memory

void *Client(void *t);


#endif /* CLIENT_HPP_ */
