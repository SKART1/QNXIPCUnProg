#include <cstdlib>
#include <iostream>


#include "../CommonLibPrj/CommonFunctions.hpp"
#include "../CommonLibPrj/Client.hpp"
#include "../CommonLibPrj/Headers.hpp"
#include "../CommonLibPrj/AboutServerInfoStruct.hpp"
#include "QNXIPCUnClient.hpp"




int readFromFile(AboutServerInfoStruct *aboutServerInfoStruct){
	FILE *filePointer = NULL;
	filePointer = fopen((aboutServerInfoStruct->pathToFileWithServerInfo), "r");
	if (filePointer == NULL) {
		printf("[ERROR]: %d can not open file with program information because of: %s\n",	errno, strerror(errno));
		return -1;
	}

	char buff[100];
	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%s", &(aboutServerInfoStruct->serverNodeName));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &((*aboutServerInfoStruct).pid));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &((*aboutServerInfoStruct).ppid));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &((*aboutServerInfoStruct).tid));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d\n\t",&((*aboutServerInfoStruct).fileDes[0]));

	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d\n\t",&((*aboutServerInfoStruct).fileDes[1]));



	if(fscanf(filePointer, "FIFO_PATH: %s\n",  &(aboutServerInfoStruct->pathToFifo))==0){
		fgets (buff,100, filePointer);
	};


	fscanf(filePointer, "%s", buff);
	fscanf(filePointer, "%d", &((*aboutServerInfoStruct).chid));

	return 0;
}





int main(int argc, char *argv[]) {
	std::cout << "Welcome to the client! Argc is: " <<argc << std::endl;


	AboutServerInfoStruct *aboutServerInfoStruct=new AboutServerInfoStruct;






	parseParametrsMy(argc, argv, aboutServerInfoStruct);
	/*END: !!!!!!!!!!!!!!!Processing input keys!!!!!!!!!!!!!!!*/
	std::cout << "Welcome to the client2" << std::endl;


	readFromFile(aboutServerInfoStruct);
	//aboutServerInfoStructl.IPCTypeSelector=IPCTypeSelector;



	usleep(300000);
	Client(aboutServerInfoStruct);
	return EXIT_SUCCESS;
}
