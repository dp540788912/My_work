#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>


#define max_length 80
#define array_increment 32
#define limit 5

// ================================================================
// ===============  helpful function Declaration    ==============
// ================================================================

typedef struct word_and_count
{
	char word[max_length];
	int count;
} wc;
// ================================================================
// ===============          implemention             ==============
// ================================================================
int sameStr(const char* a, const char* b){
	for(; *a == *b; ++a, ++b)
		if (*a == '\0' && *b == '\0') return 1;
	return 0;
}


void rm_symbol(char x[limit][max_length],char ori[], int* num){
	int i = 0; int j = 0; int number = 0;
	while(ori[i] != '\0'){
		while(!isalnum(ori[i]) && ori[i] != '\0')i++;
		if(ori[i] == '\0')break;
		while(isalnum(ori[i])){
			x[number][j++] = ori[i];
			i++;
		}
		x[number][j] = '\0';
		number++;
		j = 0;
		if(ori[i] == '\0')break;
		i++;
	}
	if(number > limit){
		fprintf(stderr, "ERROR<5 words in a row>\n");
		exit(1);
	}
	*num = number;
}

int ifRepeat(char* word, wc* w_array, int* order, int unique){
	int i;
	for(i = 0; i < unique; i++){
		if(sameStr(word, w_array[i].word)){
			*order = i;
			return 1;
		}
	}
	return 0;

}

void print_out(wc* words, int num){
	printf("All words (and corresponding counts) are:\n");
	int i;
	for(i = 0; i < num; i++){
		printf("%s -- %d\n", words[i].word, words[i].count);
	}
}

void print_out_n(wc* words, int n, int all){
	printf("First %d words (and corresponding counts) are:\n", n);
	fflush(stdout);
	int i;
	for(i = 0; i < n; i++){
		printf("%s -- %d\n", words[i].word, words[i].count);
	}
	printf("Last %d words (and corresponding counts) are:\n", n);
	fflush(stdout);
	int j;
	for(j = all-n; j < all; j++){
		printf("%s -- %d\n", words[j].word, words[j].count);
	}

}

void find_files(const char* directory, char *** filenames, int* num_file){
	DIR *dirptr = NULL;
	int i = 1;
	struct stat st;
	struct dirent *dent;

	if((dirptr = opendir(directory)) == NULL){
		// if it can't be open, give error
		fprintf(stderr, "ERROR <opendir failed>\n");
		return ;
	} else {

		while ((dent = readdir(dirptr)) != NULL){
			lstat(dent->d_name, &st);
			
			// if it's directory, continue
			if (!S_ISREG(st.st_mode)){

			// if it's regular 
				if(sameStr(dent->d_name, ".") || sameStr(dent->d_name,".."))
					// if it's . or .., ignore
					continue;
				// if it's not txt file, ignore.
				(*filenames) = realloc((*filenames), i*sizeof(char*));
				(*filenames)[i-1] = malloc(max_length*sizeof(char));
				int q = sprintf((*filenames)[i-1], "%s", directory);
				strcpy((*filenames)[i-1]+q, "/");
				strcpy((*filenames)[i-1]+1+q, dent->d_name);
				//record the directory
				i++;
			}
		}
		*num_file = i-1;
		closedir(dirptr);
	}
}

// ================================================================
// ===============         main function             ==============
// ================================================================

int main(int argc, char const *argv[])
{
//========= deal with the arguments ===================
	int argv_num = 0;
	// the third argument
	if(argc < 2){
		fprintf(stderr, "ERROR <arguments are not enough>");
	} else if (argc == 3){
		argv_num = atoi(argv[2]);
	}
	

	char** filenames = malloc(sizeof(char*));
	// the array of filenames 
	char path[max_length];
	strcpy(path, argv[1]);
	// the path as arhument 
	int num_file = 0;
	// the number of txt file
	find_files(path, &filenames, &num_file);
	// if the file number == 0, error
	if(num_file == 0) {
		fprintf(stderr, "ERROR <no txt file exists>\n");
		free(filenames);
		return 1;
	}

	//===========   main part =========================
	int word_total = 0;
	int word_unique = 0;
	int volume = array_increment;
	wc* w_array = malloc(volume*sizeof(wc));// an array to store the words and count

	printf("Allocated initial parallel arrays of size %d.\n", volume);
	fflush(stdout);
	// check each txt file
	int i ;
	for(i = 0; i < num_file; i++){ // loop over all the files
		char tmp[max_length];
		FILE * r = fopen(filenames[i], "r");
		if(r == NULL) {
			fprintf(stderr, "ERROR: <openning file failed>\n");
		}
		while(fscanf(r, "%s", tmp)!= EOF){	
			char tmp2[limit][max_length];
			int indicator;
			rm_symbol(tmp2, tmp, &indicator);
			// printf("%s ", tmp);
			// see if the size bigger than 1
			int k = 0;
			for(k = 0; k < indicator; k++  ){
				if(strlen(tmp2[k]) <= 1) continue;
				int order = 0;
				if(ifRepeat(tmp2[k], w_array, &order,word_unique)){
					w_array[order].count++;
					word_total++;
				} else {
					if(volume == word_unique){
						//resize
						volume = volume + array_increment;
						w_array = realloc(w_array, volume*sizeof(wc));
						printf("Re-allocated parallel arrays to be size %d.\n", volume);
						fflush(stdout);
					}
					//add word
					word_unique++;
					word_total++;

					strcpy(w_array[word_unique - 1].word, tmp2[k]);
					w_array[word_unique-1].count = 1;
				}
			}
		}
		// printf("\n");
		fclose(r);
		free(filenames[i]);
	}
	// print out 
	printf("All done (successfully read %d words; %d unique words).\n",
			word_total, word_unique);
	fflush(stdout);
	if(argv_num == 0){
		print_out(w_array, word_unique);
	} else if(argv_num > word_unique){
		fprintf(stderr, "ERROR: <Exceeds number of unique words>\n");
		return 1;
	} else if(argv_num < 0){
		fprintf(stderr, "ERROR: <incorrect arguments>\n");
	} else {
		print_out_n(w_array, argv_num, word_unique);
	}
	// free memory
	free(w_array);
	free(filenames);

	return 0;
}
