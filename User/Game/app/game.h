#ifndef __GAME_APP_H
#define __GAME_APP_H

#include "../config.h"

void Game_Enter(uint8_t level_id);
void Game_Update(InputEvent ev);
void Game_Draw(void);
uint8_t Game_IsRunning(void);
void LED_Update(void);

#endif
