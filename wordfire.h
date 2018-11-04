#ifndef __WORDFIRE_H__
#define __WORDFIRE_H__

#define RES_X       640
#define RES_Y       480

#define COLUMNS     (RES_X / 16)
#define ROWS        (RES_Y / 32)

#define WIDTH       (COLUMNS - 2)
#define HEIGHT      (ROWS - 2)

#define BLACK       0
#define RED         1
#define GREEN       2
#define BLUE        3
#define YELLOW      4
#define AQUA        5
#define PURPLE      6
#define WHITE       7

#define ColorPair(fg, bg)   (((bg) << 4) | (fg))

int quadkeyboardStart(void);
int hasKey(int kbn);
void clearKeyBuffer(int kbn);
int getKey(int kbn);

int quadvgaStart(void);
void quadvgaSetPalette(int i, uint32_t *palette);
void quadvgaSetUserGlyphs(int i, uint8_t *glyphs);
void quadvgaClear(int i);
void quadvgaTX(int i, int ch);
void quadvgaClearLine(int i, int y);
void quadvgaPoke(int i, int x, int y, int ch);
void quadvgaStr(int i, const char *str);
void quadvgaCRLF(int i);
void quadvgaSetXY(int i, int x, int y);

#endif

