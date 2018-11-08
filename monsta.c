/* db_monsta_v002.c - a maze game with a monster on the loose */

#include <stdlib.h>
#include <string.h>
#include "monsta.h"

/* check to see if a location is valid */
#define valid(m, x, y)  ((x) >= 0 && (x) < (m)->xsize && (y) >= 0 && (y) < (m)->ysize)

/* x and y offsets for each direction */
/*                       N  S  E  W   */
static int xoff1[4] = {  0, 0, 1,-1 };
static int yoff1[4] = { -1, 1, 0, 0 };
static int xoff2[4] = {  0, 0, 2,-2 };
static int yoff2[4] = { -2, 2, 0, 0 };

/* combinations of directions */
static char dir[24][4] = {
    {N,S,E,W},  {N,S,W,E},  {N,E,S,W},  {N,E,W,S},  {N,W,S,E},  {N,W,E,S},
    {S,N,E,W},  {S,N,W,E},  {S,E,N,W},  {S,E,W,N},  {S,W,N,E},  {S,W,E,N},
    {E,N,S,W},  {E,N,W,S},  {E,S,N,W},  {E,S,W,N},  {E,W,N,S},  {E,W,S,N},
    {W,N,S,E},  {W,N,E,S},  {W,S,N,E},  {W,S,E,N},  {W,E,N,S},  {W,E,S,N} };

/* forward declarations */
static int startgame(MAZE *maze, unsigned int time);
static int buildmaze(MAZE *maze, int level);
static int fillmaze(MAZE *maze);
static void unhiderandomizers(MAZE *maze);
static void unhidemaze(MAZE *maze);
static void randomloc(MAZE *maze, int *px, int *py);
static int visited(MAZE *maze, int x, int y);
static int findmove(MAZE *maze, int x, int y);
static int backup(MAZE *maze, int *px, int *py);
static void removewall(MAZE *maze, int *px, int *py, int d);
static void init_player(MAZE *maze, ACTOR *actor);
static int phandler(ACTOR *actor, int msg, void *arg);
static int pinit(ACTOR *actor);
static int pmove(ACTOR *actor);
static void moveplayer(MAZE *maze, int dir);
static void drop(MAZE *maze);
static int pencounter(ACTOR *actor, ACTOR *other);
static int pbumpmonster(ACTOR *actor, ACTOR *other);
static int pbumpplayer(ACTOR *actor, ACTOR *other);
static void init_monster(MAZE *maze, ACTOR *actor);
static int mhandler(ACTOR *actor, int msg, void *arg);
static int minit(ACTOR *actor);
static int mmove(ACTOR *actor);
static int mdirection(ACTOR *actor);
static int mencounter(ACTOR *actor, ACTOR *other);
static int mbumpplayer(ACTOR *actor, ACTOR *other);
static int mbumpmonster(ACTOR *actor, ACTOR *other);
static void act_init(MAZE *maze, ACTOR *actor, int piece, int x, int y, int rate, AHANDLER hndlr);
static void act_schedule(ACTOR *actor, unsigned int time);
static void act_show(ACTOR *actor);
static void act_hide(ACTOR *actor);
static void act_move(ACTOR *actor, int dir);
static void bumpwall(ACTOR *actor, int dir, int piece);
static int checkencounter(ACTOR *actor);
static int explosion(MAZE *maze, int xcenter, int ycenter);
static int iswall(int piece);
static void moverandom(ACTOR *actor);

/* InitMaze - initialize a maze */
void InitMaze(MAZE *maze, int xSize, int ySize, uint8_t *data, int pitch, int userData)
{
	int centerx, centery, x, y;
	
	/* make sure the sizes are 2n+1 */
	xSize = ((xSize - 1) & ~1) + 1;
    ySize = ((ySize - 1) & ~1) + 1;
    
    memset(maze, 0, sizeof(MAZE));
    maze->userData = userData;
    maze->xsize = xSize;
    maze->ysize = ySize;
    maze->data = data;
    maze->pitch = pitch;
    maze->level = 1;

    centerx = maze->xsize / 2;
	centery = maze->ysize / 2;
    for (y = 0; y < ySize; ++y)
        for (x = 0; x < xSize; ++x)
            putpiece(maze, x, y, EMPTY);

    putpiece(maze, centerx - 1, centery, PLAYER);
    putpiece(maze, centerx + 1, centery, MONSTER);
    ShowMessage(maze, "Monsta! Press Start");

	UpdateMaze(maze);
}

