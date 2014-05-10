/*
 * DebugInfoOut.hpp
 *
 *  Created on: 10.05.2014
 *      Author: Art
 */

#ifndef DEBUGINFOOUT_HPP_
#define DEBUGINFOOUT_HPP_

#define DEBUG_PRINT_MY

#ifdef DEBUG_PRINT_MY
	#define DEBUG_PRINT(typeInfoError, format)\
			printf("[%s]: %s\n",typeInfoError, format);
			//printf("%s"format);
			//printf("\n");
#else
	#define DEBUG_PRINT(funcname, format, ...)
#endif



#endif /* DEBUGINFOOUT_HPP_ */
