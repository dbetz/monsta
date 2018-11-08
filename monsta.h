/* monsta.h - definitions for the monsta game */

#ifndef __MONSTA_H__
#define __MONSTA_H__

#include <stdint.h>

/* useful definitions */
#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

/* maximum path length */
#define PMAX    6

/* initial actor movement rates */
#define PRATE           1
#define MRATE           (irand(3) * 10 + 20)

/* expressions to determine the number of objects */
#define NRANDOMIZERS    (irand(5) + 4)
#define NCLUBS          (irand(4) + 3)
#define NBOMBS          (irand(3) + 2)

/* maze access macros */
#define mazewidth(m)            (((m)->xsize - 1) / 2)
#define mazeheight(m)           (((m)->ysize - 1) / 2)
#define mazeloc(m, x, y)        ((m)->data[(y) * (m)->pitch + (x)])
#define getpiece(m, x, y)       (mazeloc(m, x, y) & ~CHANGED)
#define haschanged(m, x, y)     (mazeloc(m, x, y) & CHANGED)
#define clearchanged(m, x, y)   (mazeloc(m, x, y) &= ~CHANGED)
#define putpiece(m, x, y, p)    do { 										\
									if ((p) != getpiece(m, x, y))			\
										mazeloc(m, x, y) = CHANGED | (p);	\
								} while (0)

/* location contents tokens (tile numbers) */
#define PLAYER                  0       /* player (actor tokens must be first) */
#define MONSTER                 1       /* first monster */
#define MONSTER2                2       /* second monster */
#define MONSTER3                3       /* third monster */
#define MONSTER4                4       /* fourth monster */
#define WALL                    5       /* wall */
#define HIDDENWALL              6       /* hidden wall */
#define RANDOMIZER              7       /* a randomizer */
#define HIDDENRANDOMIZER        8       /* a hidden randomizer */
#define EMPTY                   9       /* nothing */
#define FOOTPRINT               10      /* the player's footprint */
#define BOMB                    11      /* an inactive bomb */
#define ACTIVEBOMB              12      /* an active bomb */
#define CLUB                    13      /* a club */
#define GOAL                    14      /* the goal */
#define DEBRIS                  15      /* debris after an explosion */

/* location changed flag */
#define CHANGED					0x80

/* maximum level */
#define MAXLEVEL                4

/* number of actors */
#define NACTORS                 (1 + MAXLEVEL)

/* macros to identify actors */
#define IS_ACTOR(t)             ((t) < NACTORS)
#define IS_PLAYER(t)            ((t) == 0)
#define IS_MONSTER(t)           ((t) > 0 && (t) < NACTORS)

/* direction codes */
enum {
    NONE = -1,
    N = 0,  /* 0-3 must be directions */
    S,
    E,
    W,
    DROP,
    CHEAT1,
    CHEAT2,
    START,
    INC,
    DEC,
    DEMO,
    QUIT
};

/* types */
typedef struct maze MAZE;
typedef struct actor ACTOR;

/* actor handler pointer */
typedef int (*AHANDLER)(ACTOR *actor, int msg, void *arg);

/* actor structure definition */
struct actor {
    MAZE *maze;                 /* maze actor is in */
    int visible;                /* TRUE if actor is visible, FALSE if actor is hidden */
    int piece;                  /* piece for this actor */
    int mazePiece;              /* maze piece actor is standing on */
    int newPiece;               /* piece to display when actor moves */
    int x, y;                   /* position */
    int nbombs;                 /* number of bombs actor has */
    int nclubs;                 /* number of clubs actor has */
    int nrandomizers;           /* number of randomizers actor has seen */
    unsigned int rate;          /* clock ticks between moves */
    unsigned int time;          /* time for next move */
    AHANDLER handler;           /* handler function */
    ACTOR *next;                /* next actor in the scheduler queue */
};

/* maze structure definition */
struct maze {
    int level;                  /* game level (zero is demo mode) */
    int built;                  /* maze has been built */
    int playing;                /* play in progress */
    int xsize, ysize;           /* size */
    int xstart, ystart;         /* starting location */
    int xgoal, ygoal;           /* goal location */
    int nbombs;                 /* number of bombs */
    int nclubs;                 /* number of clubs */
    int nrandomizers;           /* number of randomizers */
    uint8_t *data;              /* start of each maze row */
    int pitch;                  /* number of bytes from the start of one row to the next */
    ACTOR actors[NACTORS];      /* actors (player is first) */
    ACTOR *queue;               /* scheduler queue */
    long now;                   /* the current time */
    int userData;               /* user data */
};

/* monsta.c */
void InitMaze(MAZE *maze, int xSize, int ySize, uint8_t *data, int pitch, int userData);
void UpdateMaze(MAZE *maze);
void HandleInput(MAZE *maze, int key);
void GameIdle(MAZE *maze, unsigned int time);

/* ???stuff.c */
int irand(int n);
void flash(void);
void beep(void);
void ShowPiece(MAZE *maze, int x, int y);
void ShowMessage(MAZE *maze, char *fmt, ...);
void ShowStatus(MAZE *maze);

/* actor message types */
#define MSG_INIT        1
#define MSG_MOVE        2
#define MSG_ENCOUNTER   3

#endif
