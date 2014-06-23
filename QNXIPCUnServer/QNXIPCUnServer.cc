#include "../CommonLibPrj/CommonFunctions.hpp"
#include "../CommonLibPrj/Client.hpp"
#include "../CommonLibPrj/AboutServerInfoStruct.hpp"
#include "../CommonLibPrj/DebugInfoOut.hpp"

#include "QNXIPCUnServer.hpp"



extern int errno;

//#define DEBUG_MY


volatile int sig1 = 0;
int sig2 = 0;
int sig3 = 0;

volatile int fake1 = 0;
volatile int fake2 = 0;



/*------------------------------------------------------------------------------------*/
static void sigusr1_hndlr(int signo) {
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 1, "[INFO]: Entering SIGUSR1");
#ifdef DEBUG_MY
	std::cout << "[INFO]: In SIGUSR1" << std::endl;
#endif

	for (int i = 0; i < 10000; i++) {
		for (int j = 0; j < 100; j++) {
			fake1 = i * j;
		}
	}

	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 2, "[INFO]: Exiting SIGUSR1");
}
/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
static void sigusr2_hndlr(int signo) {
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Entering SIGUSR2");
#ifdef DEBUG_MY
	std::cout << "[INFO]: In SIGUSR2" << std::endl;
#endif

	for (int i = 0; i < 10000; i++) {
		for (int j = 0; j < 100; j++) {
			fake2 = i * j;
		}
	}
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 4, "[INFO]: Exiting SIGUSR2");
}
/*------------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------------*/
int inline infoToFile(AboutServerInfoStruct aboutServerInfoStruct , char *buffer_pipe_write ){
	//Output file pointer
	FILE *filePointer = NULL;

#ifdef DEBUG_MY
	std::cout << "[INFO]: Writing information to info.info" << std::endl;
#endif
	filePointer = fopen((aboutServerInfoStruct.pathToFileWithServerInfo), "w");
	if (filePointer == NULL) {
		printf("[ERROR]: %d can not create file with program information because of: %s\n",errno, strerror(errno));
		return -1;
	}
	fprintf(filePointer, "NodeName: %s\n", aboutServerInfoStruct.serverNodeName);
	fprintf(filePointer, "PID: %d\n", aboutServerInfoStruct.pid);
	fprintf(filePointer, "PPID: %d\n", aboutServerInfoStruct.ppid);
	fprintf(filePointer, "TID: %d\n", aboutServerInfoStruct.tid);

	//For pipe
	fprintf(filePointer, "PIPE[0]: %d\n", aboutServerInfoStruct.fileDes[0]);
	fprintf(filePointer, "PIPE[1]: %d\n", aboutServerInfoStruct.fileDes[1]);

	//For fifo
	fprintf(filePointer, "FIFO_PATH: %s\n", aboutServerInfoStruct.pathToFifo);


	//For message
	fprintf(filePointer, "CHID: %d\n", aboutServerInfoStruct.chid);



	fclose(filePointer);
	return 0;
}
/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
int inline preWork(AboutServerInfoStruct *aboutServerInfoStruct){
#ifdef DEBUG_MY
	std::cout << "[INFO]: Preparing server part of IPC" << std::endl;
#endif

	switch (aboutServerInfoStruct->IPCTypeSelector) {
	case signalIPC:
		//!POSIX
		struct sigaction act;
		act.sa_handler = &sigusr1_hndlr;
		act.sa_flags = 0;
		if (sigaction(SIGUSR1, &act, 0) < 0) {
			printf("[ERROR]: %d registering POSIX signal handler. That means: %s\n", errno, strerror(errno));
			return -7;
		}

		act.sa_handler = &sigusr2_hndlr;
		//!MicroKenel Adding sigusr2_hndlr to SIGUSR2 for this process (first parameter)
		if (SignalAction(0, __signalstub, SIGUSR2, &act, NULL) == -1) {
			printf("[ERROR]: %d registering Microkernel signal handler. That means: %s\n", errno, strerror(errno));
			return -8;
		}
		break;

	case pipeIPC:
		if (pipe(aboutServerInfoStruct->fileDes) < 0) {
			printf("[ERROR]: %d creating pipe file. That means: %s\n", errno,strerror(errno));
			return -9;
		}
		break;

	case fifoIPC:
		unlink(aboutServerInfoStruct->pathToFifo);
		if ((mkfifo((aboutServerInfoStruct->pathToFifo), S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
			printf("[ERROR]: %d creating fifo file. That means: %s\n", errno,strerror(errno));
			return -10;
		}
		if(((aboutServerInfoStruct->fifoDes)=open(aboutServerInfoStruct->pathToFifo, O_RDWR))<=0){
			printf("[ERROR]: %d opening fifo file. That means: %s\n", errno,strerror(errno));
			return -11;
		}
		break;

	 case messageQueuIPC:
		 if((aboutServerInfoStruct->messageQueueDescriptor=mq_open(aboutServerInfoStruct->pathToMessageQueue, O_RDONLY| O_CREAT, 0777, NULL))==-1){
			 perror("[ERROR]: Creating message queue: ");
		 }
		 break;

	 case sharedMemoryIPC:
		if((aboutServerInfoStruct->sharedMemoryId = shm_open(aboutServerInfoStruct->pathToSharedMemory, O_RDWR | O_CREAT, 0777))==-1){
			perror("[ERROR]: Shared memory initialization: ");
			//return -1;
		}
		/* Set the memory object's size */
		if( ftruncate(aboutServerInfoStruct->sharedMemoryId, 2*sizeof(int)) == -1 ) {
			perror("[ERROR]: Shared memory setting size: ");
			//return -1;
		}

		if((aboutServerInfoStruct->sharedMemoryAddrInProcessSpace = mmap(NULL, 2*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, aboutServerInfoStruct->sharedMemoryId, 0 ))==MAP_FAILED){
			perror("[ERROR]: Maping shared memory: ");
		}
		*((int *)aboutServerInfoStruct->sharedMemoryAddrInProcessSpace)=0;
		break;

	case semaphoreIPCUnnamed:
		if(sem_init( &aboutServerInfoStruct->semUnnamedStandart, NULL, 0 )== -1){
			perror("[ERROR]: Unnamed standard semaphore init: ");
		};
		if(sem_init( &aboutServerInfoStruct->semUnnamedThroughSharedMemory, 1, 0 )== -1){
			perror("[ERROR]: Unnamed shared memory semaphore init: ");
		};
		break;

	case semaphoreIPCNamed:
		if(((aboutServerInfoStruct->semNamed)=sem_open(aboutServerInfoStruct->pathToSemNamedStandart,O_CREAT | O_EXCL, S_IRWXG | S_IRWXO | S_IRWXU, 0))==SEM_FAILED){
			perror("[ERROR]: Named semaphore init: ");
		};
		break;

	case messageIPCRecieved_Block:
	case messageIPCSend_Block:
	case pulseIPCMessage:
	case pulseIPCSpecial:
		if((aboutServerInfoStruct->chid= ChannelCreate(NULL))==-1){
			perror("[ERROR]: Channel create");
			return -12;
		}
		break;
	case pulseIPCFromInterruptHandler:
		if((aboutServerInfoStruct->chid= ChannelCreate(NULL))==-1){
			perror("[ERROR]: Channel create");
			return -12;
		}
		break;
	default:
		break;
	}
	return 0;
}
/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
/*Creating thread/process
 * If we are separate server - just ignore this part*/
int makeThreadProcess(char *argv[], AboutServerInfoStruct aboutServerInfoStruct){
#ifdef DEBUG_MY
	std::cout << "[INFO]: Starting client if it necessary" << std::endl;
#endif
	pthread_t t1;
	int pidSon;
	std::string independentProcessLocalNameAndParametrs="QNXIPCUnClient";

	switch (aboutServerInfoStruct.participantsTypeSelector) {
	case oneProcessThreads:
		pthread_attr_t threadAttr;
		pthread_attr_init(&threadAttr);
		pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
		AboutServerInfoStruct *aboutServerInfoStructTemp;
		aboutServerInfoStructTemp=new AboutServerInfoStruct;
		*aboutServerInfoStructTemp=aboutServerInfoStruct;
		pthread_create(&t1, &threadAttr, Client,(void *)aboutServerInfoStructTemp);
		break;
	case relatedProcess:
		if ((pidSon = fork()) == 0) {
			if (execv("QNXIPCUnClient_g", argv) == -1) {
				printf("[ERROR]: %d error running client process. That means: %s\n", errno,	strerror(errno));
			}
		};
		break;
	/*case independentProcessLocal:
		for(int i=0; i<argc; i++){
			independentProcessLocalNameAndParametrs=independentProcessLocalNameAndParametrs+" "+std::string(argv[i]);
		}
		std::cerr<<independentProcessLocalNameAndParametrs<<std::endl;
		break;*/
	default:
		break;
	}
	return 0;
}
/*------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------*/
/*Waiting for something (Server part)
 * */
int recievingPart(AboutServerInfoStruct aboutServerInfoStruct, char* buffer_read, char *buffer_write){
#ifdef DEBUG_MY
	std::cout << "[INFO]: Starting client if it necessary" << std::endl;
#endif
	char *buffer_readPointer=buffer_read;
	int len;
	len=strlen(buffer_write);
	int ret;
	ret=0;

	//Messages
	int rcvid=-1;
	std::string temp2;
	int temp=0;

	char pulseRecieved[100];

	switch (aboutServerInfoStruct.IPCTypeSelector) {
	case signalIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before pause. Waiting for signal!");
		for (;;) {
			pause();
		}
		break;

	case pipeIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 1,"[INFO]: Before read from pipe!");
		while (len>0 && (ret=read(aboutServerInfoStruct.fileDes[0], buffer_read,len ))!= 0) {
			if(ret==-1){
				if(errno == EINTR){
					continue;
				}
				DEBUG_PRINT("ERROR","Internal read error");
				break;
			}
			else if(ret!=0){
				DEBUG_PRINT("INFO",buffer_read);
				TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 1, "[INFO]: Before read from pipe!");
				len-=ret;
				buffer_read+=ret;
			}
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 1,"[INFO]: Read from pipe is maiden!");
		close(aboutServerInfoStruct.fileDes[0]);
		close(aboutServerInfoStruct.fileDes[1]);
		break;

	case fifoIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before read from fifo!");
		while (len>0 && (ret=read(aboutServerInfoStruct.fifoDes, buffer_readPointer,len ))!= 0) {
			if(ret==-1){
				if(errno == EINTR){
					continue;
				}
				else if(errno == EAGAIN){
					continue;
				}
				DEBUG_PRINT("ERROR","Internal read error");
				break;
			}
			else if(ret!=0){
				DEBUG_PRINT("INFO",buffer_read);
				TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Read from fifo is maiden!");
				len-=ret;
				buffer_readPointer+=ret;
			}
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Reading from fifo ended!");
		close(aboutServerInfoStruct.fifoDes);
		unlink(aboutServerInfoStruct.pathToFifo);
		break;


	 case messageQueuIPC:

		 struct mq_attr obuf;
		 char *msgPointer;
		 msgPointer=NULL;
		 /* get max message size */
		 if (!mq_getattr(aboutServerInfoStruct.messageQueueDescriptor,&obuf)){
			 msgPointer = (char* )calloc(1,obuf.mq_msgsize);
		 }
		 else{
			perror("[ERROR]: Get message queue attributes: ");
		 }

		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before receiving message in message queue");
		 if(mq_receive(aboutServerInfoStruct.messageQueueDescriptor, msgPointer, obuf.mq_msgsize, NULL)== -1){
			perror("[ERROR]: Error receiving message from queue: ");
		 }
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After receiving message in message queue");

		 if(mq_close(aboutServerInfoStruct.messageQueueDescriptor)==-1){
			perror("[ERROR]: Closing message queue: ");
		 }
		 if(mq_unlink(aboutServerInfoStruct.pathToMessageQueue)==-1){
			perror("[ERROR]: Unlinking message queue: ");
		 }
		 free(msgPointer);
		 break;

	 case sharedMemoryIPC:
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before writing in shared memory");
		 *(((int *)aboutServerInfoStruct.sharedMemoryAddrInProcessSpace)+1)=1;
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in shared memory");


		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before reading from shared memory");
		 temp=*((int *)aboutServerInfoStruct.sharedMemoryAddrInProcessSpace);
		 TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After reading from shared memory");

		 while(*((int *)aboutServerInfoStruct.sharedMemoryAddrInProcessSpace)!=1);
		 if(shm_unlink(aboutServerInfoStruct.pathToSharedMemory)==-1){
			 perror("[ERROR]: Shared memory unlinking: ");
		 };
		 break;


	case semaphoreIPCUnnamed:
		for(int i=0; i<2; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before unnamed semaphore!");
			if(sem_wait(&aboutServerInfoStruct.semUnnamedStandart)==-1){
				 perror("[ERROR]: sem_wait unnamed semaphore standard in server: ");
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: In unnamed semaphore!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before post unnamed semaphore!");
			if(sem_post(&aboutServerInfoStruct.semUnnamedStandart)==-1){
				perror("[ERROR]: sem_post unnamed semaphore  standard in server: ");
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After post unnamed semaphore!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
		};

		for(int i=0; i<2; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before shared memory unnamed semaphore in server!");
			if(sem_wait(&aboutServerInfoStruct.semUnnamedThroughSharedMemory)==-1){
				perror("[ERROR]: sem_wait unnamed semaphore shared memory in server: ");
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: In shared memory unnamed semaphore in server!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before shared memory post unnamed semaphore in server!");
			if(sem_post(&aboutServerInfoStruct.semUnnamedThroughSharedMemory)==-1){
				perror("[ERROR]: sem_post unnamed semaphore shared memory in server: ");
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After shared memory post unnamed semaphore in server!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
		};
		sem_destroy(&aboutServerInfoStruct.semUnnamedStandart);
		sem_destroy(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
		break;

	case semaphoreIPCNamed:
		for(int i=0; i<2; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before named semaphore in server!");
			if(sem_wait(aboutServerInfoStruct.semNamed)==-1){
				perror("[ERROR]: sem_wait named semaphore in server: ");
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: In named semaphore in server!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before post named semaphore in server!");
			if(sem_post(aboutServerInfoStruct.semNamed)==-1){
				perror("[ERROR]: sem_post named semaphore in server: ");
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After post named semaphore in server!");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
		};

		sem_close(aboutServerInfoStruct.semNamed);
		break;


	case messageIPCRecieved_Block:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before receive message!");
		if((rcvid =MsgReceive(aboutServerInfoStruct.chid, &buffer_read, len, NULL))<=0){
			if(rcvid==0){
				DEBUG_PRINT("INFO", "Pulse! WTF?!");
				break;
			}
			else{
				perror("[ERROR]: MsgReceive");
			}
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After receive message!");

		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before reply message!");
		if(MsgReply(rcvid,EOK,&buffer_write, len)==-1){
			perror("[ERROR]: MsgReply");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After reply message!");
		break;



	case messageIPCSend_Block:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before send message!");
		if(MsgSend(aboutServerInfoStruct.chid,&buffer_write, len,&buffer_read, len )==-1){
			perror("[ERROR]: MsgSend");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After send message!");
		break;

		break;


	case pulseIPCMessage:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before receive pulse!");
		if((rcvid =MsgReceive(aboutServerInfoStruct.chid, &buffer_read, len, NULL))<=0){
			if(rcvid==0){
				TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After receiving pulse!");
				DEBUG_PRINT("INFO", "Pulse!");
				break;
			}
			else{
				perror("[ERROR]: MsgReceive");
			}
		};
		break;

	case pulseIPCSpecial:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before send message!");
		if(MsgReceivePulse(aboutServerInfoStruct.chid, pulseRecieved, sizeof(pulseRecieved),NULL)){
				perror("[ERROR]: MsgSend");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After send message!");
		break;

	case pulseIPCFromInterruptHandler:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: Before receiving pulse!");
		if(MsgReceivePulse(aboutServerInfoStruct.chid, pulseRecieved, sizeof(pulseRecieved),NULL)){
				std::cout<<errno<<std::endl;
				perror("[ERROR]: MsgSend");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3,"[INFO]: After receiving pulse!");
		break;
	default:
		break;
	}
	return 0;
}
/*------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------*/
int getInfgoAboutServer(AboutServerInfoStruct *aboutServerInfoStruct){
	aboutServerInfoStruct->pid=getpid();
	aboutServerInfoStruct->ppid=getppid();
	aboutServerInfoStruct->tid=pthread_self();
	if(netmgr_ndtostr(ND2S_DIR_SHOW, 0,  aboutServerInfoStruct->serverNodeName,100)==-1){
		perror("[ERROR]: Node name to node number");
		return -1;
	};
	return 0;
}
/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
 int main(int argc, char *argv[]) {
 	AboutServerInfoStruct aboutServerInfoStruct;
 	strcpy(aboutServerInfoStruct.pathToFifo,"test");

	parseParametrsMy(argc, argv, &aboutServerInfoStruct); //Parsing input parametrs
	getInfgoAboutServer(&aboutServerInfoStruct);


	//Buffer
	char buffer_pipe_write[]="This us write in pipe";
	char buffer_pipe_read[strlen(buffer_pipe_write)];

	preWork(&aboutServerInfoStruct);
	if(infoToFile(aboutServerInfoStruct, buffer_pipe_write )==-1){
			//goto deInit;
	};







	makeThreadProcess(argv, aboutServerInfoStruct);

	recievingPart(aboutServerInfoStruct, buffer_pipe_read,buffer_pipe_write);

	//printf("\n PROCESS PARAM: pid=%i  ppid=%i \n", pid, ppid);



	for (;;) {
		pause();
	}

	deInit:
	//fclose(filePointer);

	return 0;
}
 /*------------------------------------------------------------------------------------*/
