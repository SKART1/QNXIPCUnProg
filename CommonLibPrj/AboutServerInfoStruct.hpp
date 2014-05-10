/*
 * AboutServerInfoStruct.hpp
 *
 *  Created on: 10.05.2014
 *      Author: Art
 */

#ifndef ABOUTSERVERINFOSTRUCT_HPP_
#define ABOUTSERVERINFOSTRUCT_HPP_
#include "Headers.hpp"

typedef struct{
	//Type of IPC
	IPCType IPCTypeSelector;
	participantsType participantsTypeSelector;

	//Info about server
	pid_t pid;
	pid_t ppid;
	pthread_t tid;

	//PIPE
	int fileDes[2];

	//FIFO
	std::string pathToFifo;
	int fifoDes;

	//Path to file with server info
	std::string pathToFileWithServerInfo;

} AboutServerInfoStruct;


#endif /* ABOUTSERVERINFOSTRUCT_HPP_ */
