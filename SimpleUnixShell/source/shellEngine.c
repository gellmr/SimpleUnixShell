/*
	File:		shellEngine.c
	Author:		Mike Gell
	Created:	17 Oct 2012
	Modified:	17 Oct 2012
	
	Carries out given commands.
*/


#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <fcntl.h>

#include "../include/shellEngine.h"



#define PIPE_READ 0

#define PIPE_WRITE 1




char * indentBuffer;





	/*
	Forward declarations
	*/

//void updateCWD();

//void showCWD();

//void getPWD(char * result);

void showEnv();

void pwd();

void pr_exit(int status, pid_t childId, pid_t resultOfWait, char * childcommand);








	/*
	Pointer to a pointer to our environment variables.
	These should be located in the HIGH ADDRESS extremity of the program's virtual memory allocation.
	
	environ is the address of an array of char* values.
	Each char* contains the address of a null terminated c-string.
	
	*/
extern char ** environ;







char * makeIndent(int param)
{
	int c = param + 1;

	if (indentBuffer == NULL)
	{

		indentBuffer = (char *) malloc (c + 1);

		if (indentBuffer == NULL)
		{
			exit (1);
		}

		int n;

		for (n = 0; n < c; n++)
		{
			indentBuffer[n]='\t';
		}
		indentBuffer[n]='\0';
	}
}







	/*
	Print the exit status of a child process.
	*/
void pr_exit(int status, pid_t childId, pid_t resultOfWait, char * childcommand)
{
	if (!DEBUG_MODE)
	{
		return;
	}
	if (WIFEXITED(status))
	{
		/*
		printf
		(
			"Parent sees:   command %s (childPID: %d) has completed successfully.  normal termination, exit status = %d\n",

			childcommand,

			(int)childId,
		
			WEXITSTATUS(status)
		);
		*/
	}
	else if (WIFSIGNALED(status))
	{
		printf
		(
			"Parent sees:   command %s (childPID: %d) has terminated abnormally. waitResult: %d  signal number = %d%s\n",

			childcommand,

			(int)childId,

			(int)resultOfWait,

			WTERMSIG(status),

		#ifdef WCOREDUMP
		
			WCOREDUMP(status) ? " (core file generated)" : "");

		#else

			"");

		#endif

	}
	else if (WIFSTOPPED(status))
	{
		printf
		(
			"Parent sees:   command %s (childPID: %d) has stopped.  waitResult: %d  signal number = %d\n",

			childcommand,

			(int)childId,

			(int)resultOfWait,

			WSTOPSIG(status)
		);
	}
}









	/**
	prints all environment strings
	*/
void showEnv()
{
	printf("\n\n(environ: %s)\n\n", *environ);

	int c = 0;

	char * e = environ[c];

	do
	{
		size_t length = strlen (e);

		printf("     environ[%d]:  length: %d   %s \n\n", c, (int)length, e );
		
		e = environ[++c];
	}
	while(e != NULL);
}














	/**
	Get the home directory from the environment strings
	*/
void getHomeDir(char * result)
{
	char * pwdNVPname = "HOME";

	char * pwdNVPvalue;

	if
	(
		(pwdNVPvalue = getenv(pwdNVPname)) != NULL
	)
	{
		int dirLen = strlen(pwdNVPvalue);

		if (dirLen > MAX_PWD_LEN)
		{
			dirLen = MAX_PWD_LEN;
		}

		strncpy ( result, pwdNVPvalue, dirLen );
	}
}







void pwd()
{
	char buf[MAX_PWD_LEN];

	if (getcwd(buf, MAX_PWD_LEN) == NULL)
	{
		printf("Error getting the cwd. (MAX_PWD_LEN may be too small - See Stevens and Rago p.126) Exiting...\n");

		exit(1);
	}
	
	printf("%s\n", buf);
}






	/**
	******************************************************

		Display the command details

		This method is intended for debug only.

	******************************************************
	*/
