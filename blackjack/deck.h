#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

class deck {
public:
	deck(int num){
		for (int i = 0; i < num*13; i++){
			this->all_cards.push_back( i%13 + 1 );
			this->count++;
		}
		shuffle(all_cards);
	}

	void print_deck(){
		for(int i = 0; i < all_cards.size(); i++){
			cout << this->all_cards[i] << " ";
		}
		cout << endl;
	}

	int distribute_card(){
		int tmp = -1;
		vector<int>::iterator itr = all_cards.begin();
		if(all_cards.size() == 0){
			cout << "Cards are not enough" << endl;
			exit(1);
		}
		if(count <= 0){
			shuffle(all_cards);
			count = all_cards.size();
		}

		tmp = all_cards.front();
		all_cards.erase(itr);
		on_deck.push_back(tmp);
		count--;
		return tmp;

	}

	void One_Game_end(){
		all_cards.insert(all_cards.end(), on_deck.begin(), on_deck.end());
		on_deck.clear();
	}


private:
	void shuffle( std::vector<int>& cards ){
		srand((unsigned)time(NULL));
		if (cards.size() > 1){
			for(int i = cards.size() - 1; i > 0; i--){
				int index = rand() % (i+1) ;
				if( index != i ){
					int temp = cards[index];
					cards[index] = cards[i];
					all_cards[i] = temp;
				}
			}
		}
	}
	vector<int> all_cards;
	vector<int> on_deck;
	int count;
};