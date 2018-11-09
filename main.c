#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <propeller.h>
#include "monsta.h"
#include "wordfire.h"

#define VOL_PIN     20
#define SND_PIN     21
#define SND_MASK    (1 << SND_PIN)
#define BEEP_FREQ   2000

#define KEY_LEFT	0xe4
#define KEY_RIGHT	0xe6
#define KEY_UP		0xe8
#define KEY_DOWN	0xe2

extern uint32_t glyphs[];
                                                                                                                                
uint8_t board[4][WIDTH * (HEIGHT - 1)];
MAZE mazes[4];

int main(void)
{
    uint32_t ticksPerTenthSecond, lastTick, now;
    int i, j;
    
    DIRA |= 1 << VOL_PIN;
    DIRA |= 1 << SND_PIN;
    
    CTRA = (0x30 << 23) | (1 << 9) | VOL_PIN;   //counter mode = duty cycle for d/a using rc circuit (B pin is unused)
    FRQA = 0x20000000;                          //Vout = 3.3 * (frqa/2^32) = 3.3 * 0.5 = 1.65V max when frqa = $8000_0000 (can omit this line) 

    //printf("Starting...\n");
    
    if (quadkeyboardStart() < 0) {
        printf("quadkeyboardStart failed\n");
        return 1;
    }
    
    if (quadvgaStart() < 0) {
        printf("quadvgaStart failed\n");
        return 1;
    }
    
        
    for (i = 0; i < 4; ++i) {
        quadvgaSetUserGlyphs(i, glyphs);
        InitMaze(&mazes[i], WIDTH, HEIGHT - 1, board[i], WIDTH, i);
	    UpdateMaze(&mazes[i]);
	}
	
    ticksPerTenthSecond = CLKFREQ / 10;
    lastTick = CNT;
	now = 0;
	
	while (1) {
	
	    if (CNT - lastTick >= ticksPerTenthSecond) {
	        lastTick = CNT;
	        for (i = 0; i < 4; ++i)
	            GameIdle(&mazes[i], now);
	        ++now;
	    }
	    
		for (i = 0; i < 4; ++i) {
		    MAZE *maze = &mazes[i];
		    int ch;
		    if ((ch = getKey(i)) != 0) {
		        switch (ch) {
                case KEY_LEFT:
                    MovePlayer(maze, W);
                    break;
                case KEY_RIGHT:
                    MovePlayer(maze, E);
                    break;
                case KEY_UP:
                    MovePlayer(maze, N);
                    break;
                case KEY_DOWN:
                    MovePlayer(maze, S);
                    break;
                case 'b':
                case 'B':
                    SetLevel(maze, 0);
                    break;
                case 's':
                case 'S':
                    if (!maze->playing) {
                        srand(CNT);
                        BuildMaze(maze);
                        for (j = 0; j < 4; ++j) {
                            if (j != 0) {
                                MAZE *clone = &mazes[j];
                                CloneMaze(clone, maze);
                                Start(clone);
                            }
                        }
                        Start(maze);
                    }
                    break;
                case 'c':
                    Cheat(maze, CHEAT1);
                    break;
                case 'C':
                    Cheat(maze, CHEAT2);
                    break;
                case 'd':
                case 'D':
                    Drop(maze);
                    break;
                case 'q':
                case 'Q':
                    Quit(maze);
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                    SetLevel(maze, ch - '0');
                    break;
                }
            }
		}

		for (i = 0; i < 4; ++i)
		    UpdateMaze(&mazes[i]);
    }
	
    return 0;
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
    int cnt = 200;
    while (--cnt >= 0) {
        OUTA &= ~SND_MASK;
        waitcnt(CLKFREQ / BEEP_FREQ + CNT);
        OUTA |= SND_MASK;
        waitcnt(CLKFREQ / BEEP_FREQ + CNT);
    }
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
        piece = 0x01; //'O';
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
    quadvgaPoke(maze->userData, x, y, piece);
}

void ShowMessage(MAZE *maze, char *fmt, ...)
{
    char buf[81];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    quadvgaClearLine(maze->userData, HEIGHT - 1);
    quadvgaSetXY(maze->userData, 0, HEIGHT - 1);
    quadvgaStr(maze->userData, buf);
}

void ShowStatus(MAZE *maze)
{
    ACTOR *actor = &maze->actors[0];
    ShowMessage(maze,
                "L:%d  C:%d  B:%d       ",
                maze->level,
                actor->nclubs, 
                actor->nbombs);
}
