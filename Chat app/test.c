
#include "util.h"


struct my{
	char* a[4];
	int b;
	int c;
};

 


int main(int argc, char const *argv[])
{
	char* ori = "cao ni ma de sha bi er zi\n";

	int k = 0;
	char x[MAX_CLIENT][MAX_MSGLEN_REAL];

	rm_symbol(x, ori, &k);

	for (int i =0; i < k;i++) {
		printf("%s\n", x[i]);
	}
 	

	return 0;
}