void displayCommandDetails (int c, Command * const cm, char ** const token)
{
	printf("\n%sExecute Command %d\t", indentBuffer, c);

	int t;

	for
	(
		t = cm->first;

		t < (cm->first + (cm->argc - 1));

		t++
	)
	{
		printf("%s ", token[t]);
	}

	printf("\t(argc %d)", cm->argc);

	printf(" (sep %s)", cm->sep);

	if (cm->stdin_file != NULL)
	{
		printf(" (stdin redir %s)", cm->stdin_file);
	}

	if (cm->stdout_file != NULL)
	{
		printf(" (Stdout redir %s)", cm->stdout_file);
	}

	printf("\n");
}








	/**
	************************************************************************
		     	  built-in Directory Walk command

	This command is similar to that provided by the Bash built-in command cd.

	In particular, typing the command without a path should set the current

	directory of the shell to the home directory of the user.

	Note: this program suffers from the following limitation:

	Pathnames cannot exceed MAX_PWD_LEN

	************************************************************************


	For the algorithm used by "cd" SEE:

	http://pubs.opengroup.org/onlinepubs/009695399/utilities/cd.html

	See Rago page 125:
				#include <unistd.h>
					chdir
					fchdir
					getcwd
	*/
void directoryWalk(Command * const cm, char ** const token)
{

	if (cm->argc == 2)
	{
		if (DEBUG_MODE)
		{
			printf("\t(Built-in command \"cd\" (no args))\n");
		}
		// no args. Go to home directory.

		// WORKS IF YOU CALL "CD " STRAIGHT AWAY. DOES NOT WORK IF

		// YOU NAVIGATE TO A DIFFERENT DIRECTORY AND CALL "CD "

		// ...IT FAILS TO TAKE YOU TO THE HOME DIRECTORY.

		char homeDir[MAX_PWD_LEN];

		getHomeDir(homeDir);

		printf("\thomeDir: %s\n", homeDir);

		chdir(homeDir);
	
		pwd();

	}
	else
	{
		if (DEBUG_MODE)
		{
			printf("\t(Built-in command \"cd\" (with args))\n");
		}
		// args. change directory to the given arg.

		// Change working directory according to the given command.

		char * targetDir = token[cm->first + 1]; // eg "cd testDir"

		int targetLen = strlen(targetDir);

		// printf("TO DO: Change to %s\n", targetDir);
		
		char buf[MAX_PWD_LEN];

		if (getcwd(buf, MAX_PWD_LEN) == NULL)
		{
			printf("Error getting the cwd. (MAX_PWD_LEN may be too small - See Stevens and Rago p.126) Exiting...\n");

			exit(1);
		}

		// append pathname of target dir to the cwd in buf.

		// Need to ensure that the resulting string does not exceed MAX_PWD_LEN

		int cwdlen = strlen(buf);

		if ((cwdlen + targetLen) >= MAX_PWD_LEN)
		{
			printf("Error building target path for cd command. (cwdlen + targetLen >= MAX_PWD_LEN) Exiting...\n");

			exit(1);
		}
	
		char newCWD[MAX_PWD_LEN];

		strcpy (newCWD, buf);

		strcat (newCWD, "/");

		strcat (newCWD, targetDir);

		if (DEBUG_MODE)
		{
			printf("\tnewCWD: %s\n", newCWD);
		}
		chdir(newCWD);
	}
}









	/**
	**********************************************************************************
	
		TRY TO PERFORM THE GIVEN COMMAND AS A BUILT-IN SHELL COMMAND

	**********************************************************************************

	Returns 1 if the given command was launched	(WAS A BUILT-IN COMMAND)

	Returns 0 if the given command was not launched	(WAS NOT A BUILT-IN COMMAND)

	Returns -1 if we are exiting the shell.

	**********************************************************************************
	*/
