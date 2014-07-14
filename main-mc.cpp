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

int RUNCNT=150;
int RUNDEP=3;
float DTHRS=1.65;

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

dir_e computeDir(Grid &myGrid, char NT)
{
	int m=0,ms=myGrid.getScore(),nt;
	float dm[4]={0};
	dir_e bd=LEFT;

	for(int d=0;d<4;d++) {
	for(int i=0;i<RUNCNT;i++) {
		Grid tg=myGrid;
		if(!tg.shift(getDirFromInt(d))) break;
		//printf("%d\n", tg.getSlotNo());fflush(stdout);
		if(NT=='+') setNT(tg, genSNT(tg));
		else { setNT(tg, NT-'0'); }
		for(int j=0;j<RUNDEP;j++) {
			if(!tg.shift(getRandDir())) continue;
			setNT(tg, getNT(tg));
		}
		int s=tg.getScore();
		if(s>m) { m=s; bd=getDirFromInt(d); }
		dm[d]+=s;
	}
	}
	int d=rand()%4;
	float dmm=dm[d];
	gotoXY(5,20);
	for(int i=0;i<4;i++) {
		if(dm[i]>dmm) { dmm=dm[i]; d=i; }
		//printf("%.0f ", dm[i]);
	}
	//printf("\n%.0f %d\n", dmm, m);
	//printf("%d %d\n", d, bd);
	if(dmm/RUNCNT*DTHRS>m) return getDirFromInt(d);
	return bd;
}

void PlayNRounds(int n){
#ifdef _WIN32
    system("cls");
#elif defined(__linux__)
    system("clear");
#endif
    int score;
    Game myGame;
    bool isGameOver;
    dir_e dir;

    gotoXY(5,0);
    std::cout<<"Previous";
    gotoXY(35,0);
    std::cout<<"Current (Hint: "<<myGame.getHint()<<")";
    myGame.printGrid(35,2);

    if(myGame.isGameOver(score))  myGame.reset();

    Grid myGrid;
    for(int i = 0;i < n;i++){    
        isGameOver = false;
        while(!isGameOver){
            //while((dir = getDirFromKeyboard()) == INVALID);
			myGame.getCurrentGrid(myGrid);
			dir=computeDir(myGrid, myGame.getHint());
            gotoXY(5,10);
            std::cout<<dirToStr(dir);
            myGame.printGrid(5,2);
          
            myGame.insertDirection(dir);
            gotoXY(50,0);
            std::cout<<myGame.getHint();
            isGameOver = myGame.isGameOver(score);
            myGame.printGrid(35,2);
            
       }
        myGame.printGrid(35,2);
        if(i < n - 1)  myGame.reset();
        gotoXY(0,15); 
        printf("  Round:    %d      \n", i+1);
        printf("  Score:    %d      \n", score);
 
    }
}

int main(int argc, char* argv[]){
    // Note: API function calls performed by any 'Game' object effects the same set of static class members,
    // so even through the 2 following function calls use different 'Game' objects, the same game continues
	if(argc>1) { 
		RUNCNT=atoi(argv[1]); 
		RUNDEP=atoi(argv[2]); 
		DTHRS=atof(argv[3]); 
	}
	srand(time(NULL));
    PlayNRounds(50);
    PlayNRounds(50);
    return 0;
}

