
/**
 * codeVS 2.0 
 * Author :tokoharu
 */
#include <algorithm>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <fstream>
#include <vector>



using namespace std;

typedef long long LL;

//デバッグ用
ofstream ofs("errprint.out");
bool debug = false;


const int INF = 100000000;
const int EMPTY = 10000;

int*** pack;
int WIDTH , HEIGHT , SIZE , STEP , SUM;
int HS,HWS; // HS = HEIGHT+SIZE , HWS = HS+WIDTH-1;
int allLength[4]; //WIDTH , HS,HWS , HWSがはいっているようにする
int NUMP ;

bool isMain;

bool valid(int y , int x) {
	if(0>y) return false;
	if(0>x) return false;
	if(y>=HS) return false;
	if(x>=WIDTH ) return false;
	return true;
}
void rotatePack(int** &droppingPack) {
	int tmp[SIZE][SIZE];
	for(int i=0; i<SIZE; i++) for(int j=0; j<SIZE; j++) tmp[i][j] = droppingPack[i][j];

	for(int i=0; i<SIZE; i++) for(int j=0; j<SIZE; j++) {
		droppingPack[i][j] = tmp[SIZE-1-j][i];
	}
	
}




//---------State class----------------------------------------------------------------------------------//

class State{
  
public :
  int** field;
  LL score;
  int turn;
  
  vector<int> cntPrevEraseBlocks;
  
  vector<pair<int,int> > outputData;  
  pair<LL,pair<int,pair<int,int> > > highscore;
  
  void printForDebug();
  void eraseBlockAndUpdateScore();
  bool updateNowState(int xpos ,int rot ) ;
  
  void createStateField();
  void deleteStateField();
  
  void initializeSearchingState() {
    for(int j=0; j<4; j++) {
      int maxim = allLength[j];
      for(int i=0; i<maxim; i++) isNeededToSearch[j][i] = false;
    }
  }
  void addCoordinate(int posy , int posx) {
    int nums[4] = {posx , posy , posy+posx , posy-posx+WIDTH-1};
    int pos[4]  = {posy , posx , posy , posy};
    for(int i=0; i<4; i++) {
      int index = nums[i];
      isNeededToSearch[i][index] = true;
      searchMaxPos[i][index] = max(searchMaxPos[i][index] , pos[i]) ;
      searchMinPos[i][index] = min(searchMinPos[i][index] , pos[i]) ;
    }
  }
  bool isNeededSearching(int pat , int pos) {return isNeededToSearch[pat][pos];}
  int getSearchMaxPos(int pat , int pos) {return searchMaxPos[pat][pos];}
  int getSearchMinPos(int pat , int pos) {return searchMinPos[pat][pos];}
  
  
private :
  bool* isNeededToSearch[4];
  int* searchMaxPos[4];
  int* searchMinPos[4];
  
  void eraseAndDrop(vector<int> &eraseHeight);
  void eraseBlocks(int &cntEraseBlock);
  bool dropPack(int xpos , int** &droppingPack);
};

void State::printForDebug() {
  /*	if(!isMain) return ;
	
	ofs<<turn<<" "<<score<<endl;
	for(int i=HEIGHT+SIZE-1; i>=0; i-- , ofs<<endl) for(int j=0; j<WIDTH; j++) {
		if(field[i][j]!=EMPTY) ofs<<field[i][j]<<" ";
		else                      ofs<<0<<" ";
	}
	ofs<<endl;*/
}

void State::createStateField() {
  //  highscore = make_pair(0,make_pair(0,make_pair(0,0)));
  //  for(int i=0; i<STEP; i++) 
  // outputData.push_back(pair<int,int>(0,0));
  for(int i=0; i<4; i++) {
    isNeededToSearch[i] = new bool[allLength[i]];
    searchMaxPos[i] = new int[allLength[i]];
    searchMinPos[i] = new int[allLength[i]];
  }
  
  initializeSearchingState();
  
  field = new int*[HEIGHT+SIZE] ;
  for(int i=0; i<HEIGHT+SIZE; i++) {
    field[i] = new int[WIDTH];
    for(int j=0; j<WIDTH; j++) {
      field[i][j] = EMPTY;
    }
  }
	
}

