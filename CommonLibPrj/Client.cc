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


void *Client(void *arg) {
	AboutServerInfoStruct aboutServerInfoStruct = *(AboutServerInfoStruct * )arg;

	std::cout<<"In client!"<<std::endl;
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
	char buffer_pipe_write[]="This us write in pipe";;

	switch ((aboutServerInfoStruct.IPCTypeSelector)) {
	case signalIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending SIG1");
		kill(aboutServerInfoStruct.pid, SIGUSR1);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending  SIG1");
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before sending SIG2");
		SignalKill(0, aboutServerInfoStruct.pid, aboutServerInfoStruct.tid, SIGUSR2, NULL, NULL);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After sending SIG2");
		break;
	case pipeIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Writing in pipe");
		write(aboutServerInfoStruct.fileDes[1],buffer_pipe_write, strlen(buffer_pipe_write));
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in pipe");
		break;
	case fifoIPC:
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Writing in fifo");
		//WTF?!
		//write(aboutServerInfoStruct.fileDes[1],buffer_pipe_write, strlen(buffer_pipe_write));
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in fifo");
		break;
	default:
		break;
	}

	return NULL;
}

