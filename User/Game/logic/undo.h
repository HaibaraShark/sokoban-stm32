#ifndef __UNDO_H
#define __UNDO_H

#include "../config.h"
#include "move.h"

void Undo_Init(void);
void Undo_Push(const MoveRecord *rec);
uint8_t Undo_Pop(MoveRecord *rec);
uint8_t Undo_Count(void);
void Undo_Clear(void);

#endif
