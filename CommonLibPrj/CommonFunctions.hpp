/*
 * CommonFunctions.h
 *
 *  Created on: 24.04.2014
 *      Author: Art
 */

#ifndef COMMONFUNCTIONS_H_
#define COMMONFUNCTIONS_H_

#include <string>
#include "Headers.hpp"
#include "AboutServerInfoStruct.hpp"

void client(IPCType IPCTypeSelector);

int parseParametrsMy(int argc, char *argv[],  AboutServerInfoStruct *aboutServerInfoStruct);



#endif /* COMMONFUNCTIONS_H_ */