/* HandleInput - handle a user input */
void HandleInput(MAZE *maze, int key)
{
    switch (key) {
    case N:             /* north */
    case S:             /* south */
    case E:             /* east */
    case W:             /* west */
        if (maze->playing)
            moveplayer(maze, key);
        break;
    case DROP:          /* drop a bomb */
        if (maze->playing)
            drop(maze);
        break;
    case START:         /* start game */
        if (!maze->playing)
            startgame(maze, maze->now);
        break;
    case INC:
        if (maze->level < MAXLEVEL) {
            ++maze->level;
            ShowStatus(maze);
        }
        break;
    case DEC:
        if (maze->level > 1) {
            --maze->level;
            ShowStatus(maze);
        }
        break;
    case QUIT:
        if (maze->playing) {
            ShowMessage(maze, "Game aborted!!");
            maze->playing = FALSE;
        }
        break;
    case CHEAT1:        /* unhide randomizers */
        unhiderandomizers(maze);
        ShowStatus(maze);
        break;
    case CHEAT2:        /* unhide maze */
        unhiderandomizers(maze);
        unhidemaze(maze);
        ShowStatus(maze);
        break;
    case DEMO:          /* demo mode */
        if (maze->level != 0) {
            maze->level = 0;
            ShowStatus(maze);
        }
        break;
    }
}

/* GameIdle - handle idle processing */
void GameIdle(MAZE *maze, unsigned int time)
{
    if (maze->playing) {
        ACTOR *actor;
        maze->now = time;
        while ((actor = maze->queue) != NULL) {
            if (maze->now < actor->time)
                break;
            maze->queue = actor->next;
            if (!(*actor->handler)(actor, MSG_MOVE, NULL)) {
                maze->playing = FALSE;
                break;
            }
        }
    }
}

/* startgame - start the game */
static int startgame(MAZE *maze, unsigned int time)
{
    int i, n;

    /* build and populate the maze */
    if (!buildmaze(maze, maze->level))
        return FALSE;

    /* initialize */
    maze->now = time;
    maze->queue = NULL;

    /* initialize the player */
    init_player(maze, &maze->actors[0]);

    /* initialize the monsters */
    if ((n = maze->level) == 0)
        n = 1;
    for (i = 1; i <= n && i < NACTORS; ++i)
        init_monster(maze, &maze->actors[i]);
            
    /* update the status line */
    ShowStatus(maze);

    /* start playing */
    maze->playing = TRUE;

    /* return successfully */
    return TRUE;
}

