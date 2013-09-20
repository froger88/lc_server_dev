/* 
 * File:   pack.h
 * Author: froger
 *
 * Created on 18 styczeń 2010, 22:28
 */

#ifndef _PACK_H
#define	_PACK_H

#include <string>
#include <zlib.h>

using namespace std;

int pack(string &dataIn, unsigned char*dataOut, unsigned int opt1, unsigned char opt2, int Length=2048);

#endif	/* _PACK_H */

