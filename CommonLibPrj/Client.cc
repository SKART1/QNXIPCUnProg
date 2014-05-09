/*
 * Client.cc
 *
 *  Created on: 28.04.2014
 *      Author: Art
 */
#include "Client.hpp"
#include "Headers.hpp"
#include "CommonLibPrj.hpp"


void *Client(void *arg) {
	IPCType iPCType = *((IPCType *) (arg));
	//int *b=(int *) arg;
	//IPCType iPCType = signalIPC;
	std::cout<<"In client!"<<std::endl;
	//FIFO
	std::string path = INFO_FILE_NAME;

	//Output file pointer
	FILE *filePointer = NULL;



	filePointer = fopen(path.c_str(), "r");
	if (filePointer == NULL) {
		printf("[ERROR]: %d can not open file with program information because of: %s\n",	errno, strerror(errno));
		return NULL;
	}


	pid_t pid = -1;
	pid_t ppid=-1;
	pthread_t tid =-1;


	int fileDes[2];

	char buff[100];
	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &pid);

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &ppid);

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &tid);

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d",&(fileDes[0]));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d",&(fileDes[1]));

	//int	buffer_pipe_writeLength=fscanf(filePointer, "%d");

	//fprintf(filePointer, "BufferLength: %d\n", strlen(buffer_pipe_write));
	//char buffer_pipe_write[] = "This us write in pipe";
	char buffer_pipe_write[]="This us write in pipe";;

	switch (iPCType) {
	case signalIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending SIG1");
		kill(pid, SIGUSR1);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending  SIG1");
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending SIG2");
		SignalKill(0, pid, tid, SIGUSR2, NULL, NULL);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending SIG2");
		break;
	case pipeIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Writing in pipe");
		write(fileDes[1],buffer_pipe_write, strlen(buffer_pipe_write));
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in pipe");
		break;
	case fifoIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Writing in fifo");
		write(fileDes[1],buffer_pipe_write, strlen(buffer_pipe_write));
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in fifo");
		break;
	default:
		break;
	}

	return NULL;
}

