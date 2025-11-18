#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "tetris.h"
#include "graphics.h"


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
    //clear playfield
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
        piece.y = 18;
        piece.size = 5;
    } else if (piece.shape == O_PIECE) {
        piece.x = 4;
        piece.y = 20;
        piece.size = 2;
    } else {
        piece.x = 3;
        piece.y = 19;
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

    //game over check
    game_over = is_colliding();
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

//move piece left or right
//dir: 0 = left, 1 = right
void move(bool dir) {
    int x_old = piece.x;

    if (dir) piece.x++;
    else piece.x--;

    if (is_colliding()) piece.x = x_old;
}


int8_t rotate_offset_data[4][5][2] = {
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},   //0
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},  //R
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},   //2
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}//L
};

//ccw at even indices, cw at odd indices
int8_t rotate_offset_data_i_base[8][2] = {
    {0, -1},    //0->L
    {1, 0},     //0->R
    {-1, 0},    //R->0
    {0, -1},    //R->2
    {0, 1},     //2->R
    {-1, 0},    //2->L
    {1, 0},     //L->2
    {0, 1}      //L->0
};

int8_t rotate_offset_data_i_arika[8][5][2] = {
    {{0,0}, {2,0}, {-1,0}, {-1,2}, {2,-1}},   //0->L
    {{0,0}, {-2,0}, {1,0}, {1,2}, {-2,-1}},   //0->R
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}},   //R->0
    {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}},   //R->2
    {{0,0}, {-2,0}, {1,0}, {-2,1}, {1,-1}},   //2->R
    {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-1}},   //2->L
    {{0,0}, {1,0}, {-2,0}, {1,2}, {-2,-1}},   //L->2
    {{0,0}, {-2,0}, {1,0}, {-2,1}, {1,-2}}    //L->0
};

//rotate piece
//cw: rotate clockwise
void rotate(bool cw) {
    if (piece.shape == O_PIECE) return;

    uint8_t mask_old[25];
    memcpy(mask_old, piece.mask, piece.size * piece.size);
    int size = piece.size;

    //perform matrix rotation on piece mask
    if (cw) {
        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                piece.mask[c * size + (size - 1 - r)] = mask_old[r * size + c];
            }
        }
    } else {
        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                piece.mask[(size - 1 - c) * size + r] = mask_old[r * size + c];
            }
        }
    }
    
    int rotation_target = (piece.rotation + (cw ? 1 : -1)) % 4;
    int x_old = piece.x;
    int y_old = piece.y;
    bool success = false;

    //add piece offsets
    if (piece.shape == I_PIECE) {
        for (int i = 0; i < 5; i++) {
            piece.x = x_old + rotate_offset_data_i_base[piece.rotation*2 + cw][0] + rotate_offset_data_i_arika[piece.rotation*2 + cw][i][0];
            piece.y = y_old + rotate_offset_data_i_base[piece.rotation*2 + cw][1] + rotate_offset_data_i_arika[piece.rotation*2 + cw][i][1];
            
            if (!is_colliding()) {
                success = true;
                break;
            }
        }
    } else {
        for (int i = 0; i < 5; i++) {
            piece.x = x_old + rotate_offset_data[piece.rotation][i][0] - rotate_offset_data[rotation_target][i][0];
            piece.y = y_old + rotate_offset_data[piece.rotation][i][1] - rotate_offset_data[rotation_target][i][1];
            
            if (!is_colliding()) {
                success = true;
                break;
            }
        }
    }

    //if nothing worked, undo
    if (!success) {
        piece.x = x_old;
        piece.y = y_old;
        memcpy(piece.mask, mask_old, piece.size * piece.size);
        return;
    }

    //if succeeded, set new rotation state
    piece.rotation = rotation_target;
}

//drop piece slowly
void soft_drop() {
    piece.y--;
    if (is_colliding()) piece.y++;
}

//update ghost piece
void update_ghost() {
    ghost_piece = piece;
    ghost_piece.shape = GHOST;
    hard_drop(true);
}

//drop piece instantly
void hard_drop(bool ghost) {
    Piece* piece_sel = ghost ? &ghost_piece : &piece;

    //go from -2 to height+2 cause thats the highest/lowest a piece can be within mask
    for (int y = -2; y < M_HEIGHT+2; y++) {
        piece_sel->y = y;
        if (!is_colliding()) break;
    }

    if (!ghost) lock_piece();
}

void do_gravity() {

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

//check if current piece is colliding with blocks on playfield
//returns true if colliding or out of bounds
bool is_colliding() {
    for (int y = 0; y < piece.size; y++) {
        for (int x = 0; x < piece.size; x++) {
            //check every active block in piece mask
            if (piece.mask[y * piece.size + x]) {
                //check for oob
                if (piece.x + x < 0 || piece.x + x >= M_WIDTH || piece.y + y < 0 || piece.y + y >= M_HEIGHT)
                    return true;

                //check for collision
                if (matrix[piece.y + y][piece.x + x] != EMPTY)
                    return true;
            }
        }
    }

    return false;
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

    //game over check
    //only game over if every block in piece is oob
    if (piece.y >= 20 - piece.size) {
        bool oob = true;

        for (int y = 0; y < piece.size; y++) {
            for (int x = 0; x < piece.size; x++) {
                if (piece.mask[y * piece.size + x]) {
                    if (piece.y + y < 20) {
                        oob = false;
                        goto nested_break;
                    }
                }
            }
        }
        nested_break:

        if (oob) game_over = true;
    }
}

//shifts every line above `row` by amount
void shift_lines(int row, int amount) {
    if (amount == 0) return;

    //direction matters, avoid overwriting line data
    if (amount > 0) {
        for (int y = M_HEIGHT - 1; y >= row; y--) {
            //avoid out of bounds access
            if ((y - amount >= M_HEIGHT) || (y - amount < 0))
                memset(matrix[y], EMPTY, M_WIDTH);
            else
                memcpy(matrix[y], matrix[y - amount], M_WIDTH);
        }
    } else {
        for (int y = row; y < M_HEIGHT; y++) {
            //avoid out of bounds access
            if ((y - amount >= M_HEIGHT) || (y - amount < 0))
                memset(matrix[y], EMPTY, M_WIDTH);
            else
                memcpy(matrix[y], matrix[y - amount], M_WIDTH);
        }
    }

    //game over check
    //only game over if shift up + piece oob,
    //regardless of whether shift caused piece to be oob
    if (amount > 0) {
        for (int y = 20; y < M_HEIGHT; y++) {
            for (int x = 0; x < M_WIDTH; x++) {
                if (x != EMPTY) {
                    game_over = true;
                    goto nested_break;
                }
            }
        }
    }
    nested_break:

}

//checks for completed lines and removes them
void check_lines() {
    for (int y = 0; y < M_HEIGHT; y++) {
        int cleared_cnt = 0;

        for (int x = 0; x < M_WIDTH; x++) {
            if (matrix[y][x] == EMPTY)
                break;
            else
                cleared_cnt++;
        }

        //line has been cleared
        if (cleared_cnt == M_WIDTH) {
            shift_lines(y, -1);
            //TODO: score up
        }
    }
}

//add garbage in multiplayer moded
void add_garbage() {
    //gonna have to come up with garbage shapes
    //shift_lines(0, size_of_garbage);
}

/* //convert xy coordinates to index of playfield matrix
int coord_to_matrix(int x, int y) {
    return (M_HEIGHT - y - 1) * M_WIDTH + x;
} */