/* buildmaze - build a maze */
static int buildmaze(MAZE *maze, int level)
{
    int remaining, plen, len, lmax, x, y, d;

    /* initialize for build */
    maze->level = level;
    maze->built = FALSE;

    /* initialize the maze */
    for (y = 0; y < maze->ysize; ++y)
        for (x = 0; x < maze->xsize; ++x)
            putpiece(maze, x, y, HIDDENWALL);
    for (y = 1; y < maze->ysize; y += 2)
        for (x = 1; x < maze->xsize; x += 2)
            putpiece(maze, x, y, 0);

    /* choose a starting location */
    randomloc(maze, &x, &y);
    maze->xstart = maze->xgoal = x;
    maze->ystart = maze->ygoal = y;
    putpiece(maze, x, y, 1);
    lmax = 0;

    /* visit all of the maze locations */
    remaining = mazeheight(maze) * mazewidth(maze);
    for (plen = 0; --remaining > 0; ++plen) {

        /* choose a random path from this location */
        if (plen >= PMAX || (d = findmove(maze, x, y)) == NONE) {
            do {
                if (!backup(maze, &x, &y))
                    return FALSE;
            } while ((d = findmove(maze, x, y)) == NONE);
            plen = 0;
        }

        /* break down the wall and extend the path */
        removewall(maze, &x, &y, d);

        /* check for this being the longest path so far */
        if ((len = getpiece(maze, x, y)) > lmax) {
            lmax = len;
            maze->xgoal = x;
            maze->ygoal = y;
        }
    }

    /* empty all of the locations */
    for (y = 1; y < maze->ysize; y += 2)
        for (x = 1; x < maze->xsize; x += 2)
            putpiece(maze, x, y, EMPTY);

    /* unhide all of the exterior walls */
    for (x = 0; x < maze->xsize; ++x) {
        putpiece(maze, x, 0, WALL);
        putpiece(maze, x, maze->ysize - 1, WALL);
    }
    for (y = 1; y < maze->ysize - 1; ++y) {
        putpiece(maze, 0, y, WALL);
        putpiece(maze, maze->xsize - 1, y, WALL);
    }

    /* fill the maze with objects */
    if (!fillmaze(maze))
        return FALSE;
    
    /* mark maze as built */
    maze->built = TRUE;
    return TRUE;
}

/* fillmaze - fill the maze with interesting objects */
static int fillmaze(MAZE *maze)
{
    int max, n, x, y;

    /* place the goal */
    putpiece(maze, maze->xgoal, maze->ygoal, GOAL);

    /* place randomizers */
    maze->nrandomizers = n = NRANDOMIZERS; max = 32000;
    while (--n >= 0) {
        do {
            int d = irand(4);
            if (--max < 0) return FALSE;
            randomloc(maze, &x, &y);
            x += xoff1[d];
            y += yoff1[d];
        } while (getpiece(maze, x, y) != HIDDENWALL);
        putpiece(maze, x, y, HIDDENRANDOMIZER);
    }

    /* place clubs */
    maze->nclubs = n = NCLUBS; max = 32000;
    while (--n >= 0) {
        do {
            if (--max < 0) return FALSE;
            randomloc(maze, &x, &y);
        } while (getpiece(maze, x, y) != EMPTY);
        putpiece(maze, x, y, CLUB);
    }

    /* place bombs */
    maze->nbombs = n = NBOMBS; max = 32000;
    while (--n >= 0) {
        do {
            if (--max < 0) return FALSE;
            randomloc(maze, &x, &y);
        } while (getpiece(maze, x, y) != EMPTY);
        putpiece(maze, x, y, BOMB);
    }
    return TRUE;
}

/* UpdateMaze - update the maze image on the screen */
void UpdateMaze(MAZE *maze)
{
	int x, y;
	for (y = 0; y < maze->ysize; ++y)
		for (x = 0; x < maze->xsize; ++x)
			if (haschanged(maze, x, y)) {
				clearchanged(maze, x, y);
				ShowPiece(maze, x, y);
			}
}

/* unhiderandomizers - unhide all randomizers */
static void unhiderandomizers(MAZE *maze)
{
    int x, y;
    for (y = 0; y < maze->ysize; ++y)
        for (x = 0; x < maze->xsize; ++x)
            if (getpiece(maze, x, y) == HIDDENRANDOMIZER) {
                putpiece(maze, x, y, RANDOMIZER);
                --maze->nrandomizers;
            }
}

/* unhidemaze - unhide the whole maze */
static void unhidemaze(MAZE *maze)
{
    int x, y;
    for (y = 0; y < maze->ysize; ++y)
        for (x = 0; x < maze->xsize; ++x)
            if (getpiece(maze, x, y) == HIDDENWALL)
                putpiece(maze, x, y, WALL);
}

