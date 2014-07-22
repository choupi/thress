#include "Threes.h"
#include <ctime>
#ifdef _WIN32
#include <conio.h>
#elif defined(__linux__)
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int getch(void){
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
#endif

int RUNRND=250;
int RUNCNT=300;
int RUNDEP=6;

int _log2(int x);

const char* dirToStr(dir_e dir){
    if(dir == LEFT)   return "LEFT   ";
    if(dir == DOWN)   return "DOWN   ";
    if(dir == RIGHT)  return "RIGHT  ";
    if(dir == UP)     return "UP     ";
    return "INVALID";
}

// Exmaple1. Original game played with AWSD keys
dir_e getDirFromKeyboard(){
    int dir;
    dir = getch();
    
    if(dir == 'A' || dir == 'a')  return LEFT;
    if(dir == 'S' || dir == 's')  return DOWN;
    if(dir == 'D' || dir == 'd')  return RIGHT;
    if(dir == 'W' || dir == 'w')  return UP;
    return INVALID;
}

// Exmaple2. Random direction inserting bot
dir_e getRandDir(){
    int dir = rand()%4;
    
    if(dir == 0)  return LEFT;
    if(dir == 1)  return DOWN;
    if(dir == 2)  return RIGHT;
    if(dir == 3)  return UP;
    return INVALID;
}

dir_e getDirFromInt(int dir){
    if(dir == 0)  return LEFT;
    if(dir == 1)  return DOWN;
    if(dir == 2)  return RIGHT;
    if(dir == 3)  return UP;
    return INVALID;
}


void setNT(Grid &myGrid, int nt)
{
    int nSlot = myGrid.getSlotNo();
	int randSlot = rand() % nSlot;
	bool success = myGrid.setSlot(randSlot, nt);
}

int genSNT(Grid &myGrid)
{
	int maxTile = myGrid.getMaxTile();
	int n = rand() % (_log2(maxTile / BONUS_THRESHOLD) + 1);
	return (BONUS_BASE << n);
}

int getNT(Grid &myGrid)
{
    int maxTile = myGrid.getMaxTile();
	int m_nextTile;
    if(maxTile >= BONUS_THRESHOLD && rand() % BONUS_RATIO == BONUS_RATIO - 1){
        //int n = rand() % (_log2(maxTile / BONUS_THRESHOLD) + 1);
        //m_nextTile = (BONUS_BASE << n);
		m_nextTile = genSNT(myGrid);
    }
    else{
        int tileType;
        //int nTile = m_grabBag[0] + m_grabBag[1] + m_grabBag[2];
        int nTile = 3;
        int randTile = rand() % nTile;
		tileType=randTile+1;
/*
        if(randTile < m_grabBag[0]){
            m_grabBag[0]--;
            tileType = 1;
        }
        else if(randTile < m_grabBag[0] + m_grabBag[1]){
            m_grabBag[1]--;
            tileType = 2;
        }
        else{
            m_grabBag[2]--;
            tileType = 3;
		}
        if(nTile == 1)
            resetGrabBag();
*/
        m_nextTile = tileType;
    }
	return m_nextTile;
}

dir_e computeDir(Grid &myGrid, char NT, float& score)
{
	int m=0,ms=myGrid.getScore(),nt;
	float dm[4]={0};
	int fcnt=0;

	for(int d=0;d<4;d++) {
	for(int i=0;i<RUNCNT;i++) {
		Grid tg=myGrid;
		if(!tg.shift(getDirFromInt(d))) { dm[d]=RUNCNT; break; }
		//printf("%d\n", tg.getSlotNo());fflush(stdout);
		if(NT=='+') setNT(tg, genSNT(tg));
		else { setNT(tg, NT-'0'); }
		for(int j=0;j<RUNDEP;j++) {
			if(!tg.shift(getRandDir())) { fcnt+=1; continue; }
			setNT(tg, getNT(tg));
		}
		int s=tg.getScore();
		dm[d]+=s;
	}
	}
	int di=0;
	float dmm=dm[0];
	for(int i=1;i<4;i++) {
		if(dm[i]>dmm) { dmm=dm[i]; di=i;}
	}
	score= fcnt/4.0/(float)RUNCNT;
	return getDirFromInt(di);
}

void PlayNRounds(int n){
    int score;
    Game myGame;
    bool isGameOver;
    dir_e dir;
	float avgS;

    if(myGame.isGameOver(score))  myGame.reset();

    Grid myGrid;
    for(int i = 0;i < n;i++){    
        isGameOver = false;
            //while((dir = getDirFromKeyboard()) == INVALID);
			do {
				myGame.reset();
				for(int j=0;j<RUNRND;j++) {
					myGame.getCurrentGrid(myGrid);
					myGame.insertDirection(computeDir(myGrid, getNT(myGrid), avgS));
				}
            } while(myGame.isGameOver(score));
			myGame.getCurrentGrid(myGrid);
			computeDir(myGrid, getNT(myGrid), avgS);
            //isGameOver = myGame.isGameOver(score);
    		//gotoXY(0,20);
			for(int j=0;j<16;j++) printf("%d ", myGrid[j]);
			printf("%.2f\n", avgS);
            
        if(i < n - 1)  myGame.reset();
 
    }
}

int main(int argc, char* argv[]){
    // Note: API function calls performed by any 'Game' object effects the same set of static class members,
    // so even through the 2 following function calls use different 'Game' objects, the same game continues
	if(argc>1) { 
		RUNCNT=atoi(argv[1]); 
		RUNDEP=atoi(argv[2]); 
		RUNRND=atoi(argv[3]); 
	}
	srand(time(NULL));
    //PlayNRounds(100000);
    PlayNRounds(25000);
    return 0;
}

