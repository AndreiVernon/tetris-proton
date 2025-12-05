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
#define INACTIVE -1

typedef struct {
    int shape;          //shape id, 0-6
    int x, y;           //coords, x=0 is left and y=0 is bottom
    int rotation;       //0, 1=R, 2, 3=L
    uint8_t mask[25];   //shape mask
    int size;           //mask size
} Piece;

typedef enum {
    GENERATION = 0,
    FALLING,
    LOCK,
    CLEAR
} GamePhase;

extern uint8_t matrix[M_HEIGHT][M_WIDTH];
extern uint32_t score;
extern volatile int game_over;
extern Piece active_piece;      //currently active piece
extern Piece ghost_piece;       //ghost piece / shadow of active piece
extern int held_piece_shape;    //shape of held piece
extern bool hold_avail;         //can hold piece
extern int rand_bag[14];        //bag of upcoming pieces
extern int rand_bag_loc;        //index of bag
extern GamePhase cur_phase;

extern const int piece_mask_sizes[7];
extern const uint8_t piece_masks[7][25];   //from bottom left to top right

int game_loop();
void init_game_blank();

#endif