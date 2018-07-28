#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFERSIZE 4096


// assumes the current umask value is 0002


void closeFile(int fd, const char * pathname)
{
	// Close the file.

	if(close(fd) != 0)
	{
		printf("Problem closing file.\n");

		exit(1);
	}
	else
	{
		// printf("Closed %s\n", pathname);
	}
}




	/*
	Open and read file "foo"

	using file system calls.

	Copy the contents of "foo" into "foo2"

	(Create "foo2" if it does not exist)
	*/
int main(int argc, char * argv[])
{
	

	const char pathname[] = "foo";

	int oflag = O_RDONLY;     // read only

	char buf[BUFFERSIZE];     // buffer

	int readlen = BUFFERSIZE; // read length




	// User can specify to read less bytes.
	/*
	if (argc > 1)
	{
		int arg1 = atoi(argv[1]);

		if (arg1 < BUFFERSIZE)
		{
			readlen = arg1;
		}
	}*/




	// OPEN THE FILE

	int fd = open(pathname, oflag);



	// READ THE FILE

	size_t byteCount;

	byteCount = read(fd, buf, readlen); // returns #bytes read.

	if ((int)byteCount == 0)
	{
		//printf("\nReached EOF\n");
	}
	else if ((int)byteCount == -1)
	{
		printf("\nError reading file\n");
	}
	else
	{
		//printf("\nSuccessfully read %d bytes.\n", (int)byteCount);

		buf[readlen] = '\0';
	}




	// Display the file contents.

	//printf("buf:\n%s", buf);
	


	
	closeFile(fd, pathname);

	//printf("\n");


	int sum = atoi(buf);

	sum += 1;

	printf("\n%d", sum); // display contents

	sprintf (buf, "%d", sum);




	// (OPEN / CREATE) foo2

	const char outpath[] = "foo"; // We will write to this file.

	oflag = O_WRONLY|O_CREAT; // bitwise OR to combine flags
	
	int mode = 0777; // Permission bits: rwxrwxrwx

	mode_t oldmask = umask(0); // save current umask so we can restore it later.
	
	fd = open(outpath, oflag, mode);

	umask (oldmask); // restore old umask



	// WRITE TO FILE

	ssize_t bytesWritten = write (fd, buf, byteCount);

	if (bytesWritten == -1)
	{
		// Error

		printf("Error writing to foo. Exiting...\n");

		exit(1);
	}
	else
	{
		// Success

		/*
		printf
		(
			"Successfully wrote %d bytes to foo2.\n",

			(int)bytesWritten
		);
		*/
	}



	// Write again

	bytesWritten = write (fd, buf, byteCount);

	if (bytesWritten == -1)
	{
		// Error

		printf("Error writing to foo. Exiting...\n");

		exit(1);
	}
	else
	{
		// Success

		/*
		printf
		(
			"Successfully wrote %d bytes to foo2.\n",

			(int)bytesWritten
		);
		*/
	}

	closeFile(fd, outpath);

	return 0;
}
