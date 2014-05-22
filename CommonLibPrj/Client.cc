/*
 * Client.cc
 *
 *  Created on: 28.04.2014
 *      Author: Art
 */
#include "Client.hpp"
#include "Headers.hpp"
#include "CommonLibPrj.hpp"
#include "AboutServerInfoStruct.hpp"
#include "DebugInfoOut.hpp"

volatile int fake100;



void *Client(void *arg) {
	AboutServerInfoStruct aboutServerInfoStruct = *(AboutServerInfoStruct * )arg;
	DEBUG_PRINT("INFO", "In client!");

	std::cout<<"Node name: "<<aboutServerInfoStruct.serverNodeName<<std::endl;
	aboutServerInfoStruct.nd=netmgr_strtond(aboutServerInfoStruct.serverNodeName, NULL);
	std::cout<<"Its number is: "<<aboutServerInfoStruct.nd<<std::endl;

	//std::cout<<"In client!"<<std::endl;
	//FIFO

	//Output file pointer







	//pid_t pid = aboutServerInfoStruct.pid;
	//pid_t ppid= aboutServerInfoStruct.ppid;
	//pthread_t tid =aboutServerInfoStruct.tid;


	//int fileDes[2];
	//fileDes[1]= aboutServerInfoStruct.fileDes[1];
	//fileDes[2]= aboutServerInfoStruct.fileDes[2];



	//int	buffer_pipe_writeLength=fscanf(filePointer, "%d");

	//fprintf(filePointer, "BufferLength: %d\n", strlen(buffer_pipe_write));
	//char buffer_pipe_write[] = "This us write in pipe";
	char buffer_write[]="This us write in pipe\0";
	char *buffPointer=&(buffer_write[0]);
	int len;
	int written;

	 int temp;
	 temp=0;

	switch ((aboutServerInfoStruct.IPCTypeSelector)) {
	case signalIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending SIG1");
		kill(aboutServerInfoStruct.pid, SIGUSR1);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending  SIG1");
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending SIG2");
		SignalKill(aboutServerInfoStruct.nd, aboutServerInfoStruct.pid, aboutServerInfoStruct.tid, SIGUSR2, NULL, NULL);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending SIG2");
		break;

	case pipeIPC:
		len=strlen(buffer_write);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before writing in pipe");
		while(len!=0 && ((written=write(aboutServerInfoStruct.fileDes[1],buffPointer, len)) !=0)){
			if(written==-1){
				if(errno==EINTR){
					continue;
				}
				perror("[ERROR]: Write in pipe");
				break;
			}
			len-=written;
			buffPointer=buffPointer+written;
		}
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in pipe");
		sleep(1); //Let the pipe be readed
		close(aboutServerInfoStruct.fileDes[0]);
		close(aboutServerInfoStruct.fileDes[1]);
		break;

	case fifoIPC:
		std::cout<<aboutServerInfoStruct.pathToFifo<<std::endl;
		if(
				(aboutServerInfoStruct.fifoDes=open(aboutServerInfoStruct.pathToFifo, O_RDWR))<=0
				){
				perror("[ERROR]: Opening fifo file");
				break;
		}
		len=strlen(buffer_write);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before writing in fifo");
		while(len!=0 && ((written=write(aboutServerInfoStruct.fifoDes,buffer_write, len)) !=0)){
			if(written==-1){
				if(errno==EINTR){
					continue;
				}
				perror("[ERROR]: Write in fifo");
				break;
			}
			len-=written;
			buffPointer=buffPointer+written;
		}
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in fifo");
		close(aboutServerInfoStruct.fifoDes);
		break;

	case messageQueuIPC:
		/* if((aboutServerInfoStruct->messageQueueDescriptor=mq_open(aboutServerInfoStruct->pathToMessageQueue.c_str(), O_CREAT, 0777, NULL)==-1)){
					 perror("[ERROR]: Creating message queue: ");
		 }*/
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending message in message queue");
		/* if(mq_send(aboutServerInfoStruct.messageQueueDescriptor, buffer_read, len, NULL)== -1){
			 perror("[ERROR]: Error receiving message from queue: ");
		 }*/
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending message in message queue");
				 if(mq_close(aboutServerInfoStruct.messageQueueDescriptor)==-1){
					 perror("[ERROR]: Closing message queue: ");
				 }
				 if(mq_unlink(aboutServerInfoStruct.pathToMessageQueue)==-1){
					 perror("[ERROR]: Unlinking message queue: ");
				 }
		break;

	 case sharedMemoryIPC:
		 if((aboutServerInfoStruct.sharedMemoryId = shm_open(aboutServerInfoStruct.pathToSharedMemory.c_str(), O_RDWR, NULL))==-1){
			 perror("[ERROR]: Shared memory open in client: ");
		 }
		 if((aboutServerInfoStruct.sharedMemoryAddrInProcessSpace = mmap(NULL, 2*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, aboutServerInfoStruct.sharedMemoryId, 0 ))==MAP_FAILED){
			perror("[ERROR]: Maping shared memory: ");
		 }
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before reading from shared memory");
		 temp=*((int *)aboutServerInfoStruct.sharedMemoryAddrInProcessSpace);
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After reading from shared memory");

		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before writing in shared memory");
		 *((int *)(aboutServerInfoStruct.sharedMemoryAddrInProcessSpace)+1)=1;
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in shared memory");



		 if(munmap(aboutServerInfoStruct.sharedMemoryAddrInProcessSpace ,2*sizeof(int))==-1){
			 perror("[ERROR]: Unmapping shared memory: ");
		 };
		 break;

	case semaphoreIPCUnnamed:
		sem_post(&aboutServerInfoStruct.semUnnamedStandart);
		for(int i=0; i<1; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before standard unnamed semaphore in client!\n");
			sem_wait(&aboutServerInfoStruct.semUnnamedStandart);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: In standard unnamed semaphore in client!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before standard post unnamed semaphore in client!\n");
			sem_post(&aboutServerInfoStruct.semUnnamedStandart);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After standard post unnamed semaphore in client!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
		};


		sem_post(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
		for(int i=0; i<1; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before shared memory unnamed semaphore in client!\n");
			sem_wait(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: In shared memory unnamed semaphore in client!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before shared memory post unnamed semaphore in client!\n");
			sem_post(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After shared memory post unnamed semaphore in client!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
		};
		break;


	case messageIPCSend_Block:

		break;
	case messageIPCRecieved_Block:

		break;
	case pulseIPCMessage:

		break;
	case pulseIPCSpecial:

		break;
	case pulseIPCFromInterruptHandler:

		break;


	default:
		break;
	}

	return NULL;
}

