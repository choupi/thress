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

int RUNCNT=1000;
int RUNDEP=10;
float DTHRS=1.65;
float COEF[4]={1,1.65,0,-0.0005};

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

void resetGrabBag(int* m_grabBag)
{
    m_grabBag[0] = m_grabBag[1] = m_grabBag[2] = BAG_SIZE / BASIC_TYPE_NUM;
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

int getNT(Grid &myGrid, int* m_grabBag)
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
        int nTile = m_grabBag[0] + m_grabBag[1] + m_grabBag[2];
        //int nTile = 3;
        int randTile = rand() % nTile;
		tileType=randTile+1;

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
            resetGrabBag(m_grabBag);

        m_nextTile = tileType;
    }
	return m_nextTile;
}

float mcScore(float* dm)
{
	return dm[0]*COEF[0]+dm[1]*COEF[1]+dm[2]*COEF[2]+dm[0]*dm[3]*COEF[3];
}

dir_e computeDir(Grid &myGrid, char NT, int* m_grabBag)
{
	int m=0,ms=myGrid.getScore(),nt;
	float dm[4][4]={0};
	dir_e bd=LEFT;

	for(int d=0;d<4;d++) {
	  dm[d][2]=0xFFFFFFFF;
	  for(int i=0;i<RUNCNT;i++) {
		Grid tg=myGrid;
		int shift_f=0;
		if(!tg.shift(getDirFromInt(d))) {
			dm[d][2]=0;
			dm[d][3]=RUNCNT;
			break;
		}
		//printf("%d\n", tg.getSlotNo());fflush(stdout);
		if(NT=='+') setNT(tg, genSNT(tg));
		else { setNT(tg, NT-'0'); }
		int tmp_grabBag[3]={m_grabBag[0], m_grabBag[1], m_grabBag[2]};
		for(int j=0;j<RUNDEP;j++) {
			if(!tg.shift(getRandDir())) { shift_f=1; continue; }
			setNT(tg, getNT(tg, tmp_grabBag));
		}
		int s=tg.getScore();
		if(s>dm[d][1]) { dm[d][1]=s; }
		if(s<dm[d][2]) { dm[d][2]=s; }
		dm[d][0]+=s;
		dm[d][3]+=shift_f;
	  }
	}
	int ds=rand()%4,di=ds;
	float dms=mcScore(dm[ds]), dmm=dm[di][1];
	gotoXY(0,20);
	for(int i=0;i<4;i++) {
		dmm=mcScore(dm[i]);
		if(dmm>dms) { dms=dmm; ds=i; }
		//printf("%.2f %.0f %.0f %.2f | ", dm[i][0]/RUNCNT,dm[i][1],dm[i][3],dmm);
	}
	//printf("\n%s\n", dirToStr(getDirFromInt(ds)));
	//if(dms/RUNCNT*DTHRS>dmm) return getDirFromInt(ds);
	return getDirFromInt(ds);
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

    Grid myGrid;
    for(int i = 0;i < n;i++){    
        isGameOver = false;
		int m_grabBag[3];
		resetGrabBag(m_grabBag);
		myGame.getCurrentGrid(myGrid);
		for(int j=0;j<16;j++) if(myGrid[j]>0) m_grabBag[myGrid[j]-1]-=1;
        while(!isGameOver){
			myGame.getCurrentGrid(myGrid);
			char h=myGame.getHint();
			if(h!='+') m_grabBag[h-'1']-=1;
			if(m_grabBag[0]+m_grabBag[1]+m_grabBag[2]<1) resetGrabBag(m_grabBag);
            gotoXY(5,22);
			printf("%d %d %d\n", m_grabBag[0],m_grabBag[1],m_grabBag[2]);
			dir=computeDir(myGrid, h, m_grabBag);
            //while((getDirFromKeyboard()) == INVALID);
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
        myGame.reset();
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
		//DTHRS=atof(argv[3]); 
		COEF[0]=atof(argv[3]);
		COEF[1]=atof(argv[4]);
		COEF[2]=atof(argv[5]);
		COEF[3]=atof(argv[6]);
	}
	srand(time(NULL));
    PlayNRounds(100);
    //PlayNRounds(50);
    return 0;
}

