	/**
	@file:		command.h for Week 8

	@purpose;	to separate a list of tokens into a sequence of commands.

	@assumptions:

	any two successive commands in the list of tokens are separated by one of

	the following command separators:

			"|"  - pipe to the next command

			"&"  - shell does not wait for the proceeding command to terminate

			";"  - shell waits for the proceeding command to terminate


	@author:		HX

	@date:		2006.09.21

	@lastmodified:	2006.10.05

	@note:		not tested therefore it may contain errors

	*/

#ifndef _COMMAND_H
#define _COMMAND_H




#define MAX_NUM_COMMANDS 100 // 6




// command separators

#define pipeSep  "|"                            // pipe separator "|"

#define conSep   "&"                            // concurrent execution separator "&"

#define seqSep   ";"                            // sequential execution separator ";"











struct CommandStruct
{
	int first;      // index to the first token in the array "token" of the command

	int last;       // index to the last token in the array "token" of the command

	char * sep;	   // the command separator that follows the command,  must be one of "|", "&", and ";"


	int argc;		// The number of arguments in argv

	char ** argv;        	// an array of tokens that forms a command

	char * stdin_file;   	// if not NULL, points to the file name for stdin redirection                        

	char * stdout_file;  	// if not NULL, points to the file name for stdout redirection
};









typedef struct CommandStruct Command;  // command type











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

	@assume:
		the array "command" must have at least MAX_NUM_COMMANDS number of elements


	@note:
		The last command may be followed by "&", or ";", or nothing.

		If nothing is  followed by the last command, we assume it is followed by ";".

		If the return value nCommands >=0 then set command[nCommands] to NULL, 
	*/
int separateCommands(char * token[], Command command[]);










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
void searchRedirection(char * token[], Command *cp);








	/**
	This function dynamically allocates an array of n char pointers for cp->argv

	where n is the number of command line arguments for that command.

	The function then assigns the address of each token to

	the corresponding array element of the array cp->argv
	*/
void buildCommandArgumentArray(char *token[], Command *cp);








	/*
	Print the given error.
	*/
void printError(int errCode);








	/*
	Print the given commands.
	*/
void printCommands(Command command[], int num, char * token[]);









	/*
	Print a single command.
	*/
void printCommand(Command * c, int num, char * token[]);



#endif // _COMMAND_H