void State::deleteStateField() {
	for(int i=0; i<4; i++) {
		delete[] isNeededToSearch[i];
		delete[] searchMaxPos[i];
		delete[] searchMinPos[i];
	}
	for(int i=0; i<HEIGHT+SIZE; i++){
		delete[] field[i];
	}
	delete[] field;
}



bool State::dropPack(int xpos , int** &droppingPack) {
	
	for(int i=SIZE-1; i>=0; i--) for(int j=0; j<SIZE; j++) if(droppingPack[i][j]>0) { 
		
		int y;
		if(!valid(0,xpos+j)) return true;
		for(y=HEIGHT+SIZE-1; y>0 && field[y-1][xpos+j]==EMPTY; y--) ;
		
		field[y][xpos+j] = droppingPack[i][j];
		addCoordinate(y,xpos+j);
 	}
 	return false;
}

void State::eraseAndDrop(vector<int> &eraseHeight) {
	for(int j=0; j<WIDTH; j++) {
		
		
		int posy = eraseHeight[j];
		for(int i=posy; i<HEIGHT+SIZE; i++) {
			field[posy][j] = field[i][j];
			if(field[i][j]!=EMPTY) {
				if(posy!=i) addCoordinate(posy,j);
				posy++;
			}
		}
		for(; posy<HEIGHT+SIZE; posy++) field[posy][j] = EMPTY;
	}
	
}
/*
void nextNxNy(int &nx ,int &ny) {
	switch(pat) {
		case 0 : nx++;
		case 1 : ny++;
		case 2 : nx++ , ny--;
		case 3 : nx-- , ny--;
	}
}

void nextLxLy(int &leftX , int &leftY) {
	case 0 : nx--;
	case 1 : ny--;
	case 2 : nx-- , ny++;
	case 3 : nx++ , ny++;
	
}

void State::eraseBlocksInner(int pat , int sx , int sy , vector<int> &eraseposy , vector<int> &eraseposx) {
	
	int ny = sy , nx = sx;
	int leftY = sy , leftX = sx;
	int sum = 0;
	for(; valid(ny,nx); nextNxNy(pat,nx,ny))) {
		sum += field[ny][nx];
		while(sum>SUM) {
			sum -= field[leftY][leftX];
			nextLxLy(leftX , leftY);
		}
		if(sum==SUM) for(int k=0; k<=nx-leftX; k++) {
//			isErase[leftY-k][leftX+k] = 1 ;
			eraseposx.push_back(leftX+k);
			eraseposy.push_back(leftY-k);
			cntEraseBlock++;
		}
	}
}
*/

