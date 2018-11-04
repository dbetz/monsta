#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define WIDTH   16
#define HEIGHT  32

typedef struct {
    uint8_t bits[HEIGHT][WIDTH];
} Glyph;

int parserow(char *line, uint8_t *row)
{
    int col;
    for (col = 0; col < WIDTH; ++col) {
        if (line[col] == 'X')
            row[col] = 1;
        else if (line[col] == ' ')
            row[col] = 0;
        else
            return -1;
    }
    return 0;
}

int parseglyph(char *line, Glyph *glyph)
{
    int sts, row;
    if ((sts = parserow(line, glyph->bits[0])) != 0)
        return sts;
    for (row = 1; row < HEIGHT; ++row) {
        if (!gets(line))
            return -1;
        if ((sts = parserow(line, glyph->bits[row])) != 0)
            return -1;
    }
    return 0;
}

void dumpglyph(Glyph *glyph)
{
    int row, col;
    for (row = 0; row < HEIGHT; ++row) {
        for (col = 0; col < WIDTH; ++col) {
            printf(" %d", glyph->bits[row][col]);
        }
        printf("\n");
    }
    printf("\n");
}

char *getnonblankline(char *line)
{
    while (gets(line)) {
        char *p = line;
        while (*p != '\0') {
            if (*p == ';')
                break;
            else if (!isspace(*p))
                return line;
            ++p;
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int row, col, sts;
    char line[100];
    Glyph glyph1;
    Glyph glyph2;
    
    printf("#include <stdint.h>\n");
    printf("uint32_t glyphs[] = {\n");
    while (getnonblankline(line)) {
        if ((sts = parseglyph(line, &glyph1)) != 0) {
            printf("error: parseglyph failed\n");
            return 1;
        }
        //dumpglyph(&glyph1);
        memset(&glyph2, 0, sizeof(glyph2));
        if (getnonblankline(line)) {
            if ((sts = parseglyph(line, &glyph2)) != 0) {
                printf("error: parseglyph failed\n");
                return 1;
            }
        }
        //dumpglyph(&glyph2);
        
        for (row = 0; row < HEIGHT; ++row) {
            uint32_t bits = 0;
            for (col = 0; col < WIDTH; ++col) {
                bits |= glyph1.bits[row][col] << col * 2;
                bits |= glyph2.bits[row][col] << (col * 2 + 1);
            }
            printf("0x%08x,\n", bits);
        }
    }
    printf("};\n");
    
    return 0;
}
