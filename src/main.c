/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <sys/printk.h>
#include "lcd_i2c.h"
#include "servo.h"
#include "stepper.h"
#include "lcd_manager.h"
#include "buttons.h"
#include <logging/log.h>

#include "cut_struct.h"

#define MAJOR_V  0
#define MINOR_V  1

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define STEPPER_DELAY_US    600//1200

#define CUTTING_DELAY_MS    2000

#define SERVO_1_OPEN_ANGLE  0
#define SERVO_1_CLOSED_ANGLE 180

#define SERVO_2_OPEN_ANGLE  180
#define SERVO_2_CLOSED_ANGLE 0

#if DT_NODE_HAS_STATUS(DT_NODELABEL(keypad),okay)
#define KEYPAD_NAME DT_LABEL(DT_NODELABEL(keypad))
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(servo1),okay)
#define SERVO_1_NAME DT_LABEL(DT_NODELABEL(servo1))
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(servo2),okay)
#define SERVO_2_NAME DT_LABEL(DT_NODELABEL(servo2))
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(exstepper),okay)
#define STEPPERS_NAME DT_LABEL(DT_NODELABEL(exstepper))
#endif





enum device_state_t {
    START_UP,
    MAIN_MENU,
    ENTER_LOAD,
    ENTER_LEN,
    ENTER_CUTS,
    SLICING,
};







const static struct device *servo1;
const static struct device *servo2;

const static struct device *stepper;
const static struct device *keypad;


#define EXTRUDE_STACK_SIZE 1024
#define EXTRUDE_PRI  1


#define LCD_UPDATES_STACK_SIZE 1024
#define LCD_UPDATES_STACK_PRI  10


K_SEM_DEFINE(halt_cutting, 0, 1);
K_SEM_DEFINE(estop_cutting,0, 1);
K_SEM_DEFINE(slicing_done,0, 1);

struct extrude_info {
  struct k_work work;
  uint32_t      steps;
  uint16_t      n;
};

struct extrude_info extrude_update;

struct extrude_lcd_info {
  struct k_work work;
  uint16_t      on;
};



struct k_work_q       extrude_work_q;
K_THREAD_STACK_DEFINE(extrude_stack,LCD_UPDATES_STACK_SIZE);


struct k_work_q       lcd_work_q;
K_THREAD_STACK_DEFINE(lcd_stack,LCD_UPDATES_STACK_SIZE);

struct extrude_lcd_info lcd_updates;
struct k_work update_into_cut_work;
struct k_work push_pound_work;


//Hacked Out Extrudding function
void extrude_function(struct k_work *item)
{
  struct extrude_info *data   = CONTAINER_OF(item,struct extrude_info,work);
  uint16_t total_cuts = data->n,i;
  uint32_t steps = data->steps;
  uint32_t ii;
  p1_servo_write(servo1,SERVO_1_OPEN_ANGLE);
  p1_servo_write(servo2,SERVO_2_OPEN_ANGLE);
  k_sleep(K_MSEC(CUTTING_DELAY_MS));
  p1_stepper_enable(stepper);
  p1_stepper_set_dir(stepper,0,0);
  p1_stepper_set_dir(stepper,1,0);
  for(i=0;i<total_cuts;i++)
  {
    lcd_updates.on = i;
    k_work_submit_to_queue(&lcd_work_q,&lcd_updates.work);
    p1_stepper_enable(stepper);
    for(ii = 0;ii<steps;ii++)
    {
      if(k_sem_take(&estop_cutting,K_USEC(STEPPER_DELAY_US)) == 0)
      {
        p1_stepper_disable(stepper);
        LOG_WRN("ESTOP HIT");
        return;
      }
      p1_stepper_step(stepper,0);
      p1_stepper_step(stepper,1);
    }
    p1_stepper_disable(stepper);
    k_work_submit_to_queue(&lcd_work_q,&update_into_cut_work);
    if(k_sem_take(&estop_cutting,K_MSEC(CUTTING_DELAY_MS)) == 0)
    {
      p1_stepper_disable(stepper);
      LOG_WRN("ESTOP HIT");
      return;
    }
    p1_servo_write(servo1,SERVO_1_CLOSED_ANGLE);
    p1_servo_write(servo2,SERVO_2_CLOSED_ANGLE);
    if(k_sem_take(&estop_cutting,K_MSEC(CUTTING_DELAY_MS)) == 0)
    {
      p1_stepper_disable(stepper);
      LOG_WRN("ESTOP HIT");
      return;
    }
    p1_servo_write(servo1,SERVO_1_OPEN_ANGLE);
    p1_servo_write(servo2,SERVO_2_OPEN_ANGLE);
    if(k_sem_take(&estop_cutting,K_MSEC(CUTTING_DELAY_MS)) == 0)
    {
      p1_stepper_disable(stepper);
      LOG_WRN("ESTOP HIT");
      return;
    }
    if(k_sem_take(&halt_cutting,K_NO_WAIT) == 0)
    {
      break;
    }
  }
  k_sem_give(&slicing_done);
}




