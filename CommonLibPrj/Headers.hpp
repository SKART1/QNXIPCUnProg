/*
 * headers.h
 *
 *  Created on: 24.04.2014
 *      Author: Art
 */

#ifndef HEADERS_H_
#define HEADERS_H_



typedef enum {
	signalIPC,
	pipeIPC,
	fifoIPC,
	messageIPCSend_Block,
	messageIPCRecieved_Block
} IPCType;

typedef enum {
	oneProcessThreads,
	relatedProcess,
	independentProcessLocal,
	independentProcessNetwork,
	independentThreadsLocal,
	independentThreadsNetwork,
} participantsType;


#endif /* HEADERS_H_ */
