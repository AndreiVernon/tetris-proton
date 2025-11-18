#ifndef __TETRIS_H__
#define __TETRIS_H__

//playfield dims
#define M_WIDTH 10
#define M_HEIGHT 23  //+1 for 5 large I piece

#define I_PIECE 0
#define O_PIECE 1
#define S_PIECE 2
#define Z_PIECE 3
#define T_PIECE 4
#define L_PIECE 5
#define J_PIECE 6
#define GARBAGE 7
#define EMPTY 67
#define GHOST 61

typedef struct _Piece {
    int shape;          //shape id, 0-6
    int x, y;           //coords, x=0 is left and y=0 is bottom
    int rotation;       //0, 1=R, 2, 3=L
    uint8_t mask[25];   //shape mask
    int size;           //mask size
} Piece;

uint8_t matrix[M_HEIGHT][M_WIDTH];
uint32_t score = 0;
bool game_over = false;

Piece piece;        //currently active piece
Piece ghost_piece;        //ghost piece / shadow of active piece
int held_piece = -1;        //shape of held piece
bool hold_avail = true;     //can hold piece
int rand_bag[14];           //bag of upcoming pieces
int rand_bag_loc;           //index of bag

int game_loop();

#endif