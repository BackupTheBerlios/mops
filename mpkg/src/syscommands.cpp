#include "syscommands.h"

void runShellCommand(std::string cmd)
{
	pid_t child;
	pid_t pid;
	int status;

	child = fork();

	if ( child == (pid_t) 0 ) {
		// child
		execl( "/bin/sh", cmd.c_str() );
		perror( strerror(errno) );
		exit(1);
	}

	if ( child == -1 ) {
		// error
		perror( strerror(errno) );
		exit(1);
	}

	pid = waitpid(child, &status, 0);

	if ( WIFEXITED(status) ) {
#ifdef DEBUG
			printf("DEBUG: process %i exit successfull\n", pid);
#endif
		
	} else {
		perror( strerror(errno) );
		exit(1);
	}
}