int executeAsBuiltInCommand (Command * const cm, char ** const token, char * promptString)
{

	if
	(
		/***************************************
				"exit"
		****************************************/

		strcmp (token[cm->first], "exit") == 0
	)
	{
		return -1; // exit

	}
	else if
	(
		/***************************************
				"prompt"
		****************************************/

		strcmp (token[cm->first], "prompt") == 0
		&&
		cm->argc > 2
	)
	{
		char * promptToken = token[cm->first + 1];

		int newPromptLen = strlen(promptToken);
		
		strncpy ( promptString, promptToken, newPromptLen );
		
		printf("\tPrompt string changed to \"%s\"\n", promptString);

		return 1; // the given command was successfully executed.

	}
	else if
	(
		/***************************************
				"pwd"
		****************************************/

		strcmp (token[cm->first], "pwd") == 0
		&&
		cm->argc == 2
	)
	{
		if (DEBUG_MODE)
		{
			printf("\t(Built-in command \"pwd\")\n");
		}

		pwd();

		return 1; // the given command was successfully executed.

	}
	else if
	(
		/***************************************
				"environ"
		****************************************/

		strcmp (token[cm->first], "environ") == 0
		&&
		cm->argc == 2
	)
	{
		if (DEBUG_MODE)
		{
			printf("\t(Built-in command \"environ\")\n");
		}
		// Show the environment strings

		showEnv();

		return 1; // the given command was successfully executed.
	}
	else if
	(
		
		/***************************************
			     Directory Walk
		****************************************/

		strcmp (token[cm->first], "cd") == 0
	)
	{
		directoryWalk(cm, token);

		pwd();

		return 1; // the given command was successfully executed.
	}

	return 0; // nothing was executed.

} // executeAsBuiltInCommand












	/*
	*******************************************************

	  	   Execute command as a child process

	*******************************************************
	----------------------------------------------------------------------------------------------------------------------------------------------------------

	If MAX_NUM_COMMANDS == 3 then we will have 2 entries in allpipes.

	Eg the following illustrates the maximum possible usage of pipes, if MAX_NUM_COMMANDS == 3



			   file or NULL
				|
				|
				V
		+------------------------------------------------+
		|   command(0) stdin	    command(0) stdout    |	FIRST COMMAND: c == 0		allpipesInputIdx == -1		allpipesOutputIdx == 0
		+------------------------------------------------+
							 |
							 |
							 V
	allpipes[0] == { {PIPE_READ},    <---	  {PIPE_WRITE} }
				|
				|
				V
		+------------------------------------------------+
		|   command(1) stdin	    command(1) stdout    |	SECOND COMMAND: c == 1		allpipesInputIdx == 0		allpipesOutputIdx == 1
		+------------------------------------------------+
							 |
							 |
							 V
	allpipes[1] == { {PIPE_READ},    <---	  {PIPE_WRITE} }
				|
				|
				V
		+------------------------------------------------+
		|   command(2) stdin	    command(2) stdout    |	THIRD COMMAND: c == 2		allpipesInputIdx == 1		allpipesOutputIdx == 2
		+------------------------------------------------+
							 |
							 |
							 V
			  			    file or NULL

	----------------------------------------------------------------------------------------------------------------------------------------------------------
	*/
