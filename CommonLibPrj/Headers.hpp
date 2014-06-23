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

	messageQueuIPC,
	sharedMemoryIPC,

	semaphoreIPCUnnamed,
	semaphoreIPCNamed,

	messageIPCRecieved_Block,
	messageIPCSend_Block,

	pulseIPCMessage,
	pulseIPCSpecial,
	pulseIPCFromInterruptHandler,
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
