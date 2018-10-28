#ifndef __WORDFIRE_H__
#define __WORDFIRE_H__

#define RES_X       640
#define RES_Y       480

#define COLUMNS     (RES_X / 16)
#define ROWS        (RES_Y / 32)

#define WIDTH       (COLUMNS - 2)
#define HEIGHT      (ROWS - 2)

int quadkeyboardStart(void);
int hasKey(int kbn);
void clearKeyBuffer(int kbn);
int getKey(int kbn);

int quadvgaStart(void);
void quadvgaClear(int i);
void quadvgaTX(int i, int ch);
void quadvgaPoke(int i, int x, int y, int ch);
void quadvgaStr(int i, const char *str);
void quadvgaCRLF(int i);
void quadvgaSetXY(int i, int x, int y);

#endif