void executeAsChild (int c, Command * const cm, char ** const token)
{

	// Get the command name

	char * commandName = token[cm->first];

	if (DEBUG_MODE)
	{
		printf("%s(command %s) --> executeAsChild(c: %d)\n", indentBuffer, commandName, c);
	}


	// Get the stdin / stdout pipe handles that were allocated for this command. We don't use them if they are NULL.

	int * inputPipe;	// child process can get stdin from this.

	int * outputPipe;	// child process can send stdout to this.

	int allpipesInputIdx	= c - 1;	// Pipe coming from the previous command

	int allpipesOutputIdx	= c;		// Pipe going to the following command




	// First command cannot have an input pipe.

	if ((allpipesInputIdx) < 0)
	{
		allpipesInputIdx = -1;		// indicates non-usage

		inputPipe = NULL;		// indicates non-usage
	}
	else
	{
		// Get the input pipe.

		inputPipe = &(allpipes[allpipesInputIdx][0]);		// input from previous command, if any. If not NULL, This pipe will have already been created by the previous command.
	}




	// Last command cannot have an output pipe.

	if (allpipesOutputIdx >= (MAX_NUM_COMMANDS - 1))
	{		
		allpipesOutputIdx = -1;		// indicates non-usage

		outputPipe = NULL;		// indicates non-usage
	}
	else
	{
		// Get the output pipe.

		outputPipe = &(allpipes[allpipesOutputIdx][0]);	// output to the next command, if any.
	}




	/*
	***********************************
	***********************************
	***********************************
	***********************************


	

	// Make a pipe if necessary.

	// Note that we never need to create an INPUT pipe, as this is done by the previous command when it creates its OUTPUT pipe.

	// Check if this command has an output pipe. If this is the last command, then allpipesOutputIdx will be -1.

	if ((allpipesOutputIdx >= 0))
	{
		// This is not the last command.

		// Has the command specified that we should send its stdout to a pipe?
		
		if (strcmp (cm->sep, "|") == 0)
		{
			// This command's output goes to an unnamed pipe.

			printf("%sCommand %s will redirect its stdout to an unnamed pipe.\n", indentBuffer, commandName);
	
			// Make outputPipe

			printf("%s(Making output pipe for command %s)\n", indentBuffer, commandName);

			if (pipe(outputPipe) < 0)
			{
				perror("perror: ");

				printf("%sPipe error (outputPipe) for %s\n", indentBuffer, commandName);

				exit(1);
			}

			// Ensure we keep the pipe handles. Redundant operation ???

			int readHandle = outputPipe[PIPE_READ];		// need to dereference outputPipe??? 

			int writeHandle = outputPipe[PIPE_WRITE]; 	// need to dereference outputPipe??? 

			allpipes[allpipesOutputIdx][PIPE_READ] = readHandle; 	// saved handle.

			allpipes[allpipesOutputIdx][PIPE_WRITE] = writeHandle; 	// saved handle.

			// Note that   allpipes[allpipesOutputIdx]   is   allpipes[allpipesInputIdx]   for the following command.
		}
	}



	***********************************
	***********************************
	***********************************
	***********************************
	*/





	// allpipes is initially cleared, setting each value to -1.

	// Check if the input pipe handle is uninitialized. If so, set the pointer to null.

	if (inputPipe != NULL && (*inputPipe == -1))
	{
		inputPipe = NULL;	// indicates non-usage
	}

	// Check if the output pipe handle is uninitialized. If so, set the pointer to null.

	if (outputPipe != NULL && (*outputPipe == -1))
	{
		outputPipe = NULL;	// indicates non-usage
	}






	/*
	-----------------------------------------------------------------------------------
	LOGICAL VIEW OF THIS COMMAND:


		allpipes[allpipesInputIdx] ==   { {PIPE_READ},    <---	  {PIPE_WRITE} }	INPUT PIPE FROM PREVIOUS COMMAND
							|
							|
							V
					+-------------------------------------------------+
					|   command(c) stdin	    command(c) stdout     |	THIS COMMAND
					+-------------------------------------------------+
										 |
										 |
										 V
		allpipes[allpipesOutputIdx] ==   { {PIPE_READ},    <---	  {PIPE_WRITE} }	OUTPUT PIPE TO FOLLOWING COMMAND





	-----------------------------------------------------------------------------------
	THE SAME LOGICAL VIEW OF THIS COMMAND:


	  	  inputPipe[PIPE_READ]					INPUT PIPE FROM PREVIOUS COMMAND
				|
				|
				V
		+-------------------------------------------------+
		|   command(c) stdin	    command(c) stdout     |	THIS COMMAND
		+-------------------------------------------------+
							 |
							 |
							 V
					 outputPipe[PIPE_WRITE]		OUTPUT PIPE TO FOLLOWING COMMAND

	------------------------------------------------------------------------------------
	*/





	// Output pipe has been created if necessary. We are ready to fork.

	if (DEBUG_MODE)
	{
		printf("%s(Parent pid %d) of (command %s) is forking...\n", indentBuffer, (int)getpid(), commandName);
	}

	pid_t pid;

	if (( pid = fork()) < 0)
	{
		printf("%s*** fork error *** (command %s)\n", indentBuffer, commandName);

		exit(1);
	}
	if (pid == 0)
	{



		// Child
		
		if (DEBUG_MODE)
		{	
			printf("%sI am child (pid %d) of command %s\n", indentBuffer, (int)getpid(), commandName);
		}

		// WAIT FOR A SIGNAL FROM PARENT TO INDICATE THAT ALL PIPES IN THE CHAIN HAVE BEEN ESTABLISHED ???






		// Close the unwanted pipe handles

		if (inputPipe != NULL)
		{
			if (DEBUG_MODE)
			{
				printf("%sChild closing inputPipe[WRITE] (command %s)\n", indentBuffer, commandName);
			}
			close(inputPipe[PIPE_WRITE]); // cannot write to the input pipe.
		}
		else
		{
			if (DEBUG_MODE)
			{
				printf("%sChild - inputPipe[WRITE] is already closed (As it should be). (command %s)\n", indentBuffer, commandName);
			}
		}

		if (outputPipe != NULL)
		{
			if (DEBUG_MODE)
			{	
				printf("%sChild closing outputPipe[READ] (command %s)\n", indentBuffer, commandName);
			}
			close(outputPipe[PIPE_READ]); // cannot read from the output pipe.
		}
		else
		{
			if (DEBUG_MODE)
			{
				printf("%sChild - outputPipe[READ] is already closed (as it should be). (command %s)\n", indentBuffer, commandName);
			}
		}
	












		// Check that we have the pipes we need.

		if ((allpipesInputIdx >= 0) && (inputPipe != NULL) && (*inputPipe != -1))
		{

			// We are supposed to have an input pipe

			struct stat sb;

			if (fstat(inputPipe[PIPE_READ], &sb) == -1)
			{
				perror("stat - inputPipe PIPE_READ");

				exit(1);
			}

			ino_t handleInode = sb.st_ino;

			if (DEBUG_MODE)
			{
				printf("%sChild - (command %s) has input pipe[%d] [ino_t %d]\n", indentBuffer, commandName, inputPipe[PIPE_READ], (int)handleInode);
			}
		}

		if ((allpipesOutputIdx >= 0) && (outputPipe != NULL) && (*outputPipe != -1))
		{

			// We are supposed to have an output pipe

			struct stat sb;

			if (fstat(outputPipe[PIPE_WRITE], &sb) == -1)
			{
				perror("stat - outputPipe PIPE_WRITE");

				exit(1);
			}

			ino_t handleInode = sb.st_ino;

			if (DEBUG_MODE)
			{
				printf("%sChild - (command %s) has output pipe[%d] [ino_t %d]\n", indentBuffer, commandName, outputPipe[PIPE_WRITE], (int)handleInode);
			}
		}










		// Perform stdin / stdout redirection as necessary

		// STDIN_FILENO	 	(The keyboard)

		// STDOUT_FILENO	(The terminal window)

		// STDERR_FILENO	(The terminal window)
	
		char * stdin_pathname;

		char * stdout_pathname;

		int fd_in = -1;

		int fd_out = -1;




		// Perform stdin redirection from inputPipe or file

		if (inputPipe != NULL)
		{
			// Got an inputPipe

			// Redirect stdin from inputPipe.
		
			if (DEBUG_MODE)
			{
				printf("%sRedirecting stdin from (inputPipe) for (command %s)...", indentBuffer, commandName);
			}

			if (dup2(inputPipe[PIPE_READ], STDIN_FILENO) == -1)
			{
				perror("dup2 failed for inputPipe[PIPE_READ]-->STDIN_FILENO. perror: ");

				printf("%sdup2 failed for inputPipe[PIPE_READ]-->STDIN_FILENO (command %s)\n", indentBuffer, commandName);
				
				exit(1);
			}
			
			if (DEBUG_MODE)
			{
				printf("%s...success\n", indentBuffer);
			}
		}
		else
		{
			// There is no inputPipe

			// printf("%sCommand \"%s\" has no inputPipe.\n", indentBuffer, commandName);

			// Do we need to redirect stdin from a file ?

			stdin_pathname = cm->stdin_file;

			if ((stdin_pathname) != NULL)
			{
				int oflag = O_RDONLY;  // read only.

				fd_in = open(stdin_pathname, oflag);

				/*
				Make this program use fd_in as stdin.

				Duplicate fd_in onto our file descriptor for stdin.

				The following call will close the current process' filedescriptor for stdin.

				Then it copies the file descriptor fd_in into the perProcessTableOfOpenFiles

				row for stdin. Thus we get input from fd_in.
				*/

				if (DEBUG_MODE)
				{
					printf("%sRedirecting stdin from (%s) for (command %s)...", indentBuffer, stdin_pathname, commandName);
				}

				if (dup2(fd_in, STDIN_FILENO) < 0)
				{
					perror("dup2 failed for fd_in-->STDIN_FILENO. perror: ");

					printf("%sdup2 failed for stdin_pathname %s (command %s)\n", indentBuffer, commandName, stdin_pathname);

					exit(1);
				}

				if (DEBUG_MODE)
				{
					printf("%s...success\n", indentBuffer);
				}
			}
			else
			{
				// No stdin redirection.

				if (DEBUG_MODE)
				{
					printf("%sNo stdin redirection for command %s\n", indentBuffer, commandName);
				}
			}
		}


		

		// Perform stdout redirection to outputPipe or file

		if ( outputPipe != NULL )
		{
		
			// Got an outputPipe.

			// Redirect stdout to outputPipe.

			if (DEBUG_MODE)
			{
				printf("%sRedirecting stdout to (outputPipe) for (command %s)...", indentBuffer, commandName);
			}

			if (dup2(outputPipe[PIPE_WRITE], STDOUT_FILENO) == -1)
			{
				perror("dup2 failed for outputPipe[PIPE_WRITE]-->STDOUT_FILENO. perror: ");

				printf("%sdup2 failed for outputPipe[PIPE_WRITE]-->STDOUT_FILENO (command %s)\n", indentBuffer, commandName);
			}

			if (DEBUG_MODE)
			{
				printf("%s...success\n", indentBuffer);
			}
		}
		else
		{
			// There is no outputPipe

			// printf("%sCommand \"%s\" has no outputPipe.\n", indentBuffer, commandName);

			// Do we need to redirect stdout to a file ?

			stdout_pathname = cm->stdout_file;
		
			if ((stdout_pathname) != NULL)
			{
				int oflag = O_WRONLY | O_CREAT;  // write only. Create if necessary.

				int mode = 0755; // Permission bits: rwxr-xr-x

				mode_t oldmask = umask(0); // save current umask so we can restore it later.

				fd_out = open(stdout_pathname, oflag, mode);

				umask (oldmask); // restore old umask

				/*
				Make this program use fd_out as stdout.

				Duplicate fd_out onto our file descriptor for stdout.

				The following call will close the current process' filedescriptor for stdout.

				Then it copies the file descriptor fd_out into the perProcessTableOfOpenFiles

				row for stdout. Thus we redirect output to fd_out.
				*/

				if (DEBUG_MODE)
				{
					printf("%sRedirecting stdout to (%s) for (command %s)...", indentBuffer, stdout_pathname, commandName);
				}

				if (dup2(fd_out, STDOUT_FILENO) < 0)
				{
					perror("dup2 failed for fd_out-->STDOUT_FILENO. perror: ");

					printf("%sdup2 failed for stdout_pathname %s of command %s\n", indentBuffer, stdout_pathname, commandName);
				}

				if (DEBUG_MODE)
				{
					printf("%s...success\n", indentBuffer);
				}
			}
			else
			{
				// No stdout redirection.

				if (DEBUG_MODE)
				{
					printf("%sNo stdout redirection for command %s\n", indentBuffer, commandName);
				}
			}
		}



		// Run this process in the background ?

		if(strcmp (cm->sep, "&") == 0)
		{
			if (setpgid(0, 0))
			{
				perror("Child tried to setpgid.");
			}
		}



		/*
		---------------------------------------------------------------

				load and execute the command

		---------------------------------------------------------------

		NEED TO PASS THE COMMAND LINE ARGS FOR THE CHILD.

		char * const argumentVector = *(cm->argv);

		char * const envp[] = environ;

		---------------------------------------------------------------
		*/

		if (DEBUG_MODE)
		{
			printf("%sPerforming (command %s)...\n", indentBuffer, commandName);
		}

		if (execvpe(commandName, cm->argv, environ) < 0)
		{
			printf("%s*** execvpe error *** (command %s)\n", indentBuffer, commandName);
		}


		// Only reaches here if the execvpe failed.

		printf("%sFailed to execvpe for (command %s)\n", indentBuffer, commandName);
	

		// If we used stdin redirection from a file, close the file.

		if (fd_in >= 0)
		{
			close(fd_in);
		}

		// If we used stdout redirection to a file, close the file.

		if (fd_out >= 0)
		{
			close(fd_out);
		}

		// If we used stdin redirection from a pipe, close the pipe read handle.

		if (inputPipe != NULL)
		{
			close(inputPipe[PIPE_READ]);
		}

		// If we used stdout redirection to a pipe, close the pipe write handle.

		if (outputPipe != NULL)
		{
			close(outputPipe[PIPE_WRITE]);
		}
		
		exit(1);

		// never gets to here.
	}





	// Parent...

	if (inputPipe != NULL)
	{
		// Ensure the parent does not have any handles for the previous command's input pipe

		// Only the previous command can read from its input pipe.

		close(inputPipe[PIPE_READ]);

		close(inputPipe[PIPE_WRITE]);

		allpipesInputIdx = -1;		// indicates non-usage

		inputPipe = NULL;		// indicates non-usage
	}

	if (outputPipe != NULL)
	{
		// ensure the parent does not have a write handle for the output pipe of the child just launched (the current command)

		close(outputPipe[PIPE_WRITE]); // Only the child just launched can write to its output pipe.

		// close(outputPipe[PIPE_READ]); // DO NOT CLOSE - this is PIPE_READ for the input pipe of the following command.

		allpipes[allpipesOutputIdx][PIPE_READ] = outputPipe[PIPE_READ]; // Make sure the following command can get its input pipe handle.

		allpipesOutputIdx = -1;		// indicates non-usage

		outputPipe = NULL;		// indicates non-usage
	}


	



	/*
	----------------------------------------------------------------------------------------

	Check what kind of execution has been specified for the command we just performed.

	----------------------------------------------------------------------------------------

		;	Sequential execution.	(child executes in FOREGROUND).	Parent waits.

		&	Sequential execution.	(child executes in BACKGROUND).	Parent continues.

		|	Parallel execution.	(child executes in FOREGROUND).	Parent continues.

	---------------------------------------------------------------------------------------
	*/
	if (strcmp (cm->sep, ";") == 0)
	{
		// Sequential execution.	(child executes in FOREGROUND).	Parent waits for child to finish.

		int statusLocation;

		pid_t result = waitpid(pid, &statusLocation, 0); // parent blocked until child is finished.

		if (result != pid)
		{
			printf
			(
				"%sParent process encountered wait error. result %d != pid %d\n",

				indentBuffer, 

				(int)result,

				(int)pid
			);
		}

		pr_exit(statusLocation, pid, result, commandName);
	}
	else if(strcmp (cm->sep, "&") == 0)
	{
		// Sequential execution.	(child executes in BACKGROUND).	Parent continues.
		/*
		int statusLocation;

		pid_t result = waitpid(pid, &statusLocation, 0); // parent blocked until child is finished.

		if (result != pid)
		{
			printf
			(
				"Parent process encountered wait error. result %d != pid %d\n",

				(int)result,

				(int)pid
			);
		}

		pr_exit(statusLocation, pid, result, command);
		*/
	}
	else if(strcmp (cm->sep, "|") == 0)
	{
		// Parallel execution.	(child executes in FOREGROUND).	Parent continues.

		// The command we just performed has redirected its stdout to an unnamed pipe.



		/**** We get problems when the pipe has not been created yet. Try waiting for each command to finish

		int statusLocation;

		pid_t result = waitpid(pid, &statusLocation, 0); // parent blocked until child is finished.

		if (result != pid)
		{
			printf
			(
				"%sParent process encountered wait error. result %d != pid %d\n",

				indentBuffer, 

				(int)result,

				(int)pid
			);
		}

		pr_exit(statusLocation, pid, result, commandName);
		*********/
	}

} // executeAsChild









	/**
	Clears our pipe handles, so we are ready to launch a new set of commands.
	*/
