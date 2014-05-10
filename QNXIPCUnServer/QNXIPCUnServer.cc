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

	fprintf(filePointer, "PID: %d\n", aboutServerInfoStruct.pid);
	fprintf(filePointer, "PPID: %d\n", aboutServerInfoStruct.ppid);
	fprintf(filePointer, "TID: %d\n", aboutServerInfoStruct.tid);

	//For pipe
	fprintf(filePointer, "PIPE[0]: %d\n", aboutServerInfoStruct.fileDes[0]);
	fprintf(filePointer, "PIPE[1]: %d\n", aboutServerInfoStruct.fileDes[1]);

	//For fifo
	fprintf(filePointer, "FIFO_PATH: %s\n", aboutServerInfoStruct.pathToFifo.c_str());

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
		if ((mkfifo((aboutServerInfoStruct->pathToFifo).c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
			printf("[ERROR]: %d creating fifo file. That means: %s\n", errno,strerror(errno));
			return -10;
		}
		if(((aboutServerInfoStruct->fifoDes)=open((aboutServerInfoStruct->pathToFifo).c_str(), O_RDWR))<=0){
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
int makeThreadProcess(char *argv[], AboutServerInfoStruct aboutServerInfoStruct){
#ifdef DEBUG_MY
	std::cout << "[INFO]: Starting client if it necessary" << std::endl;
#endif
	pthread_t t1;
	int pidSon;

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
		if (execl("QNXIPCUnClient", argv) == -1) {
			perror("[ERROR]: Execl");
		}
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
	len=0;
	int ret;
	ret=0;

	switch (aboutServerInfoStruct.IPCTypeSelector) {
	case signalIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 100,"[INFO]: Before pause. Waiting for signal!\n");
		for (;;) {
			pause();
		}
		break;

	case pipeIPC:
		len=0;
		len=strlen(buffer_write);
		ret=0;
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
