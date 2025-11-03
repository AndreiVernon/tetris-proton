#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <hardware/gpio.h>
#include "pico/stdlib.h"
#include "tetris.h"

uint8_t matrix[M_HEIGHT][M_WIDTH];
uint32_t score = 0;
bool gameOver = false;
uint8_t currPiece, currRotation, currX, currY, heldPiece;
uint8_t randBag[14];
uint8_t randBagLoc;

const int tetrominoes[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},  // blank
    {0, 0, 0, 0, 1, 1, 1, 1},  // I (1)
    {0, 1, 1, 0, 0, 1, 1, 0},  // O (2)
    {0, 1, 1, 0, 1, 1, 0, 0},  // S (3)
    {1, 1, 0, 0, 0, 1, 1, 0},  // Z (4)
    {0, 1, 0, 0, 1, 1, 1, 0},  // T (5)
    {0, 0, 1, 0, 1, 1, 1, 0},  // L (6)
    {1, 0, 0, 0, 1, 1, 1, 0}   // J (7)
};

int gameloop() {
    memset(matrix, 0, sizeof(uint8_t) * M_HEIGHT * M_WIDTH);
    srand(timer_hw->timerawl);

    //set up bag
    genRandBag(false);
    genRandBag(true);

    newPiece();

    int curFrame = 0;

    while (!gameOver) {
        int targetFrametime = (curFrame + 1) * 100000 / TARGET_FRAMERATE;  //in us
        curFrame = (curFrame + 1) % TARGET_FRAMERATE;


    }

    return 0;
}

//place new tetrimino
void newPiece() {
    currPiece = randBag[randBagLoc++];

    currRotation = 0;
    //currX = (A_WIDTH / 2) - (T_WIDTH / 2);
    currY = 0;

    //got to end of current bag
    if (randBagLoc >= 7) {
        //move second half pieces to first half
        for (int i = 0; i < 7; i++)
            randBag[i] = randBag[i+7];
        
        randBagLoc = 0;
        genRandBag(true);
    }

    //gameOver = !validPos(currTetrominoIdx, currRotation, currX, currY);
}

//generates random bag of tetriminos
void genRandBag(bool secondHalf) {
    int *arr = &randBag[secondHalf * 7];

    for (int i = 0; i < 7; i++)
        arr[i] = i + 1;

    //Fisher-Yates shuffle
    for (int i = 7 - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

//move tetrimino
void move() {

}

//rotate tetrimino
void rotate() {

}

//drop tetrimino one space
void softDrop() {

}

//hard drop tetrimino
void hardDrop() {

}

//hold / swap held piece
void holdPiece() {

}