/* randomloc - compute a random location in the maze */
static void randomloc(MAZE *maze, int *px, int *py)
{
    *px = irand(mazewidth(maze)) * 2 + 1;
    *py = irand(mazeheight(maze)) * 2 + 1;
}

/* visited - determine if a location has been visited */
static int visited(MAZE *maze, int x, int y)
{
    return valid(maze, x, y) ? getpiece(maze, x, y) != 0 : TRUE;
}

/* findmove - find a move from a location */
static int findmove(MAZE *maze, int x, int y)
{
    int dv,d,i;

    /* choose a random collection of directions */
    dv = irand(24);

    /* try each direction randomly */
    for (i = 0; i < 4; ++i) {

        /* select a direction */
        d = dir[dv][i];

        /* return if this direction hasn't been visited */
        if (!visited(maze, x + xoff2[d], y + yoff2[d]))
            return d;
    }

    /* no direction is unvisited */
    return NONE;
}

/* backup - back up one move */
static int backup(MAZE *maze, int *px, int *py)
{
    int len, d;
    len = getpiece(maze, *px, *py) - 1;
    for (d = 0; d < 4; ++d) {
        int xx = *px + xoff2[d];
        int yy = *py + yoff2[d];
        if (valid(maze, xx, yy) && getpiece(maze, xx, yy) == len) {
            *px = xx;
            *py = yy;
            return TRUE;
        }
    }
    return FALSE;
}

/* removewall - remove a wall and move */
static void removewall(MAZE *maze, int *px, int *py, int d)
{
    int len;
    
    /* remove the wall in the forward direction */
    putpiece(maze, *px + xoff1[d], *py + yoff1[d], EMPTY);

    /* compute the path length of the next location */
    len = getpiece(maze, *px, *py) + 1;

    /* move to the new location */
    *px += xoff2[d];
    *py += yoff2[d];

    /* mark the new location */
    putpiece(maze, *px, *py, len);
}

/* init_player - initialize a player */
static void init_player(MAZE *maze, ACTOR *actor)
{
    act_init(maze,          /* the maze */
             actor,         /* the actor */
             PLAYER,        /* player piece */
             maze->xstart,  /* starting x position */
             maze->ystart,  /* starting y position */
             PRATE,         /* movement rate */
             phandler);     /* player handler */
}

/* phandler - player handler */
static int phandler(ACTOR *actor, int msg, void *arg)
{
    int sts = FALSE;
    switch (msg) {
    case MSG_INIT:
        sts = pinit(actor);
        break;
    case MSG_MOVE:
        sts = pmove(actor);
        break;
    case MSG_ENCOUNTER:
        sts = pencounter(actor, (ACTOR *)arg);
        break;
    }
    return sts;
}

/* pinit - player initialization routine */
static int pinit(ACTOR *actor)
{
    MAZE *maze = actor->maze;
    
    /* leave a footprint behind when the player moves */
    actor->newPiece = FOOTPRINT;

    /* setup demo mode */
    if (maze->level == 0) {
        actor->nclubs = 10;
        actor->nbombs = 3;
    }
    
    /* add actor to the scheduler queue */
    act_schedule(actor, maze->now);
    return TRUE;
}

/* pmove - handle the player's move */
static int pmove(ACTOR *actor)
{
    act_schedule(actor, actor->maze->now + actor->rate);
    return TRUE;
}

