#include <vector>


class player{
public:
	vector<int> card_hold;
	int s_count;
	int point;

	player(){
		point = 0;
		s_count = 0;
	}

	void get(int p){
		card_hold.push_back(p);
		cal_point();
	}

	void print(){
		for (int i = 0 ; i < card_hold.size(); i++){
			cout << "[" << card_hold[i] << "] ";
		}
		cout << endl << "Your total point: " << point << endl;
 	}

	void cal_point(){
		point = 0;
		s_count = 0;
		for (int i = 0; i < card_hold.size(); i++){
			int n = card_hold[i];
			if(n == 11 || n == 12 || n == 13){
				point += 10;
			} else if (n == 1){
				s_count++;
				point += 11;
			} else {
				point += n;
			}
		}
		while (point > 21){
			if(s_count == 0) break;
			point -= 10;
			s_count--;
		}
	}

};