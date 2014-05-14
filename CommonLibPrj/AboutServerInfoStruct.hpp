/*
 * AboutServerInfoStruct.hpp
 *
 *  Created on: 10.05.2014
 *      Author: Art
 */

#ifndef ABOUTSERVERINFOSTRUCT_HPP_
#define ABOUTSERVERINFOSTRUCT_HPP_
#include "Headers.hpp"
#include <semaphore.h>

typedef struct{
	//Type of IPC
	IPCType IPCTypeSelector;
	participantsType participantsTypeSelector;

	//Info about server
	unsigned int nd;
	pid_t pid;
	pid_t ppid;
	pthread_t tid;

	//PIPE
	int fileDes[2];

	//FIFO
	std::string pathToFifo;
	int fifoDes;

	//SharedMemory
	//std::string pathToSharedMemory;
	//int fifoDes;

	//Semaphore
	sem_t semUnnamedStandart;
	sem_t semUnnamedThroughSharedMemory;

	sem_t *semNamed;
	std::string pathToSemNamedStandart;




	//Messages
	int chid;

	//Path to file with server info
	std::string pathToFileWithServerInfo;

} AboutServerInfoStruct;


#endif /* ABOUTSERVERINFOSTRUCT_HPP_ */
