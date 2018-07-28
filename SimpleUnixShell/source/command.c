	/**

	@file:

	command.c for Week 8


	@purpose:

	To separate a list of tokens into a sequence of commands.


	@assumptions:

	Any two successive commands in the list of tokens are

	separated by one of the following command separators:


		"|"  - pipe to the next command

		"&"  - shell does not wait for the proceeding command to terminate

		";"  - shell waits for the proceeding command to terminate



	@author:	HX

	@date:		2006.09.21

	@note:		not tested therefore it may contain errors

	*/







#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "../include/command.h"










	/*
	return 1 if the token is a command separator

	return 0 otherwise
	*/
int isSeparator(char *token)
{
	int i = 0;

	char *commandSeparators[] = { pipeSep, conSep, seqSep, NULL };

	while (commandSeparators[i] != NULL)
	{
		if (strcmp(commandSeparators[i], token) == 0)
		{
			return 1; 
		}

		++ i;
	}

	return 0;
}
















	/*
	Fill one command structure with the details
	*/
void fillCommandStructure
(
	Command * cp,

	int first,	// first token

	int last,	// last token
	
	char * sep	// final separator
)
{
	cp->first = first;

	cp->last = last - 1;

	cp->sep = sep; 
}


















	/**
	@purpose:
		separate the list of token from array "token" into a sequence of commands, to be

		stored in the array "command". 


	@return:
		1) the number of commands found in the list of tokens, if successful, or

		2) -1, if the the array "command" is too small.

		3) < -1, if there are following syntax errors in the list of tokens.

		a) -2, if any two successive commands are separated by more than one command separator

		b) -3, the first token is a command separator

		c) -4, the last command is followed by command separator "|"

		d) -5, the number of commands exceeds MAX_NUM_COMMANDS

	@assume:
		the array "command" must have at least MAX_NUM_COMMANDS number of elements


	@note:
		The last command may be followed by "&", or ";", or nothing.

		If nothing is  followed by the last command, we assume it is followed by ";".

		If the return value nCommands >=0 then set command[nCommands] to NULL, 
	*/
int separateCommands(char * token[], Command command[])
{
	int i;

	int nTokens;


	// find out the number of tokens

	i = 0;

	while (token[i] != NULL)
	{
		++i;
	}
	nTokens = i;



	// if empty command line

	if (nTokens == 0) 
	{
		return 0;
	}



	// check the first token

	if (isSeparator(token[0]))
	{
		return -3; // Error: first token is a separator.
	}



	// check last token, add ";" if necessary 

	if (!isSeparator(token[nTokens-1]))
	{
		// might be a problem here as we are adding another token... are they using static allocation?

		// what is the max number of tokens? How do we know we can add one to the end of the list like that?

		// Check max number of tokens before performing this step???
		
		token[nTokens] = seqSep; // add the ';' separator to the end

		++nTokens; // ...can you do that?
	}




	int firstTok = 0;	// points to the first token of the command

	int lastTok;      	// points to the last  token of the command

	char * commandSep;	// The separator at the end of the command

	int commandIndex = 0;	// command index

	for (i = 0; i < nTokens; ++i)
	{
		if (commandIndex >= MAX_NUM_COMMANDS)
		{
			return -5; // too many commands.
		}

		lastTok = i;

		char * tok = token[i];
		
		if (isSeparator(tok))
		{
			commandSep = tok;

			if (firstTok == lastTok)
			{
				return -2; // error: two consecutive separators
			}

			fillCommandStructure
			(
				&(command[commandIndex]), firstTok, lastTok, commandSep
			);

			

			// For each command, check if we need to redirect stdin / stdout.
	
			searchRedirection
			(
				token,

				&(command[commandIndex])
			);

			// For each command, build argv.

			buildCommandArgumentArray
			(
				token,

				&(command[commandIndex])
			);


			++ commandIndex;

			firstTok = i + 1;
		}
	}



	// Check the last token of the last command 
 
	if (strcmp(token[lastTok], pipeSep) == 0) // last token is pipe separator
	{ 
		return -4; // error: last command pipes to nothing.
	} 



	// calculate the number of commands

	int nCommands = commandIndex;

	return nCommands;


	/*
	Once the function separateCommands finally returns,
	you will have an array of command structures and each command structure,
	command[i], contains the information that is almost ready for use
	(except wildcards handling) to execute the command with system call execvp:

	execvp(command[i].argv[0], command[i].argv);
	*/
}




















	/**

	/// NEW ///

	@purpose:

		Scans the given array of tokens, from token[cp->first] to token[cp->last],

		looking for the standard input redirection symbol "<"

		and the standard output redirection symbol ">".

		Once found, the token following the redirection symbol is treated as

		the redirection file name, and is assigned to either

		cp->stdin_file if it is the standard input redirection,

		or to stdout_file if it is the standard output redirection.


	@param:	token	An array of tokens.

	@param: cp	Pointer to a single command structure.

	*/
