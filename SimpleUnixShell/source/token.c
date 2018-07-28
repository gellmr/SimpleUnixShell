/*
	File:		token.c
	Author:		Mike Gell
	Created:	2 Oct 2012
	Modified:	2 Oct 2012
*/

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <glob.h>

#include "../include/token.h"

#include "../include/shellEngine.h"




	/*
	Takes a null-terminated line of chars inputLine

	...breaks it into a sequence of tokens

	...assigns the starting addresses of these tokens to the array token.

	The max size for token[] is MAX_NUM_TOKENS
	*/
int tokenise (char * inputLine, char * token[])
{
	char * tk; // point to the first char of each token that is found.

	int i = 0; // The number of tokens that are found.


	/*
	Register the string "inputLine" with the strtok() function.

	The first call to strtok() returns the first token that is found.
	*/

	tk = strtok(inputLine, tokenSeparators);

	token[i] = tk; // save the first token that is found.

	if (DEBUG_MODE)
	{
		printf("\ttoken[%d] == %s", i, tk);
	}

	++i;



	// printf("strlen(tk): %d\n", (int)strlen(tk));

	// find the rest of the tokens.

	int done = 0;

	while (tk != NULL && !done)
	{

		if (i >= MAX_NUM_TOKENS)
		{
			i = -1; // The inputLine is too big.

			done = 1; // exit the while loop.
		}
		else
		{
			/*
			Successive calls to strtok() return the next

			token that is found.
			*/

			tk = strtok(NULL, tokenSeparators);



			if (tk == NULL)
			{
				token[i] = NULL;

				if (DEBUG_MODE)
				{
					printf("\n");

					printf("\ttoken[%d] == NULL", i, tk);
				}
				++i;
			}
			else
			{
				// printf("strlen(tk): %d\n", (int)strlen(tk));

				tk[(int)strlen(tk)] = '\0'; // append NULL CHAR to the end of the string.

				token[i] = tk; // save this token. May be a null pointer.

				if (DEBUG_MODE)
				{
					printf("\n");

					printf("\ttoken[%d] == %s", i, tk);
				}
				++i;



				// Resolve wildcards

				char * pch = NULL;

				char wildcards[] = "*?";

				pch = strpbrk (tk, wildcards);


				// tk contains a wildcard

				if (pch != NULL)
				{
					// Token contains wildcard.

					if (DEBUG_MODE)
					{
						printf(" (found %c) ", *pch);
					}

			
					// Get any wildcard pathnames.

					/*
					struct glob_t
					{
						size_t gl_pathc,	// Stores the number of pathnames that match the given pattern.

						char ** gl_pathv,	// Pointer to a list of matched pathnames

						size_t	gl_offs		// Slots to reserve at the beginning of gl_pathv
					}
					*/

					glob_t globResult; // receives the results of our pattern matching request.

					int globFlags = 0;

					globFlags |= GLOB_MARK;	// Causes the glob function to append trailing slash to any pathname that matches a directory. eg "/pathToFolder/include" becomes "/pathToFolder/include/"

					// Match all accessible pathnames against the given pattern. Develop a list of all pathnames that match.

					int(* errFunctionName)(const char * epath, int eerrno);

					errFunctionName = NULL;

					int patMatchResult;

					patMatchResult = glob
					(
						tk,			// pointer to a pathname to be expanded, eg "*.txt" for all .txt files

						globFlags,		// flags to control how glob does its work.			

						errFunctionName,	// function pointer for receiving errors that glob encounters. We don't care about errors.

						&globResult		// glob_t * restrict globResult 
					);	

					// globResult->gl_pathc // the number of pathnames that match the given pattern. Zero if no pathnames are found to match.

					// globResult->gl_pathv // pointer to ( a list of pointers to ( pathnames ) )  // pathnames are stored in sort order. The first pointer after the last pathname is a null pointer.

					// PERFORM OPERATIONS HERE.

					if (patMatchResult == 0)
					{
						if (globResult.gl_pathc > 0)
						{

							int g = 0;

							int charCount = MAX_NUM_TOKENS * MAX_PROMPT_LEN;

							char buf[MAX_NUM_TOKENS][MAX_PROMPT_LEN];

							memset (buf, '\0', charCount);

							for(g = 0; g <= globResult.gl_pathc; g++)
							{
								char * globbedPathName = globResult.gl_pathv[g];

								if (globbedPathName != NULL)
								{
									if (g == 0)
									{
										i -= 1;
									}

									int sourceLen = (int)strlen(globbedPathName);

									char * bufPtr = buf[g];

									// strcpy(bufPtr, "Hello");

									memmove ( bufPtr, globbedPathName, sizeof(char *) * sourceLen );

									//strcpy(buf, *globbedPathName);

									token[i] = bufPtr;

									//token[i] = globResult.gl_pathv[g];

									++i;

								}
								else
								{
									//token[i] = NULL; // end of wildcard expansion.	

									//printf("\ttoken[%d] == NULL", i);	

									//printf("\n");	
								
									//++i;

									//printf("\tEnd of globResult.\n");
								}

								if (i >= MAX_NUM_TOKENS)
								{
									i = -1; // The inputLine is too big.

									done = 1; // exit the while loop.

									return i;
								}
							}
						}
						else
						{
							if (DEBUG_MODE)
							{
								printf("\tNo matches found.\n");
							}
						}
					}
					else
					{
						// printf("\t\t%d\n", patMatchResult);

						if (DEBUG_MODE)
						{
							if (patMatchResult == GLOB_ABORTED)
							{
								printf(" GLOB_ABORTED");
							}
							else if (patMatchResult == GLOB_NOMATCH)
							{
								printf(" GLOB_NOMATCH");
							}
							else if (patMatchResult == GLOB_NOSPACE)
							{
								printf(" GLOB_NOSPACE");
							}
						}

					}


					/// int idx = 0; for(idx = 0; idx < i; idx++){printf("\t\t\t%s\n", token[idx]);}	printf("\n");

					globfree(&globResult); // free any memory that was allocated by the glob() function.

					/// idx = 0; for(idx = 0; idx < i; idx++){printf("\t\t\t\t\t\t%s\n", token[idx]);}

				} // pch != NULL

			} // if (tk != NULL)
			
		} // if i >= MAX_NUM_TOKENS
		
	}

	if (DEBUG_MODE)
	{
		printf("\n");
	}
	return i; // the number of tokens that were found.
}











