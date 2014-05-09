#include "../CommonLibPrj/CommonFunctions.hpp"
#include "../CommonLibPrj/Client.hpp"

#include "QNXIPCUnServer.hpp"



extern int errno;

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
int inline infoToFile(pid_t pid, pid_t ppid, pthread_t tid, int *fileDes, char *buffer_pipe_write ){
	//Output file pointer
	FILE *filePointer = NULL;

#ifdef DEBUG_MY
	std::cout << "[INFO]: Writing information to info.info" << std::endl;
#endif
	filePointer = fopen(INFO_FILE_NAME, "w");
	if (filePointer == NULL) {
		printf("[ERROR]: %d can not create file with program information because of: %s\n",errno, strerror(errno));
		return -1;
	}

	fprintf(filePointer, "PID: %d\n", pid);
	fprintf(filePointer, "PPID: %d\n", ppid);
	fprintf(filePointer, "TID: %d\n", tid);

	//For pipe
	fprintf(filePointer, "PIPE[0]: %d\n", fileDes[0]);
	fprintf(filePointer, "PIPE[1]: %d\n", fileDes[1]);
	fprintf(filePointer, "BufferLength: %d\n", strlen(buffer_pipe_write));

	//For fifo
	fprintf(filePointer, "FIFO: %d\n", fileDes[0]);

	fclose(filePointer);
	return 0;
}
/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
int inline preWork(IPCType IPCTypeSelector, int *fileDes, std::string path, int *fifoDes){
#ifdef DEBUG_MY
	std::cout << "[INFO]: Preparing server part of IPC" << std::endl;
#endif

	switch (IPCTypeSelector) {
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
		if (pipe(fileDes) < 0) {
			printf("[ERROR]: %d creating pipe file. That means: %s\n", errno,strerror(errno));
			return -9;
		}
		break;
	case fifoIPC:
		if ((mkfifo(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
			printf("[ERROR]: %d creating fifo file. That means: %s\n", errno,strerror(errno));
			return -10;
		}
		if((*fifoDes=open(path.c_str(), O_RDWR))<=0){
			printf("[ERROR]: %d opening fifo file. That means: %s\n", errno,strerror(errno));
			return -11;
		}

		break;
	case messageIPCSend_Block:

		break;
	case messageIPCRecieved_Block:

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
int makeThreadProcess(participantsType participantsTypeSelector,char *argv,IPCType IPCTypeSelector){
#ifdef DEBUG_MY
	std::cout << "[INFO]: Starting client if it necessary" << std::endl;
#endif
	pthread_t t1;
	int pidSon;

	switch (participantsTypeSelector) {
	case oneProcessThreads:
		pthread_attr_t threadAttr;
		pthread_attr_init(&threadAttr);
		pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
		int i;
		i=0;
		pthread_create(&t1, &threadAttr, Client, &i);
		break;
	case relatedProcess:
		if ((pidSon = fork()) == 0) {
			if (execl("QNXIPCUnClient", argv) == -1) {
				printf("[ERROR]: %d error running client process. That means: %s\n", errno,	strerror(errno));
				return -11;
			}
		};
		break;
	default:
		break;
	}
	return 0;
}
/*------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------*/
/*Waiting for something (Server part)
 * */
int recievingPart(IPCType IPCTypeSelector, int *fileDes, char* buffer_pipe_read, char *buffer_pipe_write, std::string path){
#ifdef DEBUG_MY
	std::cout << "[INFO]: Starting client if it necessary" << std::endl;
#endif
	switch (IPCTypeSelector) {
	case signalIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before pause. Waiting for signal!\n");
		for (;;) {
			pause();
		}
		break;

	case pipeIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before read from pipe!\n");
		while (read(fileDes[0], buffer_pipe_read, strlen(buffer_pipe_write))!= -1) {
			TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Read from pipe!\n");
		};
		close(fileDes[0]);
		break;

	case fifoIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before read from fifo!\n");
		while (read(fileDes[0], buffer_pipe_read, strlen(buffer_pipe_write))!= -1) {
					TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Read from fifo!\n");
		};
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: After reading from fifo!\n");
		unlink(path.c_str());
		break;

	case messageIPCSend_Block:

		break;

	case messageIPCRecieved_Block:

		break;
	default:
		break;
	}
}
/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
 int main(int argc, char *argv[]) {




	IPCType IPCTypeSelector = signalIPC;
	participantsType participantsTypeSelector = oneProcessThreads;
	parseParametrsMy(argc, argv, &IPCTypeSelector, &participantsTypeSelector); //Parsing input parametrs


	/*BEGIN: !!!!!!!!!!!!!!!Getting information about server. For output info file!!!!!!!!!!!!!!!*/
	pid_t pid = getpid();
	pid_t ppid = getppid();

	pthread_t tid = pthread_self();

	//Pipe
	int fileDes[2];
	char buffer_pipe_write[]="This us write in pipe";
	char buffer_pipe_read[strlen(buffer_pipe_write)];

	//FIFO
	int fifoDes=0;
	std::string path = "/tmp/fifo.fifo";

	if(infoToFile(pid, ppid, tid, fileDes, buffer_pipe_write )==-1){
		goto deInit;
	};



	preWork(IPCTypeSelector, fileDes, path, &fifoDes);

	makeThreadProcess(participantsTypeSelector,argv[0], IPCTypeSelector);

	recievingPart(IPCTypeSelector, fileDes, buffer_pipe_read, buffer_pipe_write, path);

	printf("\n PROCESS PARAM: pid=%i  ppid=%i \n", pid, ppid);



	for (;;) {
		pause();
	}

	deInit:
	//fclose(filePointer);

	return 0;
}
 /*------------------------------------------------------------------------------------*/
