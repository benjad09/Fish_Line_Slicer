
#ifndef LCD_MANAGER_H_
#define LCD_MANAGER_H_

#include <zephyr.h>
#include "cut_struct.h"



void LCD_init(void);

void LCD_Draw_Splash_Screen(uint16_t maj_v,uint16_t min_v);

void LCD_draw_main(struct cut_settings *settings);


void LCD_push_digit(uint8_t digit);

void LCD_len_enter(void);

void LCD_cuts_enter(void);

void LCD_load_enter(void);

void LCD_start_cutting(uint16_t total_n);

void LCD_draw_cutting(void);

void LCD_draw_extrudding(uint16_t unit_on);





#endif