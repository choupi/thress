#include "Threes.h"
/*************************************************
           Static Data Members of Game
*************************************************/
// Note: please be aware of the "static initialization order fiasco" 
// when attempting to use global 'Game' objects. 
// We recommend using global Game pointers instead and newing the object
// inside main()
bool  Game::m_objInit = FALSE;
Grid  Game::m_grid;
bool  Game::m_gameOver;
int   Game::m_nRound;
int   Game::m_score;
int   Game::m_maxScore;
int   Game::m_scoreSum;
int   Game::m_maxTile;
int   Game::m_passCnt[STAGE_NUM];
int   Game::m_moveCnt;
int   Game::m_grabBag[BASIC_TYPE_NUM];
int   Game::m_nextTile;
double   Game::m_startTime;
double   Game::m_endTime;

/*************************************************
                      Math
*************************************************/
// _pow3[i] = 3^(i+1) for i = 0 ~ (WHITE_TYPE_NUM - 1)
int _pow3[WHITE_TYPE_NUM] = 
{ 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441 };

// _log2()
// Description: base 2 log function for positive integers
// Arguments:
//     x  -  positive integer
// Return Val: logarithm of x with respect to base 2
int _log2(int x){
    assert(x > 0);
    int count = 0;
    while(x >>= 1)  count++;
    return count;
}

/*************************************************
                   Grid public
*************************************************/
// copy()
// Description: copies contents of given grid into this grid
// Arguments:
//     grid  -  given grid
void Grid::copy(Grid& grid){
    if(this == &grid)  return;
    memcpy(m_data, grid.m_data, sizeof(m_data));
    m_nEmptyBlk = grid.m_nEmptyBlk;
    m_maxTile = grid.m_maxTile;
    m_nSlot = grid.m_nSlot;
    for(int i = 0;i < m_nSlot;i++)
        m_slot[i] = grid.getSlotIndex(i) + m_data;
}

// getScore()
// Description: calculate and return score (only white tiles count)
// Return Val: current score
int Grid::getScore(){
    int score = 0;
    for(int i = 0;i < GRID_SIZE;i++){
        if(m_data[i] < WHITE_BASE)  continue;
        assert(_log2(m_data[i]/WHITE_BASE) > -1);
        assert(_log2(m_data[i]/WHITE_BASE) < WHITE_TYPE_NUM);
        score += _pow3[ _log2(m_data[i]/WHITE_BASE) ];
    }
    return score;
}

// shift()
// Description: shift tiles in given direction
// Arguments:
//     dir   -   shift direction
// Return Val: TRUE if shift is successful, FALSE if not
bool Grid::shift(dir_e dir){
    m_nSlot = 0;
    bool isShifted;
    int& (Grid::* getDirEntry)(int, int) = NULL;
        
    if(dir == LEFT)          getDirEntry = &Grid::getEntry;
    else if(dir == DOWN)     getDirEntry = &Grid::getFlipTransEntry;
    else if(dir == RIGHT)    getDirEntry = &Grid::getFlipEntry;
    else if(dir == UP)       getDirEntry = &Grid::getTransEntry;

    assert(getDirEntry != NULL);

    for(int i = 0;i < GRID_LENGTH;i++){        
        isShifted = FALSE;
        for(int j = 1;j < GRID_LENGTH;j++){
            if((this->*getDirEntry)(i, j) == EMPTY)
                continue;

            if( (this->*getDirEntry)(i, j-1) == EMPTY ){
                (this->*getDirEntry)(i, j-1) = (this->*getDirEntry)(i, j);
                (this->*getDirEntry)(i, j) = EMPTY;
                isShifted = TRUE;
            }                
            else if(canMerge((this->*getDirEntry)(i, j),(this->*getDirEntry)(i, j-1))){
                (this->*getDirEntry)(i, j-1) += (this->*getDirEntry)(i, j);
                if( (this->*getDirEntry)(i, j-1) > m_maxTile )
                    m_maxTile = (this->*getDirEntry)(i, j-1);
                (this->*getDirEntry)(i, j) = EMPTY;
                isShifted = TRUE;
                m_nEmptyBlk++;
            }
        }
        if(isShifted == TRUE){
            m_slot[m_nSlot] = &(this->*getDirEntry)(i, GRID_LENGTH-1);
            m_nSlot++;
        }
    }
    return (m_nSlot > 0);
}

// print()
// Description: prints grid at given origin (xPos, yPos)
// Arguments:
//     xPos  -  x coordinate of origin
//     yPos  -  y coordinate of origin
void Grid::print(int xPos, int yPos){
#ifdef EN_PRINT
    for(int i = 0;i < GRID_LENGTH;i++){
        for(int j = 0;j < GRID_LENGTH;j++){
            int x = xPos + 6 * j;
            int y = yPos + 2 * i;
            gotoXY(x,y);
            std::cout<<"      ";
            gotoXY(x,y);
            std::cout<<m_data[i*GRID_LENGTH + j];
        }
    }
#endif
}

/*************************************************
                   Game public
*************************************************/

// Game()
// Description: initialize game record & reset game
Game::Game(){
    if(!m_objInit)
        init();
    m_objInit = TRUE;
}

// ~Game()
// Description: game ends, calculate & dump results to log
Game::~Game(){
    if(m_nRound == 100){
        m_endTime = cpuTime();
        dumpLog("open_src_version.log");
    }
}