//消して落す処理
void  State::eraseBlocks(int &cntEraseBlock) {
	cntEraseBlock = 0;
	vector<int> eraseposy;
	vector<int> eraseposx;
	
	
	for(int i=0; i<HS; i++) if(isNeededSearching(1,i)) {
		int leftJ = 0;
		int sum = 0;
		for(int j=leftJ; j<WIDTH  ; j++) {
			sum += field[i][j];
			while(sum>SUM) {
				sum -= field[i][leftJ];
				leftJ++;
			}
			if(sum==SUM) for(int k=leftJ; k<=j; k++) {
//				isErase[i][k] = 1 ;
				eraseposx.push_back(k);
				eraseposy.push_back(i);
				cntEraseBlock++;
			}
		}
	}
	
	
	for(int j=0; j<WIDTH; j++) if(isNeededSearching(0,j)){
		int leftI = 0;
		int sum = 0;
		for(int i=0; i<HS; i++) {
			if(field[i][j]==EMPTY) break;
			sum += field[i][j];
			while(sum>SUM) {
				sum -= field[leftI][j];
				leftI++;
			}
			if(sum==SUM) for(int k=leftI; k<=i; k++) {
//				isErase[k][j] = 1 ;
				eraseposx.push_back(j);
				eraseposy.push_back(k);
				cntEraseBlock++;
			}
		}
	}
	
	int sx,sy;
	
	sy=0 , sx=0;
	for(int i=0; i<HWS; i++) {
		if(isNeededSearching(2,i)) {
			
			int ny = sy , nx = sx;
			int leftY = sy , leftX = sx;
			int sum = 0;
			for(; nx<WIDTH && ny>=0; nx++ , ny--) {
				sum += field[ny][nx];
				while(sum>SUM) {
					sum -= field[leftY][leftX];
					leftY--;
					leftX++;
				}
				if(sum==SUM) for(int k=0; k<=nx-leftX; k++) {
//					isErase[leftY-k][leftX+k] = 1 ;
					eraseposx.push_back(leftX+k);
					eraseposy.push_back(leftY-k);
					cntEraseBlock++;
				}
			}
		}
		
		if(i<HEIGHT+SIZE-1) sy++;
		else sx++; 
	}
	sy=0 , sx=WIDTH-1;
	for(int i=0; i<HWS; i++) {
		if(isNeededSearching(3,i)) {
			int ny = sy , nx = sx;
			int leftY = sy , leftX = sx;
			int sum = 0;
			for(; nx>=0 && ny>=0; nx-- , ny--) {
				sum += field[ny][nx];
				while(sum>SUM) {
					sum -= field[leftY][leftX];
					leftY--;
					leftX--;
				}
				if(sum==SUM) for(int k=0; k<=leftY-ny; k++) {
//					isErase[leftY-k][leftX-k] = 1;
					eraseposx.push_back(leftX-k);
					eraseposy.push_back(leftY-k);
					cntEraseBlock++;
				}
			}
		}
		if(i<HEIGHT+SIZE-1) sy++;
		else sx--; 
	}
	
	initializeSearchingState();
	
	vector<int> eraseHeight;
	for(int i=0; i<WIDTH; i++) eraseHeight.push_back(SIZE + HEIGHT);
	int esize = (int)eraseposx.size();
	for(int i=0; i<esize; i++) {
		int y = eraseposy[i];
		int x = eraseposx[i];
		field[y][x] = EMPTY;
		eraseHeight[x] = min(eraseHeight[x],y);
	}
	eraseAndDrop(eraseHeight);
	
}


void State::eraseBlockAndUpdateScore( ) {
	cntPrevEraseBlocks.clear();
	LL keisu = 1;
	LL hoge = 1;
	int step = 0;
	while(1) {
		int cnt;
		eraseBlocks(cnt);
		if(cnt==0) break;
		hoge = max(1,step-NUMP-turn/100+1);
		score += (LL) cnt * keisu * hoge;
		
		cntPrevEraseBlocks.push_back(cnt);
	       
						if(NUMP+turn/100>step)
		keisu *= 2;
		step++;
	}
	
}
bool State::updateNowState(int xpos ,int rot ) {
	if(debug) cout<<xpos<<" "<<rot<<endl;
	
	
	int** droppingPack = new int*[SIZE];
	for(int i=0; i<SIZE; i++) {
		droppingPack[i] = new int[SIZE];
		for(int j=0; j<SIZE; j++) 
			droppingPack[i][j] = pack[turn][i][j];
	}
	
	for(int i=0; i<rot; i++) rotatePack(droppingPack);
	if(debug) cout<<"begore"<<endl;
	bool isGameOver = dropPack(xpos , droppingPack);
	if(debug) cout<<isGameOver<<endl;
	
	for(int i=0; i<SIZE; i++) {
		delete[] droppingPack[i];
	}
	delete[] droppingPack;
	
	if(isGameOver) return true;
	eraseBlockAndUpdateScore();
	printForDebug();
	
	for(int j=0; j<WIDTH; j++) if(field[HEIGHT][j]!=EMPTY) return true;
	return false;
}



