#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
#include <sys/time.h>
#include <sys/resource.h>

// move combination
static int xMove[8] = {  2, 1, -1, -2, -2, -1,  1,  2 };
static int yMove[8] = {  1, 2,  2,  1, -1, -2, -2, -1 };
typedef struct possible_move { // record possible move
	int move[8][2];
	int alter;
} p_m;
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y)) // self defined max function
int debug = 0;

void print_vec(char** vec_2d,int row_m,int col_n){ // print_out 2d vector
	int i;
	for(i = 0; i < row_m; i++){
		int j;
		printf("PID %d:   ", getpid());
		for(j = 0; j < col_n; j++){
			printf("%c", vec_2d[i][j]);
		}
		printf("\n");
		fflush(stdout);
	}
}

int isSafe(int x, int y, int row, int col, char** vec_2d){ // if the next move is valid
	return ( x >= 0 && x < row && y >=0 && y < col && vec_2d[x][y] == '.');
}

p_m search(int x, int y, int row, int col, char** vec_2d){ // search for next possible moves and then 
	int i ;												   // store it in the struct 
	p_m next_move;
	next_move.alter = 0;
	for(i = 0; i < 8; i++){
		int nextx = x + xMove[i];
		int nexty = y + yMove[i];
		if(isSafe(nextx, nexty, row, col, vec_2d)){
			next_move.move[next_move.alter][0] = nextx;
			next_move.move[next_move.alter][1] = nexty;
			next_move.alter++;
		}
	}
	return next_move;
}

// see the name
void free_vec(char** vec_2d, int row_m){
	int k2;
	if(vec_2d){
		for(k2 = 0; k2 < row_m; k2++){
			free(vec_2d[k2]);
			vec_2d[k2] = NULL;
		}
		free(vec_2d);
	}
	return;
}

// function to solve the knight tour problem
void Solve_problem(char** vec_2d, int loc_x, int loc_y,
	int outpipe[2], int row, int col, int ind){
//check if it's the goal state =======================================
	p_m tmp_pm = search(loc_x, loc_y, row, col, vec_2d);
	if(ind == 1){
		printf("PID %d: Solving the knight's tour problem for a %dx%d board\n", getpid(), row, col);
		fflush(stdout);
	}
	if(tmp_pm.alter == 0){ // reach the end, return 
		printf("PID %d: Dead end after move #%d\n", getpid(), ind);
		fflush(stdout);
#if DISPLAY_BOARD
	    print_vec(vec_2d, row,col);
	    fflush(stdout);
#endif
		printf("PID %d: Sent %d on pipe to parent\n", getpid(), ind);
		fflush(stdout);
		int byte_write = write(outpipe[1], &ind, sizeof(int));
		if(byte_write != sizeof(int)){
			perror("The pipe is not right\n");
		}
		// free memory
		return;

	}
//=====================================================================
	else if(tmp_pm.alter == 1){
		loc_x = tmp_pm.move[0][0];
		loc_y = tmp_pm.move[0][1];
		vec_2d[loc_x][loc_y] = 'k';
		Solve_problem(vec_2d, loc_x, loc_y, outpipe, row, col, ind+1);
	} else if(tmp_pm.alter > 1){
		printf("PID %d: %d moves possible after move #%d\n", getpid(), tmp_pm.alter, ind);
		fflush(stdout);
#if DISPLAY_BOARD
	    print_vec(vec_2d, row,col);
	    fflush(stdout);
#endif		
		int k1;
		int max_read = 0;
		
		for(k1 = 0; k1 < tmp_pm.alter; k1++){
			// create pipe
			int newpipe[2];
			int checkpipe = pipe(newpipe);
			if ( checkpipe == -1 ){
   				perror( "ERROR: <Pipe failed>" );
   				return;
  			}
  			//fork
  			pid_t new_pid = fork();
  			if (new_pid < 0){
				perror("ERROR: <Fork failed>\n");
				return;
			} else if( new_pid == 0 ){
				loc_x = tmp_pm.move[k1][0];
				loc_y = tmp_pm.move[k1][1];
				vec_2d[loc_x][loc_y] = 'k';
				Solve_problem(vec_2d, loc_x, loc_y, newpipe, row, col, ind+1);
				return;
			} else {
#if NO_PARALELL
				wait(NULL);
#endif	
				// gotta read
				close(newpipe[1]);
				int int_read;
				int tmp_read;
				int_read = read(newpipe[0], &tmp_read, sizeof(int));
				if(int_read != sizeof(int)){
					perror("Read ERROR\n");
					fflush(stdout);
				}
				printf("PID %d: Received %d from child\n", getpid(), tmp_read);
				fflush(stdout);
				max_read = MAX(max_read, tmp_read);
			}	
		}
		//write
		int byte_write = write(outpipe[1], &max_read, sizeof(int));
		if(byte_write != sizeof(int)){
			perror("The pipe is not right\n");
		}
		// free memory
		if(ind > 1){
			printf("PID %d: All child processes terminated; sent %d on pipe to parent\n", getpid(), max_read);
			fflush(stdout);
		}
		return;
	}
}


int main(int argc, char const *argv[])
{
  struct rlimit rl;
  getrlimit( RLIMIT_NPROC, &rl );
  rl.rlim_cur = rl.rlim_max = 1000;
  setrlimit( RLIMIT_NPROC, &rl );

	if(argc != 3){ // check number of arguments
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n>\n");
		return EXIT_FAILURE;
	}
	int row_m = atoi(argv[1]); // the row number of array
	int col_n = atoi(argv[2]); // the column number of array 

	if( !(row_m > 2 && col_n > 2) ){ // row and column must be greater than 2 
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n>\n");
		return EXIT_FAILURE;
	}
		// construct the array
	char** vec_2d = malloc(sizeof(char*)*row_m);
	int i1;
	for(i1 = 0; i1 < row_m; i1++){
		vec_2d[i1] = malloc(sizeof(char)*col_n);
		int i2;
		for(i2 = 0; i2 < col_n; i2++){
			vec_2d[i1][i2] = '.';
		}
	}
	vec_2d[0][0] = 'k';

	// we need a pipe everytime we use fork
	int outpipe[2];
	int checkPipe = pipe(outpipe);
	if ( checkPipe == -1 ){
   		perror( "ERROR: <Pipe failed>" );
   		return EXIT_FAILURE;
  	}

  	pid_t pid_1 = fork();
  	if(pid_1 < 0){
  		perror("Fork failed\n");
  		return EXIT_FAILURE;
  	}

  	if(pid_1 == 0){
	  	int loc_y = 0;
	  	int loc_x = 0;
	  	Solve_problem(vec_2d, loc_x, loc_y, outpipe, row_m, col_n,1);
	  	free_vec(vec_2d, row_m);
	} else {
	  	//read the pipe;
#if NO_PARALELL
				wait(NULL);
#endif	
	  	close(outpipe[1]);
	  	int int_read;
		int tmp_read;
		int max_read = 0;
		while( ( int_read = read(outpipe[0], &tmp_read, sizeof(int)) ) > 0 ){
			// printf("PID %d: Received %d from child\n", getpid(), tmp_read);
			fflush(stdout);
			max_read = MAX(max_read, tmp_read);
		}
		printf("PID %d: Best solution found visits %d squares (out of %d)\n", getpid(), max_read, col_n*row_m);
		fflush(stdout);

		free_vec(vec_2d, row_m);
	}
	return 0;
}

