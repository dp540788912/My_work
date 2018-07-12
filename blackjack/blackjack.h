#ifndef _blackjack_
#define _blackjack_

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include "deck.h"
#include "player.h"
using namespace std;

string getname(){
	//have name 
	cout << "What is your name?\nYour name:";
	string name;
	cin >> name;
	return name;
} 

int getnum(){
	cout << "How many decks do you want to play?" << endl;
	int num;
	cin >> num;
	while (num > 10 || num < 1){
		cout << "Not available, please enter number again" << endl;
		cin >> num;
	}
	return num;
}

void ReadyToBegin(){
	cout << "Game will begin soon, Enter [y] if you are ready" << endl;
	string temp;
	while(1){
		cin >> temp;
		if(temp == "y")break;
	}
}


void RunGame(int num){

	deck game_deck(num);
	player p1;

	p1.get(game_deck.distribute_card());
	p1.get(game_deck.distribute_card());

	p1.print();

	// while (1){
	// 	cout << "give order, stop [s], or hit [h]" << endl;
	// 	string order;
	// 	here:
	// 	cin >> order;

	// 	if(order == "s"){
	// 		p1.print();
	// 	}
	// 	else if (order == "h"){
	// 		p1.get(game_deck.distribute_card());
	// 		p1.print();
	// 	}
	// 	else {
	// 		goto here;
	// 	}

	// 	if (p1.point > 21){
	// 		cout << "boom you idiot" << endl;
	// 		p1.print();
	// 		break;
	// 	}
	// }
}



#endif