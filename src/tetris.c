#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "tetris.h"
#include "graphics.h"


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
#define EMPTY 255


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

Piece piece;         //currently active piece
int held_piece = -1;        //shape of held piece
bool hold_avail = true;     //can hold piece
int rand_bag[14];           //bag of upcoming pieces
int rand_bag_loc;           //index of bag

const int piece_mask_sizes[7] = {5, 3, 3, 3, 3, 3, 3};
//from bottom left to top right
const uint8_t piece_masks[7][25] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // I (0)
    {0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // O (1)
    {0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // S (2)
    {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // Z (3)
    {0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // T (4)
    {0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // L (5)
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // J (6)
};  //      ||       ||       || (3x3)


int game_loop() {
    memset(matrix, EMPTY, sizeof(uint8_t) * M_HEIGHT * M_WIDTH);
    
    //set up bag
    gen_rand_bag(false);
    gen_rand_bag(true);

    //game start animation here

    new_piece(-1);

    while (!game_over) {
        //process_inputs here

        //render_frame here
        while (!frame_ready) tight_loop_contents();
        //display_frame here (set frame ready to false at start of this func)
    }

    return 0;
}

//place new tetrimino
void new_piece(int new_shape) {
    //default
    if (new_shape == -1) piece.shape = rand_bag[rand_bag_loc++];
    //called by hold_piece
    else piece.shape = new_shape;

    piece.rotation = 0;
    
    //I piece is 5x5
    if (piece.shape == I_PIECE) {
        piece.x = 2;
        piece.y = 22;
        piece.size = 5;
    } else {
        piece.x = 3;
        piece.y = 21;
        piece.size = 3;
    }
    
    //get shape mask
    memcpy(piece.mask, piece_masks[piece.shape], piece.size * piece.size);

    //got to end of current bag
    if (rand_bag_loc >= 7) {
        //move second half pieces to first half
        for (int i = 0; i < 7; i++)
            rand_bag[i] = rand_bag[i+7];
        
        rand_bag_loc = 0;
        gen_rand_bag(true);
    }

    //gameOver = !validPos(currTetrominoIdx, currRotation, currX, currY);
}

//generates random bag of tetriminos
void gen_rand_bag(bool second_half) {
    int *arr = &rand_bag[second_half ? 7 : 0];

    //fill array with numbers 0-6
    for (int i = 0; i < 7; i++)
        arr[i] = i;

    //Fisher-Yates shuffle (uniform sampling)
    for (int i = 6; i > 0; i--) {
        int j;
        int limit = UINT32_MAX - (UINT32_MAX % (i + 1));

        do {
            j = rand();
        } while (j >= limit);

        j %= (i + 1);

        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

//move piece
void move() {

}

//rotate piece
void rotate() {

}

//drop piece slowly
void soft_drop() {

}

//drop piece instantly
void hard_drop() {

}

//hold / swap held piece
void hold_piece() {
    if (!hold_avail) return;
    else hold_avail = false;

    int temp = held_piece;
    held_piece = piece.shape;

    //if originally held piece is -1 (empty), new piece will come from bag
    new_piece(temp);
    return;
}

//locks location of the piece and adds to playfield
void lock_piece() {
    for (int y = 0; y < piece.size; y++) {
        for (int x = 0; x < piece.size; x++) {
            if (piece.mask[y * piece.size + x]) {
                matrix[piece.y + y][piece.x + x] = piece.shape;
            }
        }
    }
}

//checks for completed lines and removes them
void check_lines() {

}

/* //convert xy coordinates to index of playfield matrix
int coord_to_matrix(int x, int y) {
    return (M_HEIGHT - y - 1) * M_WIDTH + x;
} */