/*
 * QNXIPCUnServer.hpp
 *
 *  Created on: 10.05.2014
 *      Author: Art
 */

#ifndef QNXIPCUNSERVER_HPP_
#define QNXIPCUNSERVER_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include <errno.h>
#include <string.h>
#include <string>

#include <sys/neutrino.h>
#include <sys/trace.h>

#include <pthread.h>
#include <semaphore.h>


#include <fcntl.h>	   //for shared memory
#include <sys/mman.h> //for shared memory

#include <sys/netmgr.h> //for node name resolving
#endif /* QNXIPCUNSERVER_HPP_ */