/* moveplayer - check the results of the player's move */
static void moveplayer(MAZE *maze, int dir)
{
    ACTOR *actor = &maze->actors[0];

    /* check for a valid move */
    switch (getpiece(maze, actor->x + xoff1[dir], actor->y + yoff1[dir])) {
    case HIDDENWALL:
        bumpwall(actor, dir, WALL);
        /* fall through */
    case WALL:
        break;
    case HIDDENRANDOMIZER:
        bumpwall(actor, dir, RANDOMIZER);
        ++actor->nrandomizers;
        --maze->nrandomizers;
        ShowStatus(maze);
        /* fall through */
    case RANDOMIZER:
        moverandom(actor);
        actor->newPiece = FOOTPRINT;
        ShowMessage(maze, "* * Sproing * *");
        beep();
        break;
    default:
        act_move(actor, dir);
        actor->newPiece = FOOTPRINT;
        act_move(actor, dir);
        actor->newPiece = FOOTPRINT;
        break;
    }

    /* see if we collided with a monster */
    if (checkencounter(actor)) {
    
        /* check to see what we landed on */
        switch (actor->mazePiece) {
        case CLUB:
            actor->mazePiece = EMPTY;
            ++actor->nclubs;
            --maze->nclubs;
            ShowStatus(maze);
            break;
        case BOMB:
            actor->mazePiece = EMPTY;
            ++actor->nbombs;
            --maze->nbombs;
            ShowStatus(maze);
            break;
        case GOAL:
            ShowMessage(maze, "* * Victory * *");
            maze->playing = FALSE;
            break;
        case ACTIVEBOMB:
            explosion(maze, actor->x, actor->y);
            maze->playing = FALSE;
            break;
        }
    }
}

/* drop - player drops a bomb */
static void drop(MAZE *maze)
{
    ACTOR *actor = &maze->actors[0];
    if (actor->nbombs == 0)
		ShowMessage(maze, "No bombs!");
	else if (actor->newPiece == ACTIVEBOMB)
		ShowMessage(maze, "Bomb here!");
	else {
	    --actor->nbombs;
	    ShowStatus(maze);
	    actor->newPiece = ACTIVEBOMB;
	    ShowMessage(maze, "Careful . . .");
	}
}

/* pencounter - encounter another actor */
static int pencounter(ACTOR *actor, ACTOR *other)
{
    return IS_MONSTER(other->piece) ? pbumpmonster(actor, other)
                                    : pbumpplayer(actor, other);
}

/* pbumpmonster - player bumps monster */
static int pbumpmonster(ACTOR *actor, ACTOR *other)
{
    if (actor->nclubs) {
        --actor->nclubs;
        ShowStatus(actor->maze);
        act_show(actor);
        moverandom(other);
        if ((other->rate -= 3) < 1)
            other->rate = 1;
        ShowMessage(actor->maze, "Ouch ! !");
        beep();
        return TRUE;
    }
    else {
        act_show(other);
        ShowMessage(actor->maze, "! ! Schloorp ! !");
        return FALSE;
    }
}

/* pbumpplayer - player bumps player */
static int pbumpplayer(ACTOR *actor, ACTOR *other)
{
    /* can't happen right now */
    return TRUE;
}

/* init_monster - initialize a monster */
static void init_monster(MAZE *maze, ACTOR *actor)
{
    int x,y;
    
    /* place the monster */
    do {
        randomloc(maze, &x, &y);
    } while (getpiece(maze, x, y) != EMPTY
          || abs(x - maze->xstart) < 4  /* at least 4 spaces from player */
          || abs(y - maze->ystart) < 4);

    /* initialize the monster actor */
    act_init(maze,      /* the maze */
             actor,     /* the actor */
             MONSTER,   /* monster piece */
             x,         /* starting x position */
             y,         /* starting y position */
             MRATE,     /* movement rate */
             mhandler); /* monster handler */
}

/* mhandler - monster handler */
static int mhandler(ACTOR *actor, int msg, void *arg)
{
    int sts = FALSE;
    switch (msg) {
    case MSG_INIT:
        sts = minit(actor);
        break;
    case MSG_MOVE:
        sts = mmove(actor);
        break;
    case MSG_ENCOUNTER:
        sts = mencounter(actor, (ACTOR *)arg);
        break;
    }
    return sts;
}

/* minit - initialize a monster */
static int minit(ACTOR *actor)
{
    act_schedule(actor, actor->maze->now + actor->rate);
    return TRUE;
}

