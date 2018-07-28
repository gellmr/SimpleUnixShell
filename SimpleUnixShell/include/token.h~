/*
	File:		token.h
	Author:		Mike Gell
	Created:	2 Oct 2012
	Modified:	2 Oct 2012
*/

#ifndef _TOKEN_H
#define _TOKEN_H





#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_TOKENS 1000 // 8

#define tokenSeparators " \t\n"



	/**

	@Purpose:

	To tokenise the command line which is stored in char array "line"
	as an array of characters.

	It breaks up a sequence of characters into a sequence of tokens.



	@Return:

	1) The number of tokens found in the command line "line"
	or
	2) -1,  if inputLine is too big for us to process.



	@Assume:

	The array passed to the function, token, must have
	at least MAX_NUM_TOKENS number of elements

	*/
int tokenise (char *inputLine, char *token[]);




#endif // _TOKEN_H
