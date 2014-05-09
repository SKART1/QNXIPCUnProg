/*
 * CommonFunctions.h
 *
 *  Created on: 24.04.2014
 *      Author: Art
 */

#ifndef COMMONFUNCTIONS_H_
#define COMMONFUNCTIONS_H_

#include "Headers.hpp"

void client(IPCType IPCTypeSelector);

int parseParametrsMy(int argc, char *argv[], IPCType *IPCTypeSelector,
		participantsType *participantsTypeSelector);



#endif /* COMMONFUNCTIONS_H_ */