/* mmove - handle a monster move */
static int mmove(ACTOR *actor)
{
    MAZE *maze = actor->maze;
    int dir, sts;

    /* compute the direction the monster will travel */
    dir = mdirection(actor);
    sts = TRUE;

    /* move the monster */
    act_move(actor, dir);

    /* see if we collided with a player */
    if (!checkencounter(actor))
        return FALSE;
    
    /* check to see what we landed on */
    switch (actor->mazePiece) {
    case ACTIVEBOMB:
        act_hide(actor);
        if (explosion(maze, actor->x, actor->y) == PLAYER)
            sts = FALSE;
        else {
            moverandom(actor);
            if ((actor->rate -= 5) < 1)
                actor->rate = 1;
            break;
        }
    }

    /* reschedule monster's move */
    act_schedule(actor, maze->now + actor->rate);
    return sts;
}

/* mdirection - compute the direction of the monster's move */
static int mdirection(ACTOR *actor)
{
    int dx, dy, ew, ns, nschk, ewchk, dir;
    MAZE *maze = actor->maze;

    /* compute the difference between the player and the monster */
    dx = maze->actors[0].x - actor->x;
    dy = maze->actors[0].y - actor->y;

    /* compute the n/s and e/w motions */
    ns = (dy < 0 ? N : (dy > 0 ? S : NONE));
    ew = (dx < 0 ? W : (dx > 0 ? E : NONE));

    /* check for a diagonal move */
    if (ns != NONE && ew != NONE) {
        nschk = iswall(getpiece(maze, actor->x, actor->y + yoff1[ns]));
        ewchk = iswall(getpiece(maze, actor->x + xoff1[ew], actor->y));
        if (nschk != ewchk)
            dir = (ewchk ? ns : ew);
        else
            dir = (irand(2) ? ns : ew);
    }

    /* check for a north or south move */
    else if (ew == NONE)
        dir = ns;

    /* must be an east or west move */
    else
        dir = ew;

    /* return the selected direction */
    return dir;
}

/* mencounter - encounter another actor */
static int mencounter(ACTOR *actor, ACTOR *other)
{
    return IS_PLAYER(other->piece) ? mbumpplayer(actor, other)
                                   : mbumpmonster(actor, other);
}

/* mbumpplayer - monster bumps player */
static int mbumpplayer(ACTOR *actor, ACTOR *other)
{
    return pbumpmonster(other, actor);
}

/* mbumpmonster - monster bumps monster */
static int mbumpmonster(ACTOR *actor, ACTOR *other)
{
    moverandom(actor);
    act_show(other);
    return TRUE;
}

/* act_init - initialize an actor */
static void act_init(MAZE *maze, ACTOR *actor, int piece, int x, int y, int rate, AHANDLER hndlr)
{
    actor->maze = maze;
    actor->visible = FALSE;
    actor->piece = piece;
    actor->x = x;
    actor->y = y;
    actor->rate = rate;
    actor->nbombs = 0;
    actor->nclubs = 0;
    actor->handler = hndlr;
    act_show(actor);
    (*hndlr)(actor, MSG_INIT, NULL);
}

/* act_schedule - schedule an actor for movement */
static void act_schedule(ACTOR *actor, unsigned int time)
{
    ACTOR **pp,*p;
    for (pp = &actor->maze->queue; (p = *pp) != NULL; pp = &p->next)
        if (time < p->time)
            break;
    actor->time = time;
    actor->next = p;
    *pp = actor;
}

/* act_show - show the actor piece */
static void act_show(ACTOR *actor)
{
    if (!actor->visible) {
        actor->mazePiece = actor->newPiece = getpiece(actor->maze, actor->x, actor->y);
        putpiece(actor->maze, actor->x, actor->y, actor->piece);
        actor->visible = TRUE;
    }
}

/* act_hide - hide the actor piece */
static void act_hide(ACTOR *actor)
{
    if (actor->visible) {
        putpiece(actor->maze, actor->x, actor->y, actor->newPiece);
        actor->visible = FALSE;
    }
}

