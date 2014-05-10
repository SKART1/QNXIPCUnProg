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

	while ((c = getopt(argc, argv, "SPFabcdef")) != -1) {
		switch (c) {
		case 'S':
			counter1++;
			std::cout << "S" << std::endl;
			aboutServerInfoStruct->IPCTypeSelector = signalIPC;
			break;
		case 'P':
			counter1++;
			std::cout << "P" << std::endl;
			aboutServerInfoStruct->IPCTypeSelector = pipeIPC;
			break;
		case 'F':
			counter1++;
			std::cout << "F" << std::endl;
			aboutServerInfoStruct->IPCTypeSelector = fifoIPC;
			break;
		case 'M':
			counter1++;
			std::cout << "S" << std::endl;
			aboutServerInfoStruct->IPCTypeSelector = messageIPCSend_Block;
			break;
		case 'm':
			counter1++;
			std::cout << "m" << std::endl;
			aboutServerInfoStruct->IPCTypeSelector = messageIPCRecieved_Block;
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
	if(!(optind<(argc-1))){
		aboutServerInfoStruct->pathToFileWithServerInfo=argv[optind];
		aboutServerInfoStruct->pathToFifo=argv[optind+1];
	}
	else{
		std::cout<<"[ERROR]: Path for the file not specified!"<<std::endl;
		correctFormatInfo();
		return -1;
	}


	return 0;
}


void correctFormatInfo(){
	std::cout<<"[ERROR]: Programm usage format: server.out \n -S - signal \n -P - pipe \n -F - fifo \n -m - recievd-blocked messages \n -M - send-blocked messages  %FILEPATH_FOR_SERVERINFO_FILE%  %FILEPATH_FOR_FIFO.FIFO_FILE%"<< std::endl;
}