void clearAllPipes()
{
	int p = 0;

	for (p = 0; p < (MAX_NUM_COMMANDS - 1); p++)
	{
		allpipes[p][PIPE_READ] = -1;

		allpipes[p][PIPE_WRITE] = -1;
	}
}









	/**
	Make the pipes we will need for this set of commands.
	*/
void createPipes(Command command[], int num, char * token[])
{
	int c;

	// For each command...

	for(c = 0; c < num; c++)
	{

		// Get the command

		Command * cm = &(command[c]);

		char * commandName = token[cm->first];

		


		// Get the stdin / stdout pipe handles that used with this command. We don't use them if they are NULL.

		int * inputPipe;	// child process can get stdin from this.

		int * outputPipe;	// child process can send stdout to this.

		int allpipesInputIdx	= c - 1;	// Pipe coming from the previous command

		int allpipesOutputIdx	= c;		// Pipe going to the following command





		// First command cannot have an input pipe.

		if ((allpipesInputIdx) < 0)
		{
			allpipesInputIdx = -1;		// indicates non-usage

			inputPipe = NULL;		// indicates non-usage
		}
		else
		{
			// Get the input pipe.

			inputPipe = &(allpipes[allpipesInputIdx][0]);		// input from previous command, if any. If not NULL, This pipe will have already been created by the previous command.
		}




		// Last command cannot have an output pipe.

		if (allpipesOutputIdx >= (MAX_NUM_COMMANDS - 1))
		{		
			allpipesOutputIdx = -1;		// indicates non-usage

			outputPipe = NULL;		// indicates non-usage
		}
		else
		{
			// Get the output pipe.

			outputPipe = &(allpipes[allpipesOutputIdx][0]);	// output to the next command, if any.
		}




		// Does this command have an output pipe?

		if (strcmp (cm->sep, "|") == 0)
		{
			// This command's output goes to an unnamed pipe.

			if (DEBUG_MODE)
			{
				printf("Command %s will redirect its stdout to an unnamed pipe.\n", commandName);
			}

			// Make outputPipe

			if (DEBUG_MODE)
			{
				printf("Making output pipe for command %s...", commandName);
			}

			if (pipe(outputPipe) < 0)
			{
				perror("perror: ");

				printf("Pipe error (outputPipe) for %s\n", commandName);

				exit(1);
			}

			if (DEBUG_MODE)
			{
				printf("Done.\n");
			}
			
			// Ensure we keep the pipe handles. Redundant operation ???

			int readHandle = outputPipe[PIPE_READ];		// need to dereference outputPipe??? 

			int writeHandle = outputPipe[PIPE_WRITE]; 	// need to dereference outputPipe??? 

			allpipes[allpipesOutputIdx][PIPE_READ] = readHandle; 	// saved handle.

			allpipes[allpipesOutputIdx][PIPE_WRITE] = writeHandle; 	// saved handle.

			// Note that   allpipes[allpipesOutputIdx]   is   allpipes[allpipesInputIdx]   for the following command.
		}
	}
}








	/**
	Returns 0 if the given set of commands were all launched successfully.

	Returns -1 if we are exiting the shell.
	*/