/* act_move - move an actor in a specified direction */
static void act_move(ACTOR *actor, int dir)
{
    act_hide(actor);
    actor->x += xoff1[dir];
    actor->y += yoff1[dir];
    act_show(actor);
}

/* bumpwall - bump into a hidden wall piece */
static void bumpwall(ACTOR *actor, int dir, int piece)
{
    MAZE *maze = actor->maze;
    int x, y;

    /* compute the coordinates of the center wall piece */
    x = actor->x + xoff1[dir];
    y = actor->y + yoff1[dir];

    /* adjust the coordinates to print three wall pieces */
    switch (dir) {
    case N:
    case S:
        putpiece(maze, x - 1, y, WALL);
        putpiece(maze, x, y, piece);
        putpiece(maze, x + 1, y, WALL);
        break;
    case E:
    case W:
        putpiece(maze, x, y - 1, WALL);
        putpiece(maze, x, y, piece);
        putpiece(maze, x, y + 1, WALL);
        break;
    }       
}

/* checkencounter - check for an encounter with another actor */
static int checkencounter(ACTOR *actor)
{
    MAZE *maze = actor->maze;
    ACTOR *other;

    /* can only encounter another actor */
    if (!IS_ACTOR(actor->mazePiece))
        return TRUE;

    /* get the actor we encountered */
    other = &maze->actors[actor->mazePiece];

    /* hide the actors */
    act_hide(actor);
    act_hide(other);

    /* handle the encounter */
    return (*actor->handler)(actor, MSG_ENCOUNTER, other);
}
    
/* explosion - a bomb explodes */
static int explosion(MAZE *maze, int xcenter, int ycenter)
{
    int xradius, yradius, nlimit, slimit, elimit, wlimit, x, y;
    int pblasted, mblasted;

    /* assume that no one gets blasted (impossible of course) */
    pblasted = mblasted = FALSE;

    /* compute the horizontal and vertical radii */
    xradius = irand(2) + 1;
    yradius = irand(2) + 1;

    /* compute the east and west limits */
    if ((wlimit = xcenter - xradius) < 1)
        wlimit = 1;
    if ((elimit = xcenter + xradius) > maze->xsize - 1)
        elimit = maze->xsize - 1;

    /* compute the north and south limits */
    if ((nlimit = ycenter - yradius) < 1)
        nlimit = 1;
    if ((slimit = ycenter + yradius) > maze->ysize - 1)
        slimit = maze->ysize - 1;

    /* do the explosion */
    for (y = nlimit; y < slimit; ++y) {
        for (x = wlimit; x < elimit; ++x) {
            int piece = getpiece(maze, x, y);
            putpiece(maze, x, y, DEBRIS);
            switch(piece) {
            case PLAYER:
                pblasted = TRUE;
                break;
            case MONSTER:
                mblasted = TRUE;
                break;
            case GOAL:
                putpiece(maze, x, y, GOAL);
                break;
            case HIDDENRANDOMIZER:
                --maze->nrandomizers;
                break;
            case CLUB:
                --maze->nclubs;
                break;
            case BOMB:
                --maze->nbombs;
                break;
            }
        }
    }

    ShowMessage(maze, "Kabooom ! !");
    flash();

    /* figure out who got blasted */
    if (pblasted)               /* the player did */
        return PLAYER;
    else if (mblasted)          /* the monster did */
        return MONSTER;
    else                        /* ??? who set off the bomb anyway ??? */
        return EMPTY;
}

/* iswall - check for a wall piece */
static int iswall(int piece)
{
    return piece == WALL || piece == HIDDENWALL || piece == RANDOMIZER || piece == HIDDENRANDOMIZER;
}

/* moverandom - generate a random move */
static void moverandom(ACTOR *actor)
{
    MAZE *maze = actor->maze;
    int x, y;
    do {
        randomloc(maze, &x, &y);
    } while ((x == actor->x && y == actor->y) || getpiece(maze, x, y) == GOAL);
    act_hide(actor);
    actor->x = x;
    actor->y = y;
    act_show(actor);
}