//--------------------state class end.------------------------------------------------------------------------


void output(int xpos , int rot , State &nowState) {
  
  nowState.outputData[nowState.turn]  = (make_pair(xpos , rot));
  //  printf("%d %d\n" , xpos , rot);
  //  fflush(stdout) ;
  isMain = true;
  nowState.updateNowState(xpos , rot);
  isMain = false;
}



bool isSafeState(int** &nowState) {
	
	int cnt = 0;
	
	for(int i=0; i<HEIGHT; i++) for(int j=0; j<WIDTH; j++) {
		if(nowState[i][j]!=EMPTY) cnt++;
	}
	double per = (double)cnt / (double)(HEIGHT*WIDTH);
	
	if(per<0.9) return true;
	else return false;
}

double hiddenScore(State nowState) {
  
  int dx[6] = {1,1,1,-1,-1,-1};
  int dy[6] = {1,0,-1,1,0,-1};
  
  double bestscore = 0;
  
  for(int i=0; i<HEIGHT; i++) for(int j=0; j<WIDTH; j++) if(nowState.field[i][j]==EMPTY) if(j>=7) {
	vector<int> cand;
	for(int d=0; d<6; d++) {
	  int sum = 0;
	  int nx = j;
	  int ny = i;
	  while(1) {
	    ny += dy[d];
	    nx += dx[d];
	    if(!valid(ny,nx)) break;
	    if(nowState.field[ny][nx]==EMPTY) break;
	    sum += nowState.field[ny][nx];
	    if(sum>=SUM) break;
	    cand.push_back(SUM-sum);
	  }
	}
	sort(cand.begin() , cand.end()) ;
	
	int height = 0;
	for(int k=i-1; k>=0; k--) if(nowState.field[k][j]==EMPTY) height++;
	double d = 1;
	for(int k=0; k<height; k++) d /= 1.4;
	
	int csize = (int)cand.size();
	for(int k=0; k<csize; k++) {
	  State tmpState = nowState;
	  tmpState.createStateField();
	  //			tmpState.field = createField();
	  for(int y=0; y<HEIGHT; y++) 
	    for(int x=0; x<WIDTH; x++) 
	      tmpState.field[y][x] = nowState.field[y][x];
	  
	  tmpState.field[i][j] = cand[k];
	  tmpState.addCoordinate(i,j);
	  
	  tmpState.eraseBlockAndUpdateScore();
	  
	  //d:到達難易度
	  double score = 1;
	  //	  score = (double) tmpState.score / (nowState.score+1);
	  score *= d;
	  int total = 0;
	  for(int y=0; y<HEIGHT; y++) 
	    for(int x=0; x<WIDTH; x++)
	      if(tmpState.field[y][x]=EMPTY)
		total++;
	  int cntsize  = tmpState.cntPrevEraseBlocks.size();
	  for(int x=0; x<cntsize; x++) score *= 3.0;
	  for(int x=0; x+3<cntsize; x++) {
	    int BORDER  = 3;
	    //	    if(tmpState.turn>400) BORDER = 3;
	    total -= tmpState.cntPrevEraseBlocks[x];
	    int t = tmpState.cntPrevEraseBlocks[x] - BORDER;
	    for(int z=0; z<t; z++) {
	      score /= 1.1;
	    }	  
	  }
       	  for(int z=0; z<total; z++) score /= (1.0015+0.0015*cntsize);
      	  for(int z=0; z<i; z++) score *= 1.00001;
	  if(score>bestscore) bestscore = score;
	  tmpState.deleteStateField();
	  //			deleteField(tmpState.field);
	}
      }
  double mult = 1000000;
  mult *= mult * mult;
  //  bestscore *= (double) (STEP - nowState.turn)  * mult ;
  bestscore *= mult;
  //  for(int i=0 ; i<nowState.turn; i++) bestscore /= 1.01;
  return bestscore;
}



