#include <stdint.h>
#include <string.h>
#include <propeller.h>
#include "wordfire.h"

static volatile struct {
    uint8_t kb[32]; // four 8-byte buffers to buffer 7 keystrokes for 4 keyboards (one slot each is empty for management)
    uint8_t kh[4];  // head ptrs for 4 keyboards; keystrokes are input at the head
    uint8_t kt[4];  // tail ptrs for 4 keyboards; keystrokes are input at the head
} kbdata;

// quadkeyboardStart - start the quad keyboard driver
int quadkeyboardStart(void)
{
    extern uint8_t quadkeyboard_array[];
    memset((void *)&kbdata, 0, sizeof(kbdata));
    return cognew(quadkeyboard_array, &kbdata);
}

// hasKey - check to see if a keyboard buffer has a key available
int hasKey(int kbn) 
{
    if (kbn >= 4 || kbn < 0)
        return 0;
    return kbdata.kh[kbn] != kbdata.kt[kbn];
}

// clearKeyBuffer - clear the keyboard buffer for given keyboard
void clearKeyBuffer(int kbn)
{
    if (kbn >= 0 && kbn < 4)
        kbdata.kh[kbn] = kbdata.kt[kbn];
}

// getKey - get a key from a given keyboard buffer
int getKey(int kbn)
{
    int result = 0;
    if (kbn >= 0 && kbn < 4) {
        if (kbdata.kh[kbn] != kbdata.kt[kbn]) {
            result = kbdata.kb[kbn * 8 + kbdata.kh[kbn]];
            kbdata.kh[kbn] = (kbdata.kh[kbn] + 1) % 8;
        }
    }
    return result;
}