int executeCommands(Command command[], int num, char * token[], char * promptString)
{

	clearAllPipes();

	createPipes(command, num, token);

	int c;

	// For each command...

	for(c = 0; c < num; c++)
	{

		// Get the command

		Command * cm = &(command[c]);


		indentBuffer = NULL;

		makeIndent(c);

		
		if (DEBUG_MODE)
		{
			displayCommandDetails(c, cm, token);
		}
		

		// Try to launch as a built-in command.

		int launched = executeAsBuiltInCommand(cm, token, promptString);

		if (launched == 0)
		{
			// try to launch as a child process

			/// resolveWildcards(cm, token);
			
			executeAsChild(c, cm, token);
		}

		if (indentBuffer != NULL)
		{
			free(indentBuffer);
		}

		indentBuffer = NULL;

		if (launched == -1)
		{
			// "exit" command was given.

			return -1;
		}
	}

	if (DEBUG_MODE)
	{
		printf("\n");

		printf("\tFinished executing commands.\n\n");
	}


	// Clean up any zombie children.

	int waitOptions = 0;

	waitOptions |= WNOHANG; // non blocking call.

	for(c = 0; c < num; c++)
	{
		int status = 0;

		pid_t zombieChild = waitpid(-1, &status, waitOptions);

		if (zombieChild == -1)
		{
			if (DEBUG_MODE)
			{
				perror("\tzombie cleanup. ");
			}
			
			// printf("\tEncountered error while waiting for zombie children. status: %d\n", status);
		}
		else
		{
			if (DEBUG_MODE)
			{
				printf("\tParent has cleaned up after zombie child %d\n", (int)zombieChild);
			}
		}
	}
	
	if (DEBUG_MODE)
	{
		printf("\n");
	}

	return 0;
}
