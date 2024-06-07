#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/reboot.h>

#include "button_manager.h"
#include "display_controller.h"



#define SW0_NODE	DT_ALIAS(sw0)
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,{0});
static struct gpio_callback button1_cb_data;

#define SW1_NODE	DT_ALIAS(sw1)
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios,{0});
static struct gpio_callback button2_cb_data;

#define SW2_NODE	DT_ALIAS(sw2)
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios,{0});
static struct gpio_callback button3_cb_data;

#define SW3_NODE	DT_ALIAS(sw3)
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET_OR(SW3_NODE, gpios,{0});
static struct gpio_callback button4_cb_data;

static K_WORK_DELAYABLE_DEFINE(debouncer_work, button_manager_debouncer);

//-----------------------------------------------------------------------------------------------------------------------
/*! button_manager_button_pressed
* @brief Callback function for button press
* @param dev Pointer to the device structure
* @param cb Pointer to the gpio callback structure
* @param pins Pin number
*/
void button_manager_button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins)
{
	k_work_reschedule(&debouncer_work, K_MSEC(15));	
}

//-----------------------------------------------------------------------------------------------------------------------
/*! button_manager_debouncer
* @brief Debounce function for buttons
* @param work Pointer to the work structure
*/
void button_manager_debouncer(struct k_work *work)
{
    ARG_UNUSED(work);

	if(gpio_pin_get_dt(&button1)==1){
		#ifdef DEBUG_MODE
			printk("Button 1 pressed\n");
		#endif
		triggerPressed();
	}
	if(gpio_pin_get_dt(&button2)==1){
		#ifdef DEBUG_MODE
			printk("Button 2 pressed\n");
		#endif
		upPressed();
	}
	if(gpio_pin_get_dt(&button3)==1){
		#ifdef DEBUG_MODE
			printk("Button 3 pressed\n");
		#endif
		selectPressed();
	}
	if(gpio_pin_get_dt(&button4)==1){
		#ifdef DEBUG_MODE
			printk("Button 4 pressed\n");
		#endif
		downPressed();
	}
}

//-----------------------------------------------------------------------------------------------------------------------
/*! button_manager_init
* @brief Initialize button manager
*/
void button_manager_init( void )
{
	gpio_pin_configure_dt(&button1, GPIO_INPUT);								
	gpio_pin_interrupt_configure_dt(&button1,GPIO_INT_EDGE_BOTH); 			
	gpio_init_callback(&button1_cb_data, button_manager_button_pressed, BIT(button1.pin));
	gpio_add_callback(button1.port, &button1_cb_data);

	gpio_pin_configure_dt(&button2, GPIO_INPUT);								
	gpio_pin_interrupt_configure_dt(&button2,GPIO_INT_EDGE_BOTH); 			
	gpio_init_callback(&button2_cb_data, button_manager_button_pressed, BIT(button2.pin));
	gpio_add_callback(button2.port, &button2_cb_data);

	gpio_pin_configure_dt(&button3, GPIO_INPUT);								
	gpio_pin_interrupt_configure_dt(&button3,GPIO_INT_EDGE_BOTH); 			
	gpio_init_callback(&button3_cb_data, button_manager_button_pressed, BIT(button3.pin));
	gpio_add_callback(button3.port, &button3_cb_data);

	gpio_pin_configure_dt(&button4, GPIO_INPUT);								
	gpio_pin_interrupt_configure_dt(&button4,GPIO_INT_EDGE_BOTH); 			
	gpio_init_callback(&button4_cb_data, button_manager_button_pressed, BIT(button4.pin));
	gpio_add_callback(button4.port, &button4_cb_data);

	#ifdef DEBUG_MODE
		printk("button_manager_init\n");
	#endif
}



