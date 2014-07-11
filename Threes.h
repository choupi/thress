#ifndef GAME_H_
#define GAME_H_
#define NDEBUG       // turn off assert() for debugging
#define EN_PRINT     // turn on printing functions

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/time.h>
#endif

#include <fstream>
#include <iostream>
#include <assert.h>
#include <cstdlib>
#include <cstring>

#define TRUE                true
#define FALSE               false

#define GRID_LENGTH         4
#define GRID_SIZE           16
#define STAGE_NUM           6
#define FIRST_STAGE         192

#define INITIAL_TILE_NUM    9
#define EMPTY               0
#define WHITE_BASE          3
#define BONUS_BASE          6
#define BONUS_THRESHOLD     48

#define BAG_SIZE            12
#define BONUS_RATIO         21
#define BASIC_TYPE_NUM      3
#define WHITE_TYPE_NUM      12

#define DIR_NUM             4

#define ERROR_KEY           -1

typedef enum{
    LEFT = 0,
    DOWN,
    RIGHT,
    UP,
    INVALID
} dir_e;

void gotoXY(int xPos, int yPos);
double cpuTime();

class Grid{
    private:
        int   m_data[GRID_SIZE];
        int   m_maxTile;
        int   m_nEmptyBlk;
        int   m_nSlot;
        int*  m_slot[GRID_LENGTH];

        // inline
        int&  getEntry(int row, int col);
        int&  getFlipEntry(int row, int col);
        int&  getTransEntry(int row, int col);
        int&  getFlipTransEntry(int row, int col);
    public:
        // inline
        Grid();
        Grid(Grid& grid);
        Grid& operator=(Grid& grid);
        int   operator[](int index);
        int   operator()(int row, int col);
        void  clear();
        void  setBlock(int index, int val);
        bool  setSlot(int num, int val);
        int   getEmptyBlkNo();
        int   getSlotNo();
        int   getSlotIndex(int num);
        int   getMaxTile();
        bool  canMerge(int a, int b);
        
        void  copy(Grid& grid);
        int   getScore();
        bool  shift(dir_e dir);
        void  print(int xPos = 0, int yPos = 0);
};

class Game{
    private:
        static int   m_objCount;
        static bool  m_objInit;

        static Grid  m_grid;
        static bool  m_gameOver;
        static int   m_nRound;
        static int   m_score;
        static int   m_maxScore;
        static int   m_scoreSum;
        static int   m_maxTile;
        static int   m_passCnt[STAGE_NUM];
        static int   m_moveCnt;
        static int   m_grabBag[BASIC_TYPE_NUM];
        static int   m_nextTile;
        static double   m_startTime;
        static double   m_endTime;

        // inline
        void  init();
        int   getRand();
        int   getMaxScore();         
        void  resetGrabBag();
        
        void  genInitTile();
        void  genNewTile();
        void  setNextTile();
        int   getNextTile();
        void  updateStats();
        void  setGameOver();
        void  dumpLog(const char* fileName);
    public:
        Game();
        ~Game();
        void  reset();

        // inline
        char  getHint();
        void  getCurrentGrid(Grid& currGrid);
        bool  insertDirection(dir_e dir);
        bool  isGameOver(int& score);
        void  printGrid(int xPos, int yPos);
};

/*************************************************
                   Grid inline
*************************************************/
// Grid()
// Description: initialize members
inline
Grid::Grid(){
    clear();
}

// Grid(Grid& grid)
// Description: creates a copy of the given grid
// Arguments:
//     grid  -  given grid
inline
Grid::Grid(Grid& grid){
    copy(grid);
}

// operator=
// Description: copies contest of given grid into this grid
// Arguments:
//     grid  -  given grid
// Return Val: reference to this grid
inline
Grid& Grid::operator=(Grid& grid){
    copy(grid);
    return *this;
}

// operator[]
// Description: returns entry value specified by index
//              returns error key when given index is out of range
// Arguments:
//     index  -  index of entry
// Return Val: entry value specified by index, or ERROR_KEY if out of range
inline
int Grid::operator[](int index){
    if(index < 0 || index > GRID_SIZE - 1){
        assert(FALSE);
        return ERROR_KEY;
    }
    return m_data[index];
}

// operator()
// Description: returns entry value specified by (row, col) coordinates
//              returns error key when given coordinates are out of range
// Arguments:
//     row  -  row number of entry
//     col  -  column number of entry
// Return Val: entry value specified by (row, col) coordinates, or ERROR_KEY if out of range
inline
int Grid::operator()(int row, int col){
    int index = row * GRID_LENGTH + col;
    if(row < 0 || row > GRID_LENGTH - 1 || col < 0 || col > GRID_LENGTH - 1){
        assert(FALSE);
        return ERROR_KEY;
    }
    return m_data[index];
}

// clear()
// Description: sets all grid entries to EMPTY
inline
void Grid::clear(){
    memset(m_data, EMPTY, sizeof(m_data));
    m_nEmptyBlk = GRID_SIZE;
    m_maxTile = EMPTY;
    m_nSlot = 0;
}

// setBlock()
// Description: sets value of specified block
// Arguments:
//     index  -  index of block
//     val    -  specified value that will be assigned to block
inline
void Grid::setBlock(int index, int val){
    if(index < 0 || index >= GRID_SIZE){
        assert(FALSE);
        return;
    }
    if(m_data[index] == EMPTY && val != EMPTY)
        m_nEmptyBlk--;
    else if(m_data[index] != EMPTY && val == EMPTY)
        m_nEmptyBlk++;
    m_data[index] = val;
    if(val > m_maxTile)
        m_maxTile = val;
}

