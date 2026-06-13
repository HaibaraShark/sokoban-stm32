#include "undo.h"
#include <string.h>

static MoveRecord g_undo_stack[UNDO_DEPTH];
static uint8_t    g_undo_head;
static uint8_t    g_undo_count;

void Undo_Init(void)
{
    g_undo_head  = 0;
    g_undo_count = 0;
}

void Undo_Push(const MoveRecord *rec)
{
    g_undo_stack[g_undo_head] = *rec;
    g_undo_head = (g_undo_head + 1) % UNDO_DEPTH;
    if (g_undo_count < UNDO_DEPTH) {
        g_undo_count++;
    }
}

uint8_t Undo_Pop(MoveRecord *rec)
{
    if (g_undo_count == 0) return 0;
    g_undo_head = (g_undo_head + UNDO_DEPTH - 1) % UNDO_DEPTH;
    g_undo_count--;
    *rec = g_undo_stack[g_undo_head];
    return 1;
}

uint8_t Undo_Count(void)
{
    return g_undo_count;
}

void Undo_Clear(void)
{
    g_undo_head  = 0;
    g_undo_count = 0;
}
