#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <propeller.h>
#include "monsta.h"
#include "wordfire.h"

static int GetKey(void);

static uint32_t ticksPerTenthSecond;

int main(void)
{
    MAZE maze;
    uint8_t board[WIDTH * (HEIGHT - 1)];
    uint32_t lastTick;
    int ch, i;
    
    ticksPerTenthSecond = CLKFREQ / 10;
    lastTick = CNT;
    
    if (quadkeyboardStart() < 0) {
        printf("quadkeyboardStart failed\n");
        return 1;
    }
    
    if (quadvgaStart() < 0) {
        printf("quadvgaStart failed\n");
        return 1;
    }
    
    InitMaze(&maze, WIDTH, HEIGHT - 1, board, WIDTH);
	UpdateMaze(&maze);
	
	while (1) {
	    int key;
	
	    if (CNT - lastTick >= ticksPerTenthSecond) {
	        lastTick = CNT;
	        GameIdle(&maze, lastTick);
	    }
	    
		if ((key = GetKey()) != NONE)
			HandleInput(&maze, key);

		UpdateMaze(&maze);
    }
	
    printf("Start typing...\n");
    while (1) {
        for (i = 0; i < 4; ++i) {
            if ((ch = getKey(i)) != 0) {
                printf("%d: %02x '%c'\n", i, ch, ch);
            }
        }
    }

    printf("Done\n");
    while (1)
        ;
    
    return 0;
}

#define KEY_LEFT	0xe4
#define KEY_RIGHT	0xe6
#define KEY_UP		0xe8
#define KEY_DOWN	0xe2

static int GetKey(void)
{
    int key = NONE;
    switch (getKey(0)) {
    case KEY_LEFT:
        key = W;
        break;
    case KEY_RIGHT:
        key = E;
        break;
    case KEY_UP:
        key = N;
        break;
    case KEY_DOWN:
        key = S;
        break;
    case 'b':
    case 'B':
        key = DEMO;
        break;
    case 's':
    case 'S':
        key = START;
        srand(CNT);
        break;
    case 'c':
        key = CHEAT1;
        break;
    case 'C':
        key = CHEAT2;
        break;
    case 'd':
    case 'D':
        key = DROP;
        break;
    case 'q':
    case 'Q':
        key = QUIT;
        break;
    }
	return key;
}

int irand(int n)
{
    return rand() % n;
}

void flash(void)
{
}

void beep(void)
{
}

void ShowPiece(MAZE *maze, int x, int y)
{
    int piece;
    switch (getpiece(maze, x, y)) {
    case PLAYER:            /* player (actor tokens must be first) */
        piece = 'P';
        break;
    case MONSTER:           /* first monster */
    case MONSTER2:          /* second monster */
    case MONSTER3:          /* third monster */
        piece = 'M';
        break;
    case WALL:              /* wall */
        piece = 'X';
        break;
    case RANDOMIZER:        /* a randomizer */
        piece = '?';
        break;
    case HIDDENWALL:        /* hidden wall */
    case HIDDENRANDOMIZER:  /* a hidden randomizer */
    case EMPTY:             /* nothing */
        piece = ' ';
        break;
    case FOOTPRINT:         /* the player's footprint */
        piece = '.';
        break;
    case BOMB:              /* an inactive bomb */
        piece = 'b';
        break;
    case ACTIVEBOMB:        /* an active bomb */
        piece = 'B';
        break;
    case CLUB:              /* a club */
        piece = '!';
        break;
    case GOAL:              /* the goal */
        piece = '*';
        break;
    case DEBRIS:            /* debris after an explosion */
        piece = '%';
        break;
    default:
        piece = '@';
        break;
    }
    quadvgaPoke(0, x, y, piece);
}

void ShowMessage(char *fmt, ...)
{
    char buf[81];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    quadvgaSetXY(0, 0, HEIGHT - 1);
    quadvgaStr(0, buf);
}

void ShowStatus(MAZE *maze)
{
    ACTOR *actor = &maze->actors[0];
    ShowMessage("C:%d/%d  B:%d/%d  R:%d  ",
                actor->nclubs, maze->nclubs,
                actor->nbombs, maze->nbombs,
                maze->nrandomizers);
}
