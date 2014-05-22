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
#include <mqueue.h>
#include <fcntl.h>

typedef struct{
	//Type of IPC
	IPCType IPCTypeSelector;
	participantsType participantsTypeSelector;

	//Info about server
	unsigned int nd;
	char serverNodeName[150];
	pid_t pid;
	pid_t ppid;
	pthread_t tid;

	//PIPE
	int fileDes[2];

	//FIFO
	char pathToFifo[150];
	int fifoDes;

	//SharedMemory
	std::string pathToSharedMemory;
	int sharedMemoryId;
	void * sharedMemoryAddrInProcessSpace;

	//Message queue
	mqd_t messageQueueDescriptor;
	char pathToMessageQueue[150];

	//Semaphore
	sem_t semUnnamedStandart;
	sem_t semUnnamedThroughSharedMemory;

	sem_t *semNamed;
	char pathToSemNamedStandart[150];




	//Messages
	int chid;

	//Path to file with server info
	char pathToFileWithServerInfo[150];

} AboutServerInfoStruct;


#endif /* ABOUTSERVERINFOSTRUCT_HPP_ */