void searchRedirection(char * token[], Command * cp)
{
	int firstIdx = cp->first;

	int lastIdx = cp->last;

	int idx;

	for(idx = firstIdx; idx < lastIdx; idx++)
	{
		char * tok = token[idx];

		if (strcmp (tok, "<") == 0)
		{
			// Standard Input Redirection
			
			cp->stdin_file = token[idx + 1]; // save the redirection filename
		}
		else if (strcmp (tok, ">") == 0)
		{
			// Standard Output Redirection
			
			cp->stdout_file = token[idx + 1]; // save the redirection filename
		}
	}
}



















	/**
	
	----------------------------------------------------------------------------
	This function dynamically allocates an array of n char pointers for cp->argv

	where n is the number of command line arguments for that command.

	The function then assigns the address of each token to

	the corresponding array element of the array cp->argv

	----------------------------------------------------------------------------
	argv does not tell us about the stdio redirection,

	so we exclude any stdio redirection tokens ('<', '>')

	and the redirection filenames that follow each one.

	----------------------------------------------------------------------------

	Eg, the following command has both stdin redirection and stdout redirection,

	so we must subtract (4) from the value of (n)

		0	1	2	3	4	5	6
		grep	"mike"	<	infile	>	outfile	;

	number of tokens	 == 7
	cp->first		 == 0
	cp->last		 == 6
	cp->last - cp->first	 == 6
	cp->last - cp->first + 1 == 7	// correct for zero-based indices

				  - 2	// account for stdin redirection

				  - 2	// account for stdout redirection
				
				  + 1	// last element of argv is a NULL (char *)

				n == 4	// the number of elements in argv, for this command.

					// cp->argv = { "grep",  "mike",  ";",  (char *) NULL }

	----------------------------------------------------------------------------
	*/
void buildCommandArgumentArray(char *token[], Command *cp)
{
	int n; // The number of command line arguments for the command.

	int stdin_offset = 0;

	int stdout_offset = 0;

	if (cp->stdin_file != NULL)
	{
		stdin_offset = 2;		// account for stdin redirection
	}

	if (cp->stdout_file != NULL)
	{
		stdout_offset = 2;		// account for stdout redirection
	}
	
	n =
	(
		(cp->last - cp->first + 1)	// the numner of tokens in the command

		- ( stdin_offset )		// remove two tokens for stdin redirection

		- ( stdout_offset)		// remove two tokens for stdout redirection

		+ 1				// the last element in argv must be a NULL
	);


	// Save argc

	cp->argc = n;


	// Disallow commands with more than 1000 arguments.

	if (n > 1000)
	{
		printf("(buildCommandArgumentArray) command has more than 1000 arguments! Exiting...\n");

		exit(1);
	}
	else
	{
		// printf("(buildCommandArgumentArray) n: %d\n", n);
	}

	// Dynamically allocates an array of n char pointers for cp->argv:

	size_t sizeOfArgv = sizeof(char *) * n;

	void * result = realloc(cp->argv, sizeOfArgv);   // should check realloc's return value

	if (result == NULL)
	{
		/*
		Failed to allocate the requested block of memory.

		A null pointer is returned.

		The memory block pointed to by cp->argv is still valid, with contents unchanged.

		(No deallocation was performed)
		*/
		printf("(buildCommandArgumentArray) Could not allocate space for argv!\n");
		
		printf("Exiting...\n");
		
		exit(1);
	}
	else
	{
		cp->argv = (char **) result; // cast the void pointer so we can dereference it later.
	}


	/*
	Assign the address of each token

	to the corresponding array element

	of the array cp->argv:
	*/

	int i;

	int argvIndex = 0;

	for ( i = cp->first;  i <= cp->last;  ++i )
	{
		if
		(
			strcmp(token[i], ">") == 0

			||

			strcmp(token[i], "<") == 0
		)
		{
				// Do NOT add the std in/out redirection OPERATOR to argv.

			++i;    // Do NOT add the std in/out redirection FILENAME to argv.
		}
		else
		{
			cp->argv[argvIndex] = token[i];	// add this token to argv.

			++ argvIndex;
		}
	}

	cp->argv[argvIndex] = NULL; // Final element of argv is a null char pointer
}
















	/*
	Print the given error.
	*/
void printError(int errCode)
{
	switch(errCode)
	{
		case -1:

		printf
			("(Command Separator Error: %d) Command array is too small.\n",

			errCode
		);

		break;




		case -2:

		printf
		(
			"(Command Separator Error: %d) Commands separated by more than one command separator.\n",

			errCode
		);

		break;





		case -3:

		printf
		(
			"(Command Separator Error: %d) First token is a command separator.\n",
			
			errCode
		);

		break;





		case -4:

		printf
		(
			"(Command Separator Error: %d) last command is followed by command separator \"|\".\n",

			errCode
		);
		break;





		case 0:

		printf
		(
			"No commands were made.\n",

			errCode
		);

		break;




		default:

		printf
		(
			"Successfully separated %d commands.\n",

			errCode
		);

		break;

	}	// switch
}















	/*
	Print a single command.
	*/
void printCommand(Command * c, int num, char * token[])
{
	printf("\n");

	printf("    Command %d    ", num);

	int t;

	for(t = c->first; t <= c->last; t++)
	{
		printf("%s ", token[t]);

	}
	printf("%s", c->sep);

	// int argc = 1 + (c->last - c->first);

	printf("    argc: %d", c->argc);

	if (c->stdin_file != NULL)
	{
		printf("\t\tStdin  redirected to %s", c->stdin_file);
	}

	if (c->stdout_file != NULL)
	{
		printf("\t\tStdout redirected to %s", c->stdout_file);
	}

	printf("\n");
}












	/*
	Print the given commands.
	*/
void printCommands(Command command[], int num, char * token[])
{
	int n;

	for(n = 0; n < num; n++)
	{
		printCommand(&command[n], n, token);
	}

	printf("\n");
}
