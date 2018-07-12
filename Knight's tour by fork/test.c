#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


void my_pipe(int i, int j){
	int newpipe[2];
	int checkpipe = pipe(newpipe);
	if ( checkpipe == -1 ){
		perror( "ERROR: <Pipe failed>" );
		return;
	}
}

int main(int argc, char const *argv[])
{
	int pipe_[2];
	int check = pipe(pipe_);
	my_pipe(0,0);
	return 0;
}