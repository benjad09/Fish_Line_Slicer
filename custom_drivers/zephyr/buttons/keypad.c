#define DT_DRV_COMPAT phase1_keypad

#include <zephyr.h>
#include <kernel.h>
#include <device.h>
#include <drivers/gpio.h>
#include <pm/device.h>
#include "../include/buttons.h"
#include <logging/log.h>



LOG_MODULE_REGISTER(keypad, LOG_LEVEL_WRN);


struct keypad_data {
    uint16_t last_pressed;
    struct button_cb call_backs;
    struct k_work                  work;
    struct device                  *keypad_device;
    struct k_work_q                keypad_work_q;
    struct z_thread_stack_element  *keypad_work_stack;
};

struct keypad_config {
    uint16_t rows;
    uint16_t columns;
    struct gpio_dt_spec *row_gpio;
    struct gpio_dt_spec *column_gpio;
};

#define KEYPAD_STACK_SIZE 1024
#define KEYPAD_PRI        10



static int keypad_register_cb(const struct device *dev,struct button_cb *callbacks)
{
    struct keypad_data *data = dev->data;
    data->call_backs=*callbacks;
    return 0;
}

static const struct button_api keypad_api = {
    .register_cb = keypad_register_cb,
};

void keypad_work_fun(struct k_work *item)
{
    struct keypad_data    *data   = CONTAINER_OF(item,struct keypad_data,work);
    struct device              *dev    = data->keypad_device;
    struct keypad_config  *config = dev->config;
    uint16_t i,ii,key_in = 0xffff;
    while(1)
    {
        k_sleep(K_MSEC(20));
        key_in = 0xffff;
        for(i=0;i<config->rows;i++)
        {
            gpio_pin_configure_dt(&config->row_gpio[i],GPIO_OUTPUT);
            gpio_pin_set_dt(&config->row_gpio[i],0);
            k_sleep(K_MSEC(1));
            for(ii = 0;ii<config->columns;ii++)
            {
                if(gpio_pin_get_dt(&config->column_gpio[ii]) == 0)
                {
                    key_in = (i*config->columns)+ii;
                }
            }
            gpio_pin_configure_dt(&config->row_gpio[i],GPIO_INPUT);
            k_sleep(K_MSEC(1));
        }
        if(key_in != data->last_pressed)
        {
            if(data->last_pressed != 0xffff)
            {
                if(data->call_backs.button_unpressed!=NULL)
                {
                    data->call_backs.button_unpressed(data->last_pressed);
                }
            }
            data->last_pressed = key_in;
            
            if(key_in != 0xffff)
            {
                LOG_DBG("Button Pressed %d",key_in);
                if(data->call_backs.button_pressed!=NULL)
                {
                    data->call_backs.button_pressed(key_in);
                }
            }
        }

    }
}



static int keypad_init_fn(const struct device *dev)
{
    struct keypad_data *data = dev->data;
    struct keypad_config *config = dev->config;
    uint16_t i = 0;

    data->keypad_device = dev;//This is done so that we can refernce this from a work handler
    data->last_pressed = 0xffff;


    k_work_queue_start(&data->keypad_work_q,data->keypad_work_stack,KEYPAD_STACK_SIZE,KEYPAD_PRI,NULL);
    k_work_init(&data->work,keypad_work_fun);
    for(i = 0;i<config->columns;i++)
    {
        if(gpio_pin_configure_dt(&config->column_gpio[i],GPIO_INPUT))
        {
            LOG_WRN("GPIO NOT CONFIG");
        }
    }
    for(i = 0;i<config->rows;i++)
    {
        if(gpio_pin_configure_dt(&config->row_gpio[i],GPIO_INPUT))
        {
            LOG_WRN("GPIO NOT CONFIG");
        }
    }

    LOG_INF("Column Driver Configured with %d columns and %d rows",config->rows,config->columns);


    k_work_submit_to_queue(&data->keypad_work_q,&data->work);

    return 0;
}

#define GET_GPIOS(node_id, prop, idx)                    \
        GPIO_DT_SPEC_GET_BY_IDX(node_id,prop,idx),



//PAY ATTENTION TO WHAT IS A INST GET AND WHAT IS NOT
#define KEY_PAD_INIT(i)                                                                                      \
static const struct gpio_dt_spec const_column_gpio_##i[] = {DT_INST_FOREACH_PROP_ELEM(i,column_gpios,GET_GPIOS)};  \
static const struct gpio_dt_spec const_row_gpio_##i[] = {DT_INST_FOREACH_PROP_ELEM(i,rows_gpios,GET_GPIOS)};   \
K_THREAD_STACK_DEFINE(i_will_regret_this_##i,KEYPAD_STACK_SIZE);                                                    \
static const struct keypad_config keypad_config_##i = {                                                      \
    .rows = ARRAY_SIZE(const_row_gpio_##i),                                                                  \
    .columns = ARRAY_SIZE(const_row_gpio_##i),                                                               \
    .row_gpio = const_row_gpio_##i,                                                                          \
    .column_gpio = const_column_gpio_##i,                                                                       \
};                                                                                                           \
static struct keypad_data keypad_data_##i = {.keypad_work_stack = i_will_regret_this_##i,};                  \
DEVICE_DT_INST_DEFINE(i,&keypad_init_fn,NULL,&keypad_data_##i,&keypad_config_##i,POST_KERNEL,CONFIG_APPLICATION_INIT_PRIORITY,&keypad_api);\

DT_INST_FOREACH_STATUS_OKAY(KEY_PAD_INIT)