// setSlot()
// Description: sets value of specified slot
// Arguments:
//     num    -  number of slot
//     val    -  specified value that will be assigned to slot
// Return Val: TRUE if slot is successfully set, 
//             FALSE if given slot number is invalid
inline
bool Grid::setSlot(int num, int val){
    assert(num < m_nSlot);
    if(num < m_nSlot){
        assert(*(m_slot[num]) == EMPTY);
        *(m_slot[num]) = val;
        m_nEmptyBlk--;
        m_nSlot = 0;
        return TRUE;
    }
    return FALSE;
}

// canMerge()
// Description: returns TRUE if given tiles can be merged, FALSE if not
// Arguments:
//     a   -   first tile
//     b   -   second tile
// Return Val: TRUE if given tiles can be merged, FALSE if not
inline
bool Grid::canMerge(int a, int b){
    if(a == 6144 || b == 6144)  return FALSE;
    return (a > 2 && a == b) || (a == 1 && b == 2) || (a == 2 && b == 1);
}

// getEmptyBlkNo()
// Description: return number of empty blocks in grid
// Return Val: number of empty blocks in given grid
inline
int Grid::getEmptyBlkNo(){
    return m_nEmptyBlk;
}

// getSlotNo()
// Description: return number of slots that can take a new tile after shift
// Return Val: number of slots
inline
int Grid::getSlotNo(){
    return m_nSlot;
}

// getSlotIndex()
// Description: return index of slot specified by number
// Arguments:
// Return Val:
inline 
int Grid::getSlotIndex(int num){
    if(num < 0 || num >= m_nSlot){
      assert(FALSE);
      return ERROR_KEY;
    }
    assert(m_slot[num] - m_data > -1 && m_slot[num] - m_data < GRID_SIZE);
    return (m_slot[num] - m_data);
}

// getMaxTile()
// Description: return largest tile number in grid
// Return Val: largest tile number in grid
inline
int Grid::getMaxTile(){
    return m_maxTile;
}

// getEntry()
// Description: get entry of original grid
// Arguments:
//     row  -  row number of entry
//     col  -  col number of entry
// Return Val: entry of original grid
inline
int& Grid::getEntry(int row, int col){
    return m_data[row*GRID_LENGTH + col];
}

// getFlipEntry()
// Description: get entry of horizontally flipped grid
// Arguments:
//     row  -  row number of entry
//     col  -  col number of entry
// Return Val: entry of horizontally flipped grid
inline
int& Grid::getFlipEntry(int row, int col){
    return m_data[row*GRID_LENGTH + (GRID_LENGTH - 1 - col)];
}

// getTransEntry()
// Description: get entry of transposed grid
// Arguments:
//     row  -  row number of entry
//     col  -  col number of entry
// Return Val: entry of transposed grid
inline
int& Grid::getTransEntry(int row, int col){
    return m_data[col*GRID_LENGTH + row];
}

// getFlipTransEntry()
// Description: get entry of horizontally flipped, transposed grid
// Arguments:
//     row  -  row number of entry
//     col  -  col number of entry
// Return Val: entry of horizontally flipped, transposed grid
inline
int& Grid::getFlipTransEntry(int row, int col){
    return m_data[(GRID_LENGTH - 1 - col)*GRID_LENGTH + row];
}

/*************************************************
                   Game inline
*************************************************/
// getRand()
// Description: return pseudo random integer value between "0 to RAND_MAX"
// Return Val: pseudo random integer value between "0 to RAND_MAX"
inline
int Game::getRand(){
    return rand();
}

// getNextTile()
// Description: get the next pre-calculated tile
// Return Val: next pre-calculated tile
inline
int Game::getNextTile(){
    return m_nextTile;
}

// getHint()
// Description: get the hint of the next pre-calculated tile
// Return Val: hint of the next pre-calculated tile
inline
char Game::getHint(){
    if(m_nextTile < BONUS_BASE)  return m_nextTile + '0';
    return '+';
}

// isGameOver()
// Description: check if game is over and get score if it is
// Arguments:
//     score  -  final score of current game
// Return Val: returns TRUE if game is over, FALSE if not
inline
bool Game::isGameOver(int& score){
    if(m_gameOver){
        score = m_score;
        return TRUE;
    }
    else{
        score = 0;
        return FALSE;
    }
}

// getMaxScore()
// Description: gets the highest score ever achieved
// Return Val: highest score
inline
int Game::getMaxScore(){
    return m_maxScore;
}

// resetGrabBag()
// Description: load grab bag with basic tiles equally
inline
void Game::resetGrabBag(){
    m_grabBag[0] = m_grabBag[1] = m_grabBag[2] = BAG_SIZE / BASIC_TYPE_NUM;
}

// insertDirection()
// Description: shift tiles in given direction and generate new tile
//              if cannot be shifted, will not generate new tile
// Arguments:
//     dir  -  shift direction
//
// Return Val: returns TRUE if shift was successful and tile was generated, FALSE if not
inline
bool Game::insertDirection(dir_e dir){
    if(!m_grid.shift(dir))  return FALSE;

    genNewTile();
    m_moveCnt++;
    setGameOver();
    return TRUE;
}

// getCurrentGrid()
// Description: copy contents of game grid into given grid
// Arguments:
//     currGrid  -  game grid will be copied into this grid
inline
void Game::getCurrentGrid(Grid& currGrid){
    currGrid.copy(m_grid);
}

// printGrid()
// Description: prints the game grid at the given (x,y) coordinates
// Arguements:
//      xPos  -  x coordinate
//      yPos  -  y coordinate 
inline
void Game::printGrid(int xPos, int yPos){
    m_grid.print(xPos, yPos);
}

#endif

