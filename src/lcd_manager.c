





#include <zephyr.h>
#include <device.h>
#include <string.h>
#include <sys/printk.h>
#include "lcd_i2c.h"
#include "lcd_manager.h"
#include <logging/log.h>

#include "cut_struct.h"


LOG_MODULE_REGISTER(lcd_manager, LOG_LEVEL_DBG);



#define OPTION_STR_LEN  5

#define TRUNK_A         1
#define TRUNK_B         2
#define TRUNK_C         3
#define TRUNK_D         4



#if DT_NODE_HAS_STATUS(DT_NODELABEL(lcdthing),okay)
#define LCD_NAME DT_LABEL(DT_NODELABEL(lcdthing))
#endif

const static struct device *lcd_dev;


const uint8_t trunk_a[8]       = {0x0C, 0x12, 0x12, 0x12, 0x1E, 0x12, 0x12, 0x00};
const uint8_t trunk_b[8]       = {0x1C, 0x12, 0x12, 0x1C, 0x12, 0x12, 0x1C, 0x00};
const uint8_t trunk_c[8]       = {0x0C, 0x12, 0x10, 0x10, 0x10, 0x12, 0x0C, 0x00};
const uint8_t trunk_d[8]       = {0x18, 0x14, 0x12, 0x12, 0x12, 0x14, 0x18, 0x00};

// const uint8_t wave_part[8] = {  0x00, 0x02, 0x03, 0x05, 0x19, 0x00, 0x00, 0x00};
// const uint8_t fish_back[8] = {0x10, 0x19, 0x1E, 0x18, 0x10, 0x18, 0x1E, 0x19};
// const uint8_t fish_front[8] = {0x00, 0x1C, 0x02, 0x09, 0x01, 0x0F, 0x02, 0x1C};



void LCD_init(void)
{
    #ifdef LCD_NAME
	lcd_dev = device_get_binding(LCD_NAME);
    lcdi2c_init(lcd_dev);
    lcdi2c_createChar(lcd_dev,TRUNK_A,trunk_a);
    lcdi2c_createChar(lcd_dev,TRUNK_B,trunk_b);
    lcdi2c_createChar(lcd_dev,TRUNK_C,trunk_c);
    lcdi2c_createChar(lcd_dev,TRUNK_D,trunk_d);

	#endif
}

static void draw_menu_items(char *A_option,char *B_option,char *C_option,char *D_option,char *Star_option,char *Pound_option)
{
    char temp[OPTION_STR_LEN+2];
    temp[6]='\0';
    uint8_t str_len,i,j;
    const char *op_pointers[6] = {A_option,B_option,C_option,D_option,Star_option,Pound_option};
    const char option_char[6] = {TRUNK_A,TRUNK_B,TRUNK_C,TRUNK_D,'*','#'};
    for(j=0;j<6;j++)
    {
        if(op_pointers[j] != NULL)
        {
            temp[0] = option_char[j];
            str_len = strlen(op_pointers[j]);
            if(str_len > OPTION_STR_LEN)
            {
                LOG_WRN("option too long len %d",str_len);
            }
            for(i=0;i<OPTION_STR_LEN;i++)
            {
                if(i<str_len)
                {
                    temp[(OPTION_STR_LEN+1-1)-i] = op_pointers[j][str_len-1-i];
                }
                else
                {
                    temp[(OPTION_STR_LEN+1-1)-i] = ' ';
                }
            }
        }
        else
        {
            for(i=0;i<OPTION_STR_LEN+1;i++)
            {
                temp[i] = ' ';
            }
        }
        switch (j)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            lcdi2c_setCursor(lcd_dev,j, 20-OPTION_STR_LEN-1);
            break;
        case 4:
            lcdi2c_setCursor(lcd_dev,3, 0);
            break;
        case 5:
            lcdi2c_setCursor(lcd_dev,3, 7);
            break;
        default:
            break;
        }
        lcdi2c_printstr(lcd_dev,temp);
    }
}
static void print_len(uint32_t len)
{
    if(len <= 1000)
    {
        lcdi2c_printint(lcd_dev,len);
        lcdi2c_printstr(lcd_dev,"mm");
    }
    else if(len <= 10000)
    {
        lcdi2c_printint(lcd_dev,(len/10));
        lcdi2c_printstr(lcd_dev,"cm");
    }
    else
    {
        lcdi2c_printint(lcd_dev,(len/1000));
        lcdi2c_printstr(lcd_dev,"m");
    }
}
void enter_entry()
{
    lcdi2c_blink_on(lcd_dev);
}

void exit_entry()
{
    lcdi2c_blink_off(lcd_dev);
}

