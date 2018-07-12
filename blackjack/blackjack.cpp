#include "blackjack.h"
using namespace std;

int main(int argc, char const *argv[])
{
	
	// get name 
	string guest_name = getname();
	// get cards
	int deck_num = getnum();
	// begin to start 
	ReadyToBegin();
	// begin game 
	RunGame(deck_num);
	


	return 0;
}