#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
// max covered 
int max_square = 0;
#define limit 2048000
int ini_num = 1024;
// the struct for dead_end_board
typedef struct dead_end_board {
	char** board; // the three int can be changed 
	int covered;
} Board;
 // packet sent to child
typedef struct packet{
	Board* my_board;
	int* record;
	char** current;
	int m, n, cur_cov;
	int x, y;
} pac;
// There are for find moves 
static int xMove[8] = {  2, 1, -1, -2, -2, -1,  1,  2 };
static int yMove[8] = {  1, 2,  2,  1, -1, -2, -2, -1 };
typedef struct possible_move { // record possible move
	int move[8][2];
	int alter;
} p_m;
// search for next possible move
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

void print_vec(char** vec_2d,int row_m,int col_n){ // print_out 2d vector
	for( int i = 0; i < row_m; i++){
		printf("THREAD %u:", (unsigned int)pthread_self());
		if ( i == 0){
			printf(" > ");
		} else {
			printf("   ");
		}
		for(int j = 0; j < col_n; j++){
			printf("%c", vec_2d[i][j]);
		}
		printf("\n");
	}
	fflush(stdout);
}

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y)) // self defined max function


// a fucntion to read files 
void readfile( int* m, int *n, int *k, int argc, char const *argv[]){
	if (argc < 3){
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<k>]\n");
		exit(EXIT_FAILURE);
	} else if(argc > 3) {
		*k = atoi(argv[3]);
		*m = atoi(argv[1]);
		*n = atoi(argv[2]);
		if( *k > (*m)*(*n) || *k == 0 ) {
			fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<k>]\n");
			exit(EXIT_FAILURE);
		}
		
	} else if (argc == 3){
		*m = atoi(argv[1]);
		*n = atoi(argv[2]);
		*k = -1;
	} 
	if (  *m <= 2 || *n <= 2){
			fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<k>]\n");
			exit(EXIT_FAILURE);
	}
	return;
}

void print_board( Board* my_b,int r, int k, int m, int n ){
		for( int i = 0; i < r; i++){
			if ( my_b[i].covered >= k){
				print_vec( my_b[i].board, m, n);
			}
		}
}

// for semaphore mutex
int mutex = 1;
void p(int* mutex){
	while(*mutex == 0);
	(*mutex)--;
}
// release opreation
void v(int* mutex){
	(*mutex)++;
}
// free a 2d vec
void free_vec( char*** vec_2d, int m, int n){
	for(int k2 = 0; k2 < m; k2++){
		free((*vec_2d)[k2]);
		(*vec_2d)[k2] = NULL;
	}
	free(*vec_2d);
}

// copy a 2d vec on heap
char** copy_2d( int m, int n, char** vec_2d){
	char** tmp_vec = malloc(sizeof(char*)*m);
	for(int i1 = 0; i1 < m; i1++){
		tmp_vec[i1] = malloc(sizeof(char)*n);
		for(int i2 = 0; i2 < n; i2++){
			tmp_vec[i1][i2] = vec_2d[i1][i2];
		}
	}
	return tmp_vec;
}


void* dojob(void* arg){
	pac mp1 = *(pac*) arg;
	
	if( mp1.cur_cov <= mp1.m * mp1.n ){
			p_m tmp_pm = search(mp1.x, mp1.y, mp1.m , mp1.n , mp1.current);
			while( tmp_pm.alter == 1){
				mp1.x = tmp_pm.move[0][0];
				mp1.y = tmp_pm.move[0][1];
				mp1.current[mp1.x][mp1.y] = 'k';
				mp1.cur_cov++;
				tmp_pm = search(mp1.x, mp1.y, mp1.m , mp1.n , mp1.current);
			}

			if( tmp_pm.alter == 0){
				// add to the dead end board
			p(&mutex);
				(mp1.my_board)[*(mp1.record)].board = mp1.current;
				(mp1.my_board)[*(mp1.record)].covered = mp1.cur_cov;
				(*mp1.record)++;
				if( *(mp1.record) >= ini_num){
					mp1.my_board = realloc(mp1.my_board, sizeof(Board)*(*(mp1.record))*2);
				}
				max_square = MAX(mp1.cur_cov, max_square);
			v(&mutex);
				printf("THREAD %u: Dead end after move #%d\n" ,  (unsigned int)pthread_self() , mp1.cur_cov);
				return NULL;			
			} else if ( tmp_pm.alter > 1 ){
				// prepare for threads 
				printf("THREAD %u: %d moves possible after move #%d; creating threads\n", 
					(unsigned int)pthread_self(), tmp_pm.alter, mp1.cur_cov);
				pthread_t tid[tmp_pm.alter];
				int rc;
				char** tmp_vec1;
				pac my_pac1[tmp_pm.alter];
				for ( int i = 0; i < tmp_pm.alter; i++){
					//set up current vec 
					if( i == tmp_pm.alter){
						tmp_vec1 = mp1.current;
					} else {
						tmp_vec1 = copy_2d(mp1.m, mp1.n, mp1.current);
					}
					// printf("%p\n", tmp_vec1);
					int cur_x = tmp_pm.move[i][0];
					int cur_y = tmp_pm.move[i][1];
					tmp_vec1[cur_x][cur_y] = 'k';
					//set up mypac
					my_pac1[i] = mp1;
					my_pac1[i].current = tmp_vec1;
					my_pac1[i].x = cur_x;
					my_pac1[i].y = cur_y;
					my_pac1[i].cur_cov ++;
					// let's go thread 
					rc = pthread_create(&tid[i], NULL, dojob, &my_pac1[i]);
					if ( rc < 0)
					{
						perror("<ERROR> System call pthread_create failed");
					}
				}
				free_vec(&mp1.current, mp1.m, mp1.n);
				// join thread 
				for (int i = 0; i < tmp_pm.alter; i++){
					rc = pthread_join(tid[i], NULL);
					if( rc != 0){
						fprintf(stderr, "Main: Could not join thread %u\n", (unsigned int) pthread_self());
					}
				}
			} 
	}
	return NULL;
}