void LCD_push_digit(uint8_t digit)
{
    if(digit < 10)
    {
        char temp = {(digit | 0x30),'\0'};
        lcdi2c_printstr(lcd_dev,temp);
    }
}


void LCD_len_enter(void)
{
    draw_menu_items("mm","cm","m",NULL,NULL,"Nope!");
    lcdi2c_setCursor(lcd_dev,1,7);
    lcdi2c_printstr(lcd_dev,"      ");
    lcdi2c_setCursor(lcd_dev,1,7);
    enter_entry();
}

void LCD_cuts_enter(void)
{
    draw_menu_items("x1","x10","x100",NULL,NULL,"Nope!");
    lcdi2c_setCursor(lcd_dev,2,7);
    lcdi2c_printstr(lcd_dev,"      ");
    lcdi2c_setCursor(lcd_dev,2,7);
    enter_entry();
}

void LCD_load_enter(void)
{
    draw_menu_items(NULL,NULL,NULL,NULL,"Load","Nope!");
    lcdi2c_setCursor(lcd_dev,0,10);
    lcdi2c_printstr(lcd_dev,"   ");
    lcdi2c_setCursor(lcd_dev,0,10);
    enter_entry();
}
uint8_t current_number_loc;

void LCD_start_cutting(uint16_t total_n)
{
    lcdi2c_clear(lcd_dev);
    draw_menu_items(NULL,NULL,NULL,NULL,"Halt","Estop");
    lcdi2c_setCursor(lcd_dev,0,5);
    lcdi2c_printstr(lcd_dev,"Extruding");
    if(total_n >= 10000)
    {
        current_number_loc = 3;
    }
    else if(total_n >= 1000)
    {
        current_number_loc = 4;
    }
    else if(total_n >= 100)
    {
        current_number_loc = 5;
    }
    else if(total_n >= 10)
    {
        current_number_loc = 6;
    }
    else
    {
        current_number_loc = 7;
    }
    lcdi2c_setCursor(lcd_dev,1,current_number_loc);
    lcdi2c_printint(lcd_dev,1);

    lcdi2c_setCursor(lcd_dev,1,9);
    lcdi2c_printstr(lcd_dev,"of");

    lcdi2c_setCursor(lcd_dev,1,12);
    lcdi2c_printint(lcd_dev,total_n);

    lcdi2c_setCursor(lcd_dev,1,5);
}

void LCD_draw_cutting(void)
{
    lcdi2c_setCursor(lcd_dev,0,5);
    lcdi2c_printstr(lcd_dev," Cutting ");
}

void LCD_draw_extrudding(uint16_t unit_on)
{
    lcdi2c_setCursor(lcd_dev,0,5);
    lcdi2c_printstr(lcd_dev,"Extruding");
    lcdi2c_setCursor(lcd_dev,1,current_number_loc);
    lcdi2c_printint(lcd_dev,unit_on+1);
    lcdi2c_setCursor(lcd_dev,1,5);
}



void LCD_draw_main(struct cut_settings *settings)
{
    exit_entry();
    lcdi2c_clear(lcd_dev);
    draw_menu_items("Load","Len","Cuts",NULL,"Cut 1","Cut n");
    lcdi2c_setCursor(lcd_dev,0,0);
    lcdi2c_printstr(lcd_dev,"Save Slot ");
    lcdi2c_printint(lcd_dev,settings->save_slot);

    lcdi2c_setCursor(lcd_dev,1,0);
    lcdi2c_printstr(lcd_dev,"Lenght ");
    print_len(settings->len_mm);

    lcdi2c_setCursor(lcd_dev,2,0);
    lcdi2c_printstr(lcd_dev,"Amount ");
    lcdi2c_printint(lcd_dev,settings->cuts);
    //lcdi2c_setCursor(lcd_dev,3, 7);
}


void LCD_Draw_Splash_Screen(uint16_t maj_v,uint16_t min_v)
{
  char temp[] = "i";
  lcdi2c_clear(lcd_dev);

  lcdi2c_setCursor(lcd_dev,0, 0);
  lcdi2c_printstr(lcd_dev,"The Fish Line");

  lcdi2c_setCursor(lcd_dev,1, 0);
  lcdi2c_printstr(lcd_dev,"Slicer 2000!!");

  lcdi2c_setCursor(lcd_dev,2, 0);
  lcdi2c_printstr(lcd_dev,"Blehm & DeVries");

  lcdi2c_setCursor(lcd_dev,3, 0);
  lcdi2c_printstr(lcd_dev,"V");
  lcdi2c_printint(lcd_dev,maj_v);
  lcdi2c_printstr(lcd_dev,".");
  lcdi2c_printint(lcd_dev,min_v);

  lcdi2c_printstr(lcd_dev," @");
  lcdi2c_printstr(lcd_dev,__TIME__);
}

