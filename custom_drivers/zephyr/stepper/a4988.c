

#define DT_DRV_COMPAT phase1_afour



#include <zephyr.h>
#include <kernel.h>
#include <device.h>
#include <drivers/gpio.h>
#include <pm/device.h>
#include "../include/stepper.h"
#include <logging/log.h>
#include "errno.h"

struct a4988_data {
    bool is_on;
};

struct a4988_config {
    uint16_t steps;
    uint16_t dirs;
    struct gpio_dt_spec en_gpio;
    struct gpio_dt_spec *step_gpio;
    struct gpio_dt_spec *dir_gpio;
};

static int a4988_enable(const struct device *dev)
{
    struct a4988_data *data = dev->data;
    const struct a4988_config *config = dev->config;
    if(data->is_on == false)
    {
        data->is_on = true;
        gpio_pin_set_dt(&config->en_gpio,1U);
    }

    return 0;
}

static int a4988_set_dir(const struct device *dev,uint8_t inst,uint8_t dir)
{
    struct a4988_data *data = dev->data;
    const struct a4988_config *config = dev->config;
    if(inst >= config->dirs)
    {
        return -EINVAL;
    }
    if(data->is_on)
    {
        gpio_pin_set_dt(&config->dir_gpio[inst],(dir ? 0U : 1U));
    }

    return 0;    
}

static int a4988_step(const struct device *dev,uint8_t inst)
{
    struct a4988_data *data = dev->data;
    const struct a4988_config *config = dev->config;
    if(inst >= config->steps)
    {
        return -EINVAL;
    }
    if(data->is_on)
    {
        gpio_pin_toggle_dt(&config->step_gpio[inst]);
    }
    return 0;    
}

static int a4988_disable(const struct device *dev)
{
    struct a4988_data *data = dev->data;
    const struct a4988_config *config = dev->config;
    if(data->is_on == true)
    {
        data->is_on = false;
        gpio_pin_set_dt(&config->en_gpio,0U);
    }
    return 0;    
}

static const struct stepper_api a4988_api = {
    .enable = a4988_enable,
    .set_dir = a4988_set_dir,
    .step = a4988_step,
    .disable = a4988_disable,
};

static int a4988_init(const struct device *dev)
{
    struct a4988_data *data = dev->data;
    const struct a4988_config *config = dev->config;
    data->is_on = false;
    for(uint8_t i = 0;i<config->steps;i++)
    {
        gpio_pin_configure_dt(&config->step_gpio[i],GPIO_OUTPUT_LOW);
    }
    for(uint8_t i = 0;i<config->dirs;i++)
    {
        gpio_pin_configure_dt(&config->dir_gpio[i],GPIO_OUTPUT_LOW);
    }
    gpio_pin_configure_dt(&config->en_gpio,GPIO_OUTPUT_LOW);
    gpio_pin_set_dt(&config->en_gpio,0U);
    return 0;
}


#define GET_GPIOS_STEP(node_id, prop, idx)                    \
        GPIO_DT_SPEC_GET_BY_IDX(node_id,prop,idx),

#define INITA4998(i)                        \
static const struct gpio_dt_spec step_gpio_##i[] = {DT_INST_FOREACH_PROP_ELEM(i,step_gpios,GET_GPIOS_STEP)};     \
static const struct gpio_dt_spec dir_gpio_##i[] = {DT_INST_FOREACH_PROP_ELEM(i,dir_gpios,GET_GPIOS_STEP)};       \
static struct a4988_data a4988_data_##i;                                                                    \
static const struct a4988_config a4988_config_##i = {                                                                   \
    .steps = ARRAY_SIZE(step_gpio_##i),                                                                     \
    .dirs = ARRAY_SIZE(dir_gpio_##i),                                                                       \
    .en_gpio = GPIO_DT_SPEC_INST_GET(i,en_gpios),                                                          \
    .step_gpio = step_gpio_##i,                                                                             \
    .dir_gpio = dir_gpio_##i,                                                                               \
};                                                                                                           \
DEVICE_DT_INST_DEFINE(i,&a4988_init,NULL,&a4988_data_##i,&a4988_config_##i,POST_KERNEL,CONFIG_APPLICATION_INIT_PRIORITY,&a4988_api);\





DT_INST_FOREACH_STATUS_OKAY(INITA4998)


