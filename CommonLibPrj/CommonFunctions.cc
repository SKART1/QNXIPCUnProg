#include "Headers.hpp"
#include "CommonFunctions.hpp"
#include "CommonLibPrj.hpp"
#include "AboutServerInfoStruct.hpp"

void correctFormatInfo();

int parseParametrsMy(int argc, char *argv[], AboutServerInfoStruct *aboutServerInfoStruct) {
	int c = 0;
	int counter1=0;
	int counter2=0;


	if (argc < 3) {
		correctFormatInfo();
		return (EXIT_FAILURE);
	};

	while ((c = getopt(argc, argv, "ABC:DEFG:HIabcdef")) != -1) {
		switch (c) {
		case 'A':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = signalIPC;
			break;
		case 'B':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = pipeIPC;
			break;
		case 'C':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = fifoIPC;
			aboutServerInfoStruct->pathToFifo=optarg;
			break;
		case 'D':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = messageQueuIPC;
			strcpy(aboutServerInfoStruct->pathToMessageQueue,optarg);
			break;
		case 'E':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = sharedMemoryIPC;
			strcpy(aboutServerInfoStruct->pathToSemNamedStandart,optarg);
			break;
		case 'F':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = semaphoreIPCUnnamed;
			break;
		case 'G':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = semaphoreIPCNamed;
			strcpy(aboutServerInfoStruct->pathToSemNamedStandart,optarg);
			break;
		case 'H':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = messageIPCSend_Block;
			break;
		case 'I':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = messageIPCRecieved_Block;
			break;
		case 'J':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = pulseIPCMessage;
			break;
		case 'K':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = pulseIPCSpecial;
			break;
		case 'L':
			counter1++;
			aboutServerInfoStruct->IPCTypeSelector = pulseIPCFromInterruptHandler;
			break;

		case 'a':
			counter2++;
			std::cout << "a" << std::endl;
			aboutServerInfoStruct->participantsTypeSelector = oneProcessThreads;
			break;
		case 'b':
			counter2++;
			std::cout << "b" << std::endl;
			aboutServerInfoStruct->participantsTypeSelector = relatedProcess;
			break;
		case 'c':
			counter2++;
			std::cout << "c" << std::endl;
			aboutServerInfoStruct->participantsTypeSelector = independentProcessLocal;
			break;
		case 'd':
			counter2++;
			std::cout << "d" << std::endl;
			aboutServerInfoStruct->participantsTypeSelector = independentProcessNetwork;
			break;
		case 'e':
			counter2++;
			std	::cout << "e" << std::endl;
			aboutServerInfoStruct->participantsTypeSelector = independentThreadsLocal;
			break;
		case 'f':
			counter2++;
			std	::cout << "f" << std::endl;
			aboutServerInfoStruct->participantsTypeSelector = independentThreadsNetwork;
			break;
		case '?':
			std::cout << "[ERROR]: Unrecognized option!" << std::endl;
			correctFormatInfo();
			return -1;
		default:
			std	::cout << "[ERROR]: Unrecognized option!" << std::endl;
			correctFormatInfo();
			return -1;
		}
		if(counter2>1 || counter1>1){
			std	::cout << "[ERROR]: Only one parameter allowed!" << std::endl;
			correctFormatInfo();
			return -1;
		}
	}

	if(counter2<1 || counter1<1){
		std::cout<<"[ERROR]: Not enough arguments!"<<std::endl;
		correctFormatInfo();
		return -1;
	};

	if(optind<(argc)){
		strcpy(aboutServerInfoStruct->pathToFileWithServerInfo,argv[optind]);
	}
	else{
		std::cout<<"[ERROR]: Path for the file not specified!"<<std::endl;
		correctFormatInfo();
		return -1;
	}


	return 0;
}


void correctFormatInfo(){
	std::cout<<"[ERROR]: Programm usage format: server.out \n -S - signal \n -P - pipe \n -F  %FILEPATH_FOR_FIFO.FIFO_FILE% - fifo \n -m - recievd-blocked messages \n -M - send-blocked messages  \n %FILEPATH_FOR_SERVERINFO_FILE%  "<< std::endl;
}
