#include "../CommonLibPrj/CommonFunctions.hpp"
#include "../CommonLibPrj/Client.hpp"
#include "../CommonLibPrj/AboutServerInfoStruct.hpp"
#include "../CommonLibPrj/DebugInfoOut.hpp"

#include "QNXIPCUnServer.hpp"



extern int errno;

#define DEBUG_PRINT_MY
#define DEBUG_MY


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
	filePointer = fopen((aboutServerInfoStruct.pathToFileWithServerInfo).c_str(), "w");
	if (filePointer == NULL) {
		printf("[ERROR]: %d can not create file with program information because of: %s\n",errno, strerror(errno));
		return -1;
	}

	fprintf(filePointer, "ND: %d\n", aboutServerInfoStruct.nd);
	fprintf(filePointer, "PID: %d\n", aboutServerInfoStruct.pid);
	fprintf(filePointer, "PPID: %d\n", aboutServerInfoStruct.ppid);
	fprintf(filePointer, "TID: %d\n", aboutServerInfoStruct.tid);

	//For pipe
	fprintf(filePointer, "PIPE[0]: %d\n", aboutServerInfoStruct.fileDes[0]);
	fprintf(filePointer, "PIPE[1]: %d\n", aboutServerInfoStruct.fileDes[1]);

	//For fifo
	fprintf(filePointer, "FIFO_PATH: %s\n", aboutServerInfoStruct.pathToFifo.c_str());


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
		unlink(aboutServerInfoStruct->pathToFifo.c_str());
		if ((mkfifo((aboutServerInfoStruct->pathToFifo).c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
			printf("[ERROR]: %d creating fifo file. That means: %s\n", errno,strerror(errno));
			return -10;
		}
		if(((aboutServerInfoStruct->fifoDes)=open((aboutServerInfoStruct->pathToFifo).c_str(), O_RDWR))<=0){
			printf("[ERROR]: %d opening fifo file. That means: %s\n", errno,strerror(errno));
			return -11;
		}

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
		if(((aboutServerInfoStruct->semNamed)=sem_open(aboutServerInfoStruct->pathToSemNamedStandart.c_str(),O_CREAT | O_EXCL, S_IRWXG | S_IRWXO | S_IRWXU, 0))==SEM_FAILED){
			perror("[ERROR]: Named semaphore init: ");
		};
		break;

	case messageIPCSend_Block:
	case messageIPCRecieved_Block:
	case pulseIPCMessage:
	case pulseIPCSpecial:
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
		pthread_create(&t1, &threadAttr, Client,&(aboutServerInfoStruct));
		break;
	case relatedProcess:
		if ((pidSon = fork()) == 0) {
			if (execv("QNXIPCUnClient", argv) == -1) {
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
	int len;
	len=strlen(buffer_read);
	int ret;
	ret=0;

	//Messages
	int rcvid=-1;

	switch (aboutServerInfoStruct.IPCTypeSelector) {
	case signalIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before pause. Waiting for signal!\n");
		for (;;) {
			pause();
		}
		break;

	case pipeIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before read from pipe!\n");
		while (len>0 && (ret=read(aboutServerInfoStruct.fileDes[0], buffer_read,len ))!= 0) {
			if(ret==-1){
				if(errno == EINTR){
					continue;
				}
				DEBUG_PRINT("ERROR","Internal read error");
				break;
			}
			if(ret!=0){
				DEBUG_PRINT("INFO",buffer_read);
				TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Read from pipe!\n");
				len-=ret;
				buffer_read+=ret;
			}
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After read from pipe!\n");
		close(aboutServerInfoStruct.fileDes[0]);
		close(aboutServerInfoStruct.fileDes[1]);
		break;

	case fifoIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before read from fifo!\n");
		while (len>0 && (ret=read(aboutServerInfoStruct.fifoDes, buffer_read,len ))!= 0) {
			if(ret==-1){
				if(errno == EINTR){
					continue;
				}
				if(errno == EAGAIN){
					continue;
				}
				DEBUG_PRINT("ERROR","Internal read error");
				break;
			}
			if(ret!=0){
				DEBUG_PRINT("INFO",buffer_read);
				TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Read from fifo!\n");
				len-=ret;
				buffer_read+=ret;
			}
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After reading from fifo!\n");
		close(aboutServerInfoStruct.fifoDes);
		unlink(aboutServerInfoStruct.pathToFifo.c_str());
		break;

	case semaphoreIPCUnnamed:
		for(int i=0; i<2; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before unnamed semaphore!\n");
			sem_wait(&aboutServerInfoStruct.semUnnamedStandart);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: In unnamed semaphore!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before post unnamed semaphore!\n");
			sem_post(&aboutServerInfoStruct.semUnnamedStandart);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After post unnamed semaphore!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
		};

		for(int i=0; i<2; i++){
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before shared memory unnamed semaphore in server!\n");
			sem_wait(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: In shared memory unnamed semaphore in server!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before shared memory post unnamed semaphore in server!\n");
			sem_post(&aboutServerInfoStruct.semUnnamedThroughSharedMemory);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After shared memory post unnamed semaphore in server!\n");
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
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before named semaphore in server!\n");
			sem_wait(aboutServerInfoStruct.semNamed);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: In named semaphore in server!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before post named semaphore in server!\n");
			sem_post(aboutServerInfoStruct.semNamed);
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After post named semaphore in server!\n");
			for (int i = 0; i < 10000; i++) {
				for (int j = 0; j < 100; j++) {
					fake1 = i * j;
				}
			}
		};

		sem_close(aboutServerInfoStruct.semNamed);
		break;

	case messageIPCSend_Block:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before receive message!\n");
		if((rcvid =MsgReceive(aboutServerInfoStruct.chid, &buffer_read, len, NULL))<=0){
			if(rcvid==0){
				DEBUG_PRINT("INFO", "Pulse! WTF?!");
				break;
			}
			else{
				perror("[ERROR]: MsgReceive");
			}
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After receive message!\n");

		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before reply message!\n");
		if(MsgReply(rcvid,EOK,&buffer_write, len)==-1){
			perror("[ERROR]: MsgReply");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before reply message!\n");
		break;

	case messageIPCRecieved_Block:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before send message!\n");
		if(MsgSend(aboutServerInfoStruct.chid,&buffer_write, len,&buffer_read, len )==-1){
			perror("[ERROR]: MsgSend");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After send message!\n");
		break;

	case pulseIPCMessage:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before receive pulse!\n");
		if((rcvid =MsgReceive(aboutServerInfoStruct.chid, &buffer_read, len, NULL))<=0){
			if(rcvid==0){
				TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After receiving pulse!\n");
				DEBUG_PRINT("INFO", "Pulse!");
				break;
			}
			else{
				perror("[ERROR]: MsgReceive");
			}
		};

		break;

	case pulseIPCSpecial:
		char pulseRecieved[5];
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before send message!\n");
		if(MsgReceivePulse(aboutServerInfoStruct.chid, pulseRecieved, sizeof(pulseRecieved),NULL)){
				perror("[ERROR]: MsgSend");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After send message!\n");
		break;

	default:
		break;
	}
}
/*------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------*/
void getInfgoAboutServer(AboutServerInfoStruct *aboutServerInfoStruct){
	aboutServerInfoStruct->pid=getpid();
	aboutServerInfoStruct->ppid=getppid();
	aboutServerInfoStruct->tid=pthread_self();
}
/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
 int main(int argc, char *argv[]) {
 	AboutServerInfoStruct aboutServerInfoStruct;

	parseParametrsMy(argc, argv, &aboutServerInfoStruct); //Parsing input parametrs
	getInfgoAboutServer(&aboutServerInfoStruct);


	//Pipe
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
