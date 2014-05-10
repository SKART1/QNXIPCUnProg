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

void *Client(void *arg) {
	AboutServerInfoStruct aboutServerInfoStruct = *(AboutServerInfoStruct * )arg;
	DEBUG_PRINT("INFO", "In client!");




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
	char *buffer_write="This us write in pipe\0";
	int len;
	int written;

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
		len=strlen(buffer_write);
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: Before writing in pipe");
		while(len!=0 && ((written=write(aboutServerInfoStruct.fileDes[1],buffer_write, len)) !=0)){
			if(written==-1){
				if(errno==EINTR){
					continue;
				}
				perror("[ERROR]: Write in pipe");
				break;
			}
			len-=written;
			buffer_write+=written;
		}
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in pipe");
		close(aboutServerInfoStruct.fileDes[0]);
		close(aboutServerInfoStruct.fileDes[1]);
		break;
	case fifoIPC:
		if(
				(aboutServerInfoStruct.fifoDes=open((aboutServerInfoStruct.pathToFifo).c_str(), O_RDWR))<=0
				){
				perror("[ERROR]: Opening fifo file");
				break;
		}
		while(1);
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
			buffer_write+=written;
		}
		TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, 3, "[INFO]: After writing in fifo");
		if(aboutServerInfoStruct.participantsTypeSelector==relatedProcess){
			unlink(aboutServerInfoStruct.pathToFifo.c_str());
		}
		close(aboutServerInfoStruct.fifoDes);
		break;
	default:
		break;
	}

	return NULL;
}

