




#define DT_DRV_COMPAT phase1_hd4478ui2c

#include <zephyr.h>
#include <kernel.h>
#include <device.h>
#include <drivers/i2c.h>
#include <pm/device.h>
#include "../include/lcd_i2c.h"
#include <logging/log.h>


#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define Rs 0b00000001  // Register select bit


LOG_MODULE_REGISTER(lcd_screen, LOG_LEVEL_DBG);

static struct hd44780uic_data  
{
    uint8_t _Addr;
    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;
    uint8_t _numlines;
    uint8_t _cols;
    uint8_t _rows;
    uint8_t _backlightval;
    const struct device *I2C_BUS;
};

static void expanderWrite(const struct device *dev,uint8_t _data)
{
    struct hd44780uic_data *data = dev->data;
    uint8_t out = (_data | data->_backlightval);
    struct i2c_msg msgs[1];

    msgs[0].buf = &out;
	msgs[0].len = 1U;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

    i2c_transfer(data->I2C_BUS,&msgs[0],1,data->_Addr);
}

static void pulseEnable(const struct device *dev,uint8_t _data)
{
    expanderWrite(dev,_data | En);	// En high
	k_sleep(K_USEC(2));		// enable pulse must be >450ns
	
	expanderWrite(dev,_data & ~En);	// En low
	k_sleep(K_USEC(50));		// commands need > 37us to settle 
}

static void write4bits(const struct device *dev,uint8_t value)
{
    expanderWrite(dev,value);
	pulseEnable(dev,value);
}

static void send(const struct device *dev,uint8_t value,uint8_t mode)
{
    uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
    write4bits(dev,(highnib)|mode);
	write4bits(dev,(lownib)|mode); 
}

static void command(const struct device *dev,uint8_t value)
{
    send(dev, value, 0);
}

static void data(const struct device *dev,uint8_t value)
{
    send(dev, value, Rs);
}



static int hd44780uic_clear(const struct device *dev){
    command(dev,LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	k_sleep(K_USEC(2000));  // this command takes a long time!
     return 0;
     }

static int hd44780uic_home(const struct device *dev){
    command(dev,LCD_RETURNHOME);// clear display, set cursor position to zero
	k_sleep(K_USEC(2000));  // this command takes a long time!
    return 0;
}

static int hd44780uic_noDisplay(const struct device *dev){ 
    struct hd44780uic_data *data = dev->data;
    data->_displaycontrol &= ~LCD_DISPLAYON;
	command(dev,LCD_DISPLAYCONTROL | data->_displaycontrol);
    return 0;
    }

static int hd44780uic_display(const struct device *dev){
    struct hd44780uic_data *data = dev->data;
    data->_displaycontrol |= LCD_DISPLAYON;
	command(dev,LCD_DISPLAYCONTROL | data->_displaycontrol);
    return 0;
    }

static int hd44780uic_blink_off(const struct device *dev){
    struct hd44780uic_data *data = dev->data;
    data->_displaycontrol &= ~LCD_BLINKON;
	command(dev,LCD_DISPLAYCONTROL | data->_displaycontrol); 
    return 0;
    }

static int hd44780uic_blink_on(const struct device *dev){
    struct hd44780uic_data *data = dev->data;
    data->_displaycontrol |= LCD_BLINKON;
	command(dev,LCD_DISPLAYCONTROL | data->_displaycontrol); 
    return 0;
    }

static int hd44780uic_cursor_off(const struct device *dev){
        struct hd44780uic_data *data = dev->data; 
    	data->_displaycontrol &= ~LCD_CURSORON;
	    command(dev,LCD_DISPLAYCONTROL | data->_displaycontrol);
    return 0;
    }
static int hd44780uic_cursor_on(const struct device *dev){
        struct hd44780uic_data *data = dev->data;
    	data->_displaycontrol |= LCD_CURSORON;
	    command(dev,LCD_DISPLAYCONTROL | data->_displaycontrol);
     return 0;
     }

static int hd44780uic_createChar(const struct device *dev,uint8_t inst, uint8_t *array){

    	inst &= 0x7; // we only have 8 locations 0-7
        command(dev,LCD_SETCGRAMADDR | (inst << 3));
        for (int i=0; i<8; i++) {
            data(dev,array[i]);//write(array[i]);
        }
     return 0;
     }

static int hd44780uic_setCursor(const struct device *dev,uint8_t row, uint8_t column){

    struct hd44780uic_data *data = dev->data;
    	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > (data->_rows-1) ) {
		row = data->_rows-1;    // we count rows starting w/0
	}
	command(dev,LCD_SETDDRAMADDR | (column + row_offsets[row]));
     return 0;
     }

