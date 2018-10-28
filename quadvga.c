// long[par][0]:   screen:   [!Z]:addr =   16:16 -> zero (accepted), 2n
// long[par][1]: pal/user: u:[!Z]:addr = 1:15:16 -> zero (accepted), 2n/4n
// long[par][2]:   colour:   [!Z]:addr =   16:16 -> zero (accepted), 2n
// long[par][3]: frame indicator, lock target and vcfg on startup (16:7:9)

#include <stdint.h>
#include <string.h>
#include <propeller.h>
#include "wordfire.h"

#define ROWS_RAW    ((RES_Y + 32 - 1) / 32)
#define BCNT_RAW    (COLUMNS * ROWS_RAW)

#define XSTART      1
#define XEND        (COLUMNS - 2)
#define YSTART      1
#define YEND        (ROWS - 2)

#define BW          0x37

typedef struct {
    int32_t screen;
    uint32_t palUser;
    uint32_t colour;
    uint32_t config;
} vgamailbox;

typedef struct {
    volatile uint8_t scrn[BCNT_RAW];    // screen buffer (2n aligned)
    volatile uint8_t indx[BCNT_RAW];    // colour buffer (2n aligned)
    vgamailbox mbox;
    uint32_t cog;
    int x;
    int y;
} screen;

static screen screen0;
static screen screen1;
static screen screen2;
static screen screen3;

static screen *screens[] = {
    &screen0,
    &screen1,
    &screen2,
    &screen3
};

static uint32_t vconfig[] = {
    (0 << 9) | 0x4f,    // with sync %010011_11  
    (1 << 9) | 0x4c,    //   no sync %010011_00      
    (2 << 9) | 0x4c,   
    (3 << 9) | 0x4c
};
        
// quadvgaStart - start the quad VGA drivers
int quadvgaStart(void)
{
    extern uint32_t binary_quadvga_dat_start[];
    uint32_t lock;
    int i;
    
    lock = (((CNT >> 16) + 2) | 1) << 16;
    for (i = 0; i < 4; ++i) {
        screen *s = screens[i];
        quadvgaClear(i);
        s->mbox.screen = (uint32_t)s->scrn;
        s->mbox.palUser = 0;
        s->mbox.colour = (uint32_t)s->indx;
        s->mbox.config = lock | vconfig[i];
        if ((s->cog = cognew(binary_quadvga_dat_start, &s->mbox)) < 0)
            return -1;
        s->x = XSTART;
        s->y = YSTART;
    }
    
    return 0;
}

// quadvgaClear - clear one of the screens
void quadvgaClear(int i)
{
    screen *s = screens[i];
    memset((void *)s->indx, BW, BCNT_RAW);
    memset((void *)s->scrn, ' ', BCNT_RAW);
    s->x = XSTART;
    s->y = YSTART;
}
      
// quadvgaTX - write a character to a screen
void quadvgaTX(int i, int ch)
{
    screen *s = screens[i];
    if (ch == 0x08) {
        if (s->x > XSTART)
            --s->x;
    }
    else if (ch == 0x0a) {
        quadvgaCRLF(i);
    }
    else {
        int j = s->y * COLUMNS + s->x;
        s->indx[j] = BW;
        s->scrn[j] = ch;
        if (++s->x > XEND) {                                    
            s->x = XSTART;
            quadvgaCRLF(i);
        }
    }
}

// quadvgaPoke - poke a character directly into the buffer
void quadvgaPoke(int i, int x, int y, int ch)
{
    screen *s = screens[i];
    int j = (YSTART + y) * COLUMNS + XSTART + x;
    s->indx[j] = BW;
    s->scrn[j] = ch;
}

// quadvgaStr - write a string to a screen
void quadvgaStr(int i, const char *str)
{
    while (*str)
        quadvgaTX(i, *str++);
}

// quadvgaCRLF - go to the start of the next line
void quadvgaCRLF(int i)
{
    screen *s = screens[i];
    s->x = XSTART;
    if (++s->y > YEND)
        s->y = YEND;
    memcpy((void *)&s->indx[COLUMNS], (void *)&s->indx[COLUMNS * 2], BCNT_RAW - COLUMNS * 2);
    memcpy((void *)&s->scrn[COLUMNS], (void *)&s->scrn[COLUMNS * 2], BCNT_RAW - COLUMNS * 2);
    memset((void *)&s->indx[(ROWS - 1) * COLUMNS], BW, COLUMNS);
    memset((void *)&s->scrn[(ROWS - 1) * COLUMNS], ' ', COLUMNS);
}

// quadvgaSetXY - set the cursor position of a screen
void quadvgaSetXY(int i, int x, int y)
{
    screen *s = screens[i];
    s->x = XSTART + x;
    s->y = YSTART + y;
}
