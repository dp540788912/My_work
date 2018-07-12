#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#define test 100

void rm_symbol(char x[]){
	int i = 0; int j = 0;
	for(; x[i] != '\0'; i++){
		if(isalnum(x[i])){
			x[j] = x[i];
			j++;
		}
	}
	x[j] = '\0';
}


int main(int argc, char const *argv[])
{
	// FILE * r = fopen("ftest.txt", "r");
	// char my[80];
	// while(fscanf(r, "%s", my)!= EOF){
	// 	printf("%s ", my);
	// }
	// printf("\n");
	// struct hehe {
	// 	char x[30];
	// 	int k;
	// };

	// struct hehe* my_test = malloc(10*sizeof(struct hehe));
	// printf("the sieze if hehe: %d\n", (int)sizeof(struct hehe));

	// for (int i = 0; i < 10; i++){
	// 	strcpy(my_test[i].x, "the i element");
	// 	my_test[i].k = i*i;
	// 	printf("the char: %s\n", my_test[i].x);
	// 	printf("the int: %d\n", my_test[i].k);
	// }

	// printf("\n\n\n\nthe second time \n");
	// my_test = realloc(my_test, 20*sizeof(struct hehe));
	// for (int i = 0; i < 10; i++){
	// 	printf("the char: %s\n", my_test[i].x);
	// 	printf("the int: %d\n", my_test[i].k);
	// }

	// free(my_test);
	char x[] = "my*ba@#ad/''";
	
	rm_symbol(x);
	printf("the x is : %s\n", x);


	return 0;
}