void running() {
  
  State nowState;
  nowState.createStateField();
  nowState.highscore = make_pair(0,make_pair(0,make_pair(0,0))) ;  
  for(int i=0; i<STEP; i++) 
    nowState.outputData.push_back(make_pair(0,0));

  nowState.score = 0;
  for(nowState.turn = 0; nowState.turn<STEP; nowState.turn++){
    double bestscore = 0;
    int bestX = 0;
    int bestRot = 0;
    
    int xpos = -SIZE+1;
    //    if(nowState.turn==0) xpos = 6;
    for(; xpos<WIDTH; xpos++) {
      for(int rot = 0; rot<4; rot++) {
	//				ofs<<nowState.turn<<" "<<xpos<<" "<<rot<<endl;
	State tmpState = nowState;
	tmpState.createStateField();
	
	//				tmpState.field = createField();
	for(int i=0; i<HEIGHT+SIZE; i++) 
	  for(int j=0; j<WIDTH; j++) 
	    tmpState.field[i][j] = nowState.field[i][j];
	bool isGameover = tmpState.updateNowState(xpos , rot );
	if(!isGameover) {
	  if(tmpState.score>nowState.highscore.first) {
	    nowState.highscore = make_pair(tmpState.score , make_pair(nowState.turn , make_pair(xpos , rot)));
	  }
	  double score = hiddenScore(tmpState) + tmpState.score;
	  //					ofs<<nowState.turn<<" "<<xpos<<" "<<rot<<"  : "<<score-tmpState.score<<" "<<tmpState.score<<endl;
	  if(score>bestscore) {
	    bestX = xpos;
	    bestRot = rot; 
	    bestscore = score;
	  }
	}
	if(debug) cout<<"ge"<<endl;
	tmpState.deleteStateField();
	if(debug) cout<<"kami"<<endl;
      }
    } 
    output(bestX , bestRot , nowState);
    if(nowState.turn==STEP-1) {
      ofs<<nowState.highscore.first <<" "<<nowState.score<<endl;
      if(nowState.highscore.first>nowState.score) {
	int turns = nowState.highscore.second.first;
	nowState.deleteStateField();
	nowState.createStateField();
	nowState.score = 0;
	for(nowState.turn=0; nowState.turn<turns; nowState.turn++) 
	  output(nowState.outputData[nowState.turn].first , nowState.outputData[nowState.turn].second , nowState);
	output(nowState.highscore.second.second.first , nowState.highscore.second.second.second , nowState);
	ofs<<nowState.score<<endl;
      }
    }
  }
  for(int i=0; i<STEP; i++) {
    printf("%d %d\n" , nowState.outputData[i].first ,nowState.outputData[i].second);
    fflush(stdout);
  }

}


int main(){
	
	
	srand((unsigned)time(NULL));
//	int wid , hei , size , sum , step;
	int sum;
	scanf("%d %d %d %d %d" , &WIDTH , &HEIGHT , &SIZE , &sum , &STEP);
	HS = HEIGHT + SIZE;
	HWS = HS + WIDTH -1;

	NUMP = WIDTH+15;
	
	allLength[0] = WIDTH;
	allLength[1] = HS;
	allLength[2] = HWS;
	allLength[3] = HWS;
	
	SUM = (int)sum;
	
	ofs<<"START!!"<<endl;
	ofs<<WIDTH<<" "<<HEIGHT<<endl;

	
	
	char endstr[1024];
	pack = new int**[STEP];
	for(int i=0 ; i<STEP ; i++) {
		pack[i]=new int*[SIZE];
		for(int j=0; j<SIZE; j++) {
			pack[i][j] = new int[SIZE];
			for(int k=0; k<SIZE; k++) {
				scanf( "%d" , &pack[i][j][k]);
			}
		}
		scanf("%s",endstr);
	}
	
	running();
	
	
	for(int i=0; i<STEP; i++){
		for(int j=0; j<SIZE; j++){
			delete[] pack[i][j];
		}
		delete[] pack[i];
	}
	delete[] pack;
	return 0;
}
