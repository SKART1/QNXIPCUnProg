#include "Headers.hpp"
#include "CommonFunctions.hpp"
#include "CommonLibPrj.hpp"

int parseParametrsMy(int argc, char *argv[], IPCType *IPCTypeSelector,
		participantsType *participantsTypeSelector) {
	int c = 0;
	int counter1=0;
	int counter2=0;

	if (argc < 2) {
		std::cout<< "[ERROR]: Programm usage format: server.out \n -S - signal \n -P - pipe \n -F - fifo \n -m - recievd-blocked messages \n -M - send-blocked messages"<< std::endl;
		return (EXIT_FAILURE);
	};

	while ((c = getopt(argc, argv, "SPFabcdef")) != -1) {
		switch (c) {
		case 'S':
			counter1++;
			std::cout << "S" << std::endl;
			*IPCTypeSelector = signalIPC;
			break;
		case 'P':
			counter1++;
			std::cout << "P" << std::endl;
			*IPCTypeSelector = pipeIPC;
			break;
		case 'F':
			counter1++;
			std::cout << "F" << std::endl;
			*IPCTypeSelector = fifoIPC;
			break;
		case 'M':
			counter1++;
			std::cout << "S" << std::endl;
			*IPCTypeSelector = messageIPCSend_Block;
			break;
		case 'm':
			counter1++;
			std::cout << "m" << std::endl;
			*IPCTypeSelector = messageIPCRecieved_Block;
			break;

		case 'a':
			counter2++;
			std::cout << "a" << std::endl;
			*participantsTypeSelector = oneProcessThreads;
			break;
		case 'b':
			counter2++;
			std::cout << "b" << std::endl;
			*participantsTypeSelector = relatedProcess;
			break;
		case 'c':
			counter2++;
			std::cout << "c" << std::endl;
			*participantsTypeSelector = independentProcessLocal;
			break;
		case 'd':
			counter2++;
			std::cout << "d" << std::endl;
			*participantsTypeSelector = independentProcessNetwork;
			break;
		case 'e':
			counter2++;
			std	::cout << "e" << std::endl;
			*participantsTypeSelector = independentThreadsLocal;
			break;
		case 'f':
			counter2++;
			std	::cout << "f" << std::endl;
			*participantsTypeSelector = independentThreadsNetwork;
			break;
		case '?':
			std::cout << "[ERROR]: Unrecognized option!" << std::endl;
			return -1;
		default:
			std	::cout << "[ERROR]: Unrecognized option!" << std::endl;
			return -1;
		}
		if(counter2>1 || counter1>1){
			std	::cout << "[ERROR]: Only one parameter allowed!" << std::endl;
			return -1;
		}
	}
	return 0;
}
