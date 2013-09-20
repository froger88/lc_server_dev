/* 
 * File:   unpack.h
 * Author: froger
 *
 * Created on 18 styczeń 2010, 21:51
 */

#ifndef _UNPACK_H
#define	_UNPACK_H

#include <string>
#include <zlib.h>

using namespace std;

bool unpack(const unsigned char* buffIn, int buffLength, string &buffOut, int maxOutSize=4096); //zwraca true jezeli OK, false jezeli critical error

#endif	/* _UNPACK_H */