void new_extrude_function(struct k_work *item)
{
  struct extrude_lcd_info *data   = CONTAINER_OF(item,struct extrude_lcd_info,work);
  LCD_draw_extrudding(data->on);
}

void update_into_cut_fn(struct k_work *item)
{
  LCD_draw_cutting();
}

void push_pound_fn(struct k_work *item)
{
  
}


struct key_pad_item {
    void *fifo_reserved;   /* 1st word reserved for use by FIFO */
    char key_pressed;
};

struct key_pad_item keypad_data;

K_FIFO_DEFINE(keypad_fifo);
const char keypad_look_up[16] = {'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};
void keypad_in_cb(uint16_t in)
{

  if(in<16)
  {
    keypad_data.key_pressed = keypad_look_up[in];
    k_fifo_put(&keypad_fifo, &keypad_data);
    LOG_DBG("Key Pressed %c",keypad_look_up[in]);
  }
}


static const struct button_cb keypad_cd = {
    .button_pressed = keypad_in_cb,
};

struct cut_settings default_settings = 
{
  .save_slot = 0,
  .cuts      = 5,
  .len_mm    = 1000,
};

struct cut_settings *selected = &default_settings;
struct key_pad_item  *data_in;

void handle_slicing(void)
{

    LCD_start_cutting(extrude_update.n);
    k_work_submit_to_queue(&extrude_work_q,&extrude_update.work);
    while(k_sem_take(&slicing_done,K_MSEC(10)) != 0)
    {
      data_in = k_fifo_get(&keypad_fifo,K_MSEC(10));
      if(data_in != NULL)
      {
        switch (data_in->key_pressed)
        {
        case '*':
          k_sem_give(&halt_cutting);
          break;
        case '#':
          k_sem_give(&estop_cutting);
          break;
        default:
          LOG_INF("Unhanded Button %c",data_in->key_pressed);
          break;
        }
      }
    }
}




void main(void)
{
	LOG_INF("\n\n\n\nFISH LINE SLICER V%d.%d Built on "__DATE__" at "__TIME__,MAJOR_V,MINOR_V);

  LCD_init();
  LCD_Draw_Splash_Screen(MAJOR_V,MINOR_V);





//   struct k_work update_into_cut_work;
// struct k_work push_pound_work;
//struct extrude_lcd_info lcd_updates;

  #ifdef SERVO_1_NAME
  servo1 = device_get_binding(SERVO_1_NAME);
  p1_servo_start(servo1);
  p1_servo_write(servo1,SERVO_1_OPEN_ANGLE);
  #endif

  #ifdef SERVO_2_NAME
  servo2 = device_get_binding(SERVO_2_NAME);
  p1_servo_start(servo2);
  p1_servo_write(servo2,SERVO_2_OPEN_ANGLE);
  #endif

  #ifdef STEPPERS_NAME
  stepper = device_get_binding(STEPPERS_NAME);
  p1_stepper_disable(stepper);
  #endif

  k_work_queue_start(&extrude_work_q,extrude_stack,LCD_UPDATES_STACK_SIZE,EXTRUDE_PRI,NULL);
  k_work_init(&extrude_update.work,extrude_function);

  k_work_queue_start(&lcd_work_q,lcd_stack,LCD_UPDATES_STACK_SIZE,LCD_UPDATES_STACK_PRI,NULL);
  k_work_init(&lcd_updates.work,new_extrude_function);
  k_work_init(&push_pound_work,push_pound_fn);
  k_work_init(&update_into_cut_work,update_into_cut_fn);


  k_sleep(K_SECONDS(5));



  #ifdef KEYPAD_NAME
  keypad = device_get_binding(KEYPAD_NAME);
  buttons_register_cb(keypad,&keypad_cd);
  #endif

  

  enum device_state_t state = MAIN_MENU;


	while(1)
	{
    LCD_draw_main(selected);
    data_in = k_fifo_get(&keypad_fifo,K_FOREVER);
    switch(data_in->key_pressed)
    {
      case 'A':

      break;
      case 'B':

      break;
      case 'C':

      break;
      case 'D':

      break;
      case '*':
        extrude_update.n     = 1;
        extrude_update.steps = (selected->len_mm)*8;
        handle_slicing();
        break;
      case '#':
        extrude_update.n     = (selected->cuts);
        extrude_update.steps = (selected->len_mm)*8;
        handle_slicing();
        break;
      default:
        LOG_INF("Unhandled Menu Option %c",data_in->key_pressed);
      break;
    }
    

	}
}
