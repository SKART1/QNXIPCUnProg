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

int end=0;
struct sigevent event;


/*------------------------------------------------------------------------------------*/
const struct sigevent * intHandler(void *arg, int id){
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 1, "[INFO]: Entering interrupt handler handler");
	end=1;
	//std::cerr<<"Entering handler"<<std::endl;
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 1, "[INFO]: Exiting interrupt handler handler");
	return &event;
}
/*------------------------------------------------------------------------------------*/



void *Client(void *arg) {
	AboutServerInfoStruct aboutServerInfoStruct = *(AboutServerInfoStruct * )arg;
	delete (AboutServerInfoStruct *) arg;
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
	char buffer_read[]="This us write in pipe\0";
	char *buffPointer=&(buffer_write[0]);
	int len;
	int written;
	len=strlen(buffer_write);
	int temp;
	temp=0;
	int coid=-1;
	int rcvid=-1;

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
		if((aboutServerInfoStruct.fifoDes=open(aboutServerInfoStruct.pathToFifo, O_RDWR))<=0){
			perror("[ERROR]: Opening fifo file");
			break;
		}
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
		 if((aboutServerInfoStruct.messageQueueDescriptor=mq_open(aboutServerInfoStruct.pathToMessageQueue,  O_WRONLY  | O_CREAT, 0777, NULL))==-1){
			 perror("[ERROR]: Creating message queue: ");
		 }
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending message in message queue");
		 if(mq_send(aboutServerInfoStruct.messageQueueDescriptor, buffer_write, len, NULL)== -1){
			 perror("[ERROR]: Error receiving message from queue: ");
		 }
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending message in message queue");
		 if(mq_close(aboutServerInfoStruct.messageQueueDescriptor)==-1){
			 perror("[ERROR]: Closing message queue: ");
		 }
		 break;

	 case sharedMemoryIPC:
		 if((aboutServerInfoStruct.sharedMemoryId = shm_open(aboutServerInfoStruct.pathToSharedMemory, O_RDWR, NULL))==-1){
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
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before standard unnamed semaphore in client!");
			sem_wait(&aboutServerInfoStruct.semUnnamedStandart);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: In standard unnamed semaphore in client!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before standard post unnamed semaphore in client!");
			sem_post(&aboutServerInfoStruct.semUnnamedStandart);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After standard post unnamed semaphore in client!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
		};


		sem_post(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
		for(int i=0; i<1; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before shared memory unnamed semaphore in client!");
			sem_wait(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: In shared memory unnamed semaphore in client!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before shared memory post unnamed semaphore in client!");
			sem_post(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After shared memory post unnamed semaphore in client!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake100 = i * j;
				}
			}
		};
		break;

	case messageIPCRecieved_Block:
		if((coid=ConnectAttach(aboutServerInfoStruct.nd,aboutServerInfoStruct.pid,aboutServerInfoStruct.chid,NULL,NULL))==-1){
			perror("[ERROR]: ConenctAttach");
			return NULL;
		}
		sleep(1);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before sending message from client");
		if(MsgSend(coid, buffer_write, len,buffer_read, len)==-1){
			perror("[ERROR]: Message send");
			return NULL;
		}
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After sending message from client");
		break;

	case messageIPCSend_Block:
		if((coid=ConnectAttach(aboutServerInfoStruct.nd,aboutServerInfoStruct.pid,aboutServerInfoStruct.chid,NULL,NULL))==-1){
			perror("[ERROR]: ConenctAttach");
			return NULL;
		}
		sleep(1);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before sending message from client");
		if((rcvid=MsgReceive(coid, buffer_read, len, NULL))==-1){
			perror("[ERROR]: Message send");
			return NULL;
		}
		if(MsgReply(rcvid, NULL, buffer_write, len)==-1){
			perror("[ERROR]: Message reply");
			return NULL;
		}
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After sending message from client");
		break;

	case pulseIPCMessage:
	case pulseIPCSpecial:
		if((coid=ConnectAttach(aboutServerInfoStruct.nd,aboutServerInfoStruct.pid,aboutServerInfoStruct.chid,NULL,NULL))==-1){
			perror("[ERROR]: ConenctAttach");
			return NULL;
		}
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before sending pulse from client");
		if(MsgSendPulse (coid, -1, NULL, NULL)==-1){
			perror("[ERROR]: Send pulse");
			return NULL;
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After sending pulse from client");
		break;
	case pulseIPCFromInterruptHandler:
		if((aboutServerInfoStruct.coid=ConnectAttach(aboutServerInfoStruct.nd, aboutServerInfoStruct.pid ,aboutServerInfoStruct.chid, NULL, NULL))==-1){
			perror("[ERROR]: Connect attach");
			return NULL;
		}

		SIGEV_PULSE_INIT( &event, aboutServerInfoStruct.coid, -1 , NULL, NULL);
		if( ThreadCtl(_NTO_TCTL_IO, 0)==-1){
			perror("[ERROR]: setuid");
			return NULL;
		};

		if((aboutServerInfoStruct.intDescr= InterruptAttach( 1,intHandler, NULL, 0, 0| _NTO_INTR_FLAGS_TRK_MSK))==-1){
			perror("[ERROR]: InterruptAttach");
			return NULL;
		};

		while(end!=1);
		//InterruptWait(0,NULL);
		ConnectDetach(aboutServerInfoStruct.coid);
		InterruptDetach(aboutServerInfoStruct.intDescr);
		break;


	default:
		break;
	}

	return NULL;
}