int main(int argc, char const *argv[])
{
	int row, col, kval; // k value is kval
	readfile(&row, &col, &kval, argc, argv ); // read values of important variables 

	// create board
	int ini = 0;
	int* record = &ini;
	Board* my_board = malloc(sizeof(Board)*ini_num);

	// set up packet as the argv to thread 
	pac my_pac;
	my_pac.my_board = my_board;
	my_pac.m = row;
	my_pac.n = col;
	my_pac.x = 0;
	my_pac.y = 0;

	//set up board 
	char** vec_2d = malloc(sizeof(char*)*row);
	for(int i1 = 0; i1 < row; i1++){
		vec_2d[i1] = malloc(sizeof(char)*col);
		for(int i2 = 0; i2 < col; i2++){
			vec_2d[i1][i2] = '.';
		}
	}
	vec_2d[0][0] = 'k';


	// print_vec(vec_2d, row, col);
	// current covered 
	int covered = 1;

	// search for steps 
	int cur_x = 0;
	int cur_y = 0;
	// tmp_pm, temporary possible move
	p_m tmp_pm = search(cur_x,cur_y, row, col, vec_2d);
	printf("THREAD %u: Solving the knight's tour problem for a %dx%d board\n",(unsigned int)pthread_self(), row, col);
	if( tmp_pm.alter == 0 ) {
		// it means bye bye, it's over 
		printf("THREAD %u: Dead end after move #%d\n", (unsigned int)pthread_self(), covered);
		return 0;

	} else if ( tmp_pm.alter > 0 ){
		// prepare for threads 
		printf("THREAD %u: %d moves possible after move #%d; creating threads\n", 
			(unsigned int)pthread_self(), tmp_pm.alter, covered);
		pthread_t tid[tmp_pm.alter];
		int rc = 0;
		// set up packet
		my_pac.cur_cov = covered;
		my_pac.current = vec_2d;
		char** tmp_vec1;
		pac my_pac1[tmp_pm.alter];
		// for loop
		for ( int i = 0; i < tmp_pm.alter; i++){
			//set up current vec 
			if( i == tmp_pm.alter){
				tmp_vec1 = vec_2d;
			} else {
				tmp_vec1 = copy_2d(row, col, vec_2d);
			}
			
			// printf("%p\n", tmp_vec1);
			cur_x = tmp_pm.move[i][0];
			cur_y = tmp_pm.move[i][1];
			tmp_vec1[cur_x][cur_y] = 'k';
			//set up mypac
			my_pac1[i] = my_pac;
			my_pac1[i].current = tmp_vec1;
			my_pac1[i].x = cur_x;
			my_pac1[i].y = cur_y;
			my_pac1[i].cur_cov ++;
			max_square = my_pac1[i].cur_cov;
			// let's go thread 
			rc = pthread_create(&tid[i], NULL, dojob, &my_pac1[i]);
			if ( rc < 0)
			{
				perror("<ERROR> System call pthread_create failed");
			}
		}
		// free the vec
		// free_vec(&vec_2d, row, col);
		#if 1
		// join thread 
		for (int i = 0; i < tmp_pm.alter; i++){
			rc = pthread_join(tid[i], NULL);
			if( rc != 0){
				fprintf(stderr, "Main: Could not join thread %u\n", (unsigned int) pthread_self());
			}
		}
		#endif

	} else {
		printf("That's proposterous, you idiot!\n");
	}
	printf("THREAD %u: Best solution found visits %d squares (out of %d)\n", (unsigned int)pthread_self(),
	 max_square, row*col);
	p(&mutex);
	print_board( my_board, *record, kval, row, col );
	v(&mutex);
	// free dynamic memeory 
	for(int i =0; i < *record; i++){
		free_vec(&(my_board[i].board), row, col);
	}


	return 0;
}