static int hd44780uic_printstr(const struct device *dev,uint8_t *str){
     for(uint16_t i = 0;(str[i]!='\0')&&(i<20);i++)
     {
         data(dev,str[i]);
     }
     return 0;
     }

static int hd44780uic_printint(const struct device *dev,uint16_t num){
    char str[7];
    str[6] = '\0';
    uint8_t i;
    if(num == 0)
    {
        str[5] = '0';
        i = 4;
    }
    else
    {
        for(i = (7-2);(num);i--)  
        {
            str[i] = (num % 10) | 0x30;//convert number into dec repersentation no fancy tricks here
            num /= 10;//devide by ten
        }
    }
    hd44780uic_printstr(dev,&str[i+1]);  

    return 0;
}

static int hd44780uic_init(const struct device *dev)
{
    struct hd44780uic_data *data = dev->data;
    data->_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;



	data->_displayfunction |= LCD_2LINE;


	// for some 1 line displays you can select a 10 pixel high font

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	k_sleep(K_MSEC(50)); 
  
	// Now we pull both RS and R/W low to begin commands
	expanderWrite(dev,data->_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	k_sleep(K_MSEC(1000));

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46
	
	  // we start in 8bit mode, try to set 4 bit mode
   write4bits(dev,0x03 << 4);
   k_sleep(K_USEC(4500)); // wait min 4.1ms
   
   // second try
   write4bits(dev,0x03 << 4);
   k_sleep(K_USEC(4500)); // wait min 4.1ms
   
   // third go!
   write4bits(dev,0x03 << 4); 
   k_sleep(K_USEC(150));
   
   // finally, set to 4-bit interface
   write4bits(dev,0x02 << 4); 


	// set # lines, font size, etc.
	command(dev,LCD_FUNCTIONSET | data->_displayfunction);  
	
	// turn the display on with no cursor or blinking default
	data->_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;

	
	hd44780uic_display(dev);
	
	// clear it off
	hd44780uic_clear(dev);
	
	// Initialize to default text direction (for roman languages)
	data->_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(dev,LCD_ENTRYMODESET | data->_displaymode);
	
	hd44780uic_home(dev);
}

static const struct lcdi2c_api hd44780uic_api = {
    .init = hd44780uic_init,
    .clear = hd44780uic_clear,
    .home = hd44780uic_home,
    .noDisplay = hd44780uic_noDisplay,
    .display = hd44780uic_display,
    .blink_off = hd44780uic_blink_off,
    .blink_on = hd44780uic_blink_on,
    .cursor_off = hd44780uic_cursor_off,
    .cursor_on = hd44780uic_cursor_on,
    .createChar =  hd44780uic_createChar,
    .setCursor = hd44780uic_setCursor,
    .printstr = hd44780uic_printstr,
    .printint = hd44780uic_printint,
};

static int ok_i_get_it(const struct device *dev)
{
    LOG_INF("LCD_STARTED",0x20);
    return 0;
}


#define HD44780UIC_INIT(i)                \
static struct hd44780uic_data hd44780uic_data_##i  = {\
      ._Addr = DT_INST_PROP(i,reg),                   \
      ._cols = DT_INST_PROP(i,lcd_cols),              \
      ._rows = DT_INST_PROP(i,lcd_rows),              \
      ._backlightval = LCD_BACKLIGHT,               \
      .I2C_BUS =DEVICE_DT_GET(DT_INST_BUS(i)),        \
};                                                     \
DEVICE_DT_INST_DEFINE(i,&ok_i_get_it,NULL,&hd44780uic_data_##i,NULL,POST_KERNEL,CONFIG_APPLICATION_INIT_PRIORITY,&hd44780uic_api);



DT_INST_FOREACH_STATUS_OKAY(HD44780UIC_INIT)
//HD44780UIC_INIT(0);