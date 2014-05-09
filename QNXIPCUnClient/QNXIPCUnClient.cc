#include <cstdlib>
#include <iostream>


#include "../CommonLibPrj/CommonFunctions.hpp"
#include "../CommonLibPrj/Client.hpp"
#include "../CommonLibPrj/Headers.hpp"
#include "QNXIPCUnClient.hpp"


int main(int argc, char *argv[]) {
	std::cout << "Welcome to the client!" << std::endl;

	/*BEGIN: !!!!!!!!!!!!!!!Processing input keys!!!!!!!!!!!!!!!*/
	IPCType IPCTypeSelector = signalIPC;
	participantsType participantsTypeSelector = oneProcessThreads;

	parseParametrsMy(argc, argv, &IPCTypeSelector, &participantsTypeSelector);
	/*END: !!!!!!!!!!!!!!!Processing input keys!!!!!!!!!!!!!!!*/


	sleep(2);
	Client(&IPCTypeSelector);
	return EXIT_SUCCESS;
}