// reset()
// Description: prepare variables for new game
void Game::reset(){
    updateStats();
    m_grid.clear();
    m_gameOver = FALSE;
    resetGrabBag();
    setNextTile();
        
    for(int i = 0;i < INITIAL_TILE_NUM;i++)
        genInitTile();
}

/*************************************************
                   Game private
*************************************************/
// init()
// Description: initialize game records
void Game::init(){
    m_nRound = 0;
    m_moveCnt = 0;
    m_score = 0;
    m_maxScore = 0;
    m_scoreSum = 0;
    m_maxTile = 0;
    memset(m_passCnt, 0, sizeof(m_passCnt));
    m_gameOver = FALSE;
    resetGrabBag();
    setNextTile();

    for(int i = 0;i < INITIAL_TILE_NUM;i++)
        genInitTile();

    m_startTime = cpuTime();
}

// genInitTile()
// Description: generate initial tile in random empty block
void Game::genInitTile(){
    int randBlk = getRand() % m_grid.getEmptyBlkNo() + 1;
    
    for(int i = 0;i < GRID_SIZE;i++){             
        if(m_grid[i] == EMPTY)
            randBlk--;
        if(randBlk == 0){
            m_grid.setBlock(i, getNextTile());
            setNextTile();
            return;
        }
    }
    assert(FALSE);
}    

// genNewTile()
// Description: generate new tile in random slot
void Game::genNewTile(){
    int nSlot = m_grid.getSlotNo();
    int randSlot = getRand() % nSlot;
    bool success = m_grid.setSlot(randSlot, getNextTile());
    assert(success);
    setNextTile();
}

// setNextTile()
// Description: determine the next random bonus tile or grab a basic tile from grab bag
void Game::setNextTile(){
    int maxTile = m_grid.getMaxTile();
    if(maxTile >= BONUS_THRESHOLD && getRand() % BONUS_RATIO == BONUS_RATIO - 1){
        int n = getRand() % (_log2(maxTile / BONUS_THRESHOLD) + 1);
        m_nextTile = (BONUS_BASE << n);
    }
    else{
        int tileType;
        int nTile = m_grabBag[0] + m_grabBag[1] + m_grabBag[2];
        assert(nTile > 0);
        int randTile = getRand() % nTile;
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
        m_nextTile = tileType;
    }
}

// updateStats()
// Description: update game statistics at end of round
void Game::updateStats(){
    m_nRound++;
    m_scoreSum += m_score;
    if(m_score > m_maxScore)
        m_maxScore = m_score;
    int maxTile = m_grid.getMaxTile();
    if(maxTile > m_maxTile)
        m_maxTile = maxTile;
 
    for(int i = 0;i < STAGE_NUM;i++){
        if(maxTile >= (FIRST_STAGE << i))
            m_passCnt[i]++;
    }
}

// setGameOver()
// Description: determine whether game is over and calculate score if it is
void Game::setGameOver(){
    if(m_grid.getEmptyBlkNo() > 0){
        m_gameOver = FALSE;
        return;
    }
    
    for(int i = 0;i < GRID_LENGTH;i++){
        for(int j = 0;j < GRID_LENGTH;j++){
            if( (i < GRID_LENGTH - 1) && m_grid.canMerge(m_grid(i,j),m_grid(i+1,j)) ){
                m_gameOver = FALSE;
                return;
            }
            if( (j < GRID_LENGTH - 1) && m_grid.canMerge(m_grid(i,j),m_grid(i,j+1)) ){
                m_gameOver = FALSE;
                return;
            }
        }
    }
    m_gameOver = TRUE;
    m_score = m_grid.getScore();
}

// dumpLog()
// Description: dump results into log
// Arguements:
//      fileName  -  log file name, will overwrite if already exists
void Game::dumpLog(const char* fileName){
    std::ofstream logFile(fileName, std::ios::out);
    logFile<<"#Rounds: "<<m_nRound<<"\n";
    logFile<<"Highest Score: "<<m_maxScore<<"\n";
    logFile<<"Average Score: "<< (double)m_scoreSum / m_nRound<<"\n";
    logFile<<"Max Tile: "<<m_maxTile<<'\n';
    for(int i = 0;i < STAGE_NUM;i++){
        logFile<< (FIRST_STAGE << i) <<" Rate: "<<(double)m_passCnt[i]*100/m_nRound<<"%\n";
    }
    logFile<<"Move Count: "<<m_moveCnt<<'\n';
    logFile<<"Time: "<<m_endTime - m_startTime<<'\n';
    logFile.close();
}

/*************************************************
                     Printing
*************************************************/
// gotoXY()
// Description: move cursor to (xPos, yPos)
// Arguments:
//     xPos  -  x coordinate
//     yPos  -  y coordinate
void gotoXY(int xPos, int yPos){
#ifdef EN_PRINT
  #ifdef _WIN32
  COORD scrn;
  HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
  scrn.X = xPos; scrn.Y = yPos;
  SetConsoleCursorPosition(hOuput,scrn);
  #elif defined(__linux__)
  printf("\033[%d;%df", yPos, xPos);
  fflush(stdout);
  #endif
#endif
}

/*************************************************
                  Time Measuring
*************************************************/
double cpuTime(){
#ifdef _WIN32
    FILETIME a,b,c,d;
    if (GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d) != 0)
        return (double)(d.dwLowDateTime | ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
    else
        return 0;

#elif defined(__linux__)
    return (double)clock() / CLOCKS_PER_SEC;
#endif
}


