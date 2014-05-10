#include <cstdlib>
#include <iostream>


#include "../CommonLibPrj/CommonFunctions.hpp"
#include "../CommonLibPrj/Client.hpp"
#include "../CommonLibPrj/Headers.hpp"
#include "../CommonLibPrj/AboutServerInfoStruct.hpp"
#include "QNXIPCUnClient.hpp"




int readFromFile(AboutServerInfoStruct *aboutServerInfoStructl){
	FILE *filePointer = NULL;
	filePointer = fopen((aboutServerInfoStructl->pathToFileWithServerInfo).c_str(), "r");
	if (filePointer == NULL) {
		printf("[ERROR]: %d can not open file with program information because of: %s\n",	errno, strerror(errno));
		return -1;
	}

	char buff[100];
	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &((*aboutServerInfoStructl).pid));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &((*aboutServerInfoStructl).ppid));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &((*aboutServerInfoStructl).tid));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d",&((*aboutServerInfoStructl).fileDes[0]));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d",&((*aboutServerInfoStructl).fileDes[1]));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%s",&((*aboutServerInfoStructl).pathToFifo));

	return 0;
}





int main(int argc, char *argv[]) {
	std::cout << "Welcome to the client!" << std::endl;

	AboutServerInfoStruct aboutServerInfoStruct;






	parseParametrsMy(argc, argv, &aboutServerInfoStruct);
	/*END: !!!!!!!!!!!!!!!Processing input keys!!!!!!!!!!!!!!!*/



	readFromFile(&aboutServerInfoStruct);
	//aboutServerInfoStructl.IPCTypeSelector=IPCTypeSelector;



	sleep(2);
	Client(&aboutServerInfoStruct);
	return EXIT_SUCCESS;
}
