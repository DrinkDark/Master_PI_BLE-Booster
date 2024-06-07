
//includes
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include "led.h"
#include "../define.h"

#define LED_STACK_SIZE 1024
#define LED_PRIORITY 7

K_THREAD_STACK_DEFINE(LED_STACK, LED_STACK_SIZE);
static struct k_thread ledThread;

#define LED_SLEEP_TIME_MS   200

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

//-----------------------------------------------------------------------------------------------------------------------
/*! led_controller
* @brief Function to control the LEDs
*/
void led_controller(void)
{	
	#ifdef DEBUG_MODE
	 	printk("led_controller\n");
	#endif

	if (!device_is_ready(led1.port)) {return;}
	if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE) < 0) {return;}
	gpio_pin_set_dt(&led1, 0);

	if (!device_is_ready(led2.port)) {return;}
	if (gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE) < 0) {return;}
	gpio_pin_set_dt(&led2, 0);

	if (!device_is_ready(led3.port)) {return;}
	if (gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE) < 0) {return;}
	gpio_pin_set_dt(&led3, 0);

		if (!device_is_ready(led4.port)) {return;}
	if (gpio_pin_configure_dt(&led4, GPIO_OUTPUT_ACTIVE) < 0) {return;}
	gpio_pin_set_dt(&led4, 0);

	while (true)
	{
		gpio_pin_set_dt(&led1, 1);
		k_msleep(LED_SLEEP_TIME_MS);
		gpio_pin_set_dt(&led1, 0);
		k_msleep(LED_SLEEP_TIME_MS);
	}	
}

//-----------------------------------------------------------------------------------------------------------------------
/*! led_thread_init
* @brief Function to initialize the LED thread
*/
void led_thread_init( void )
{
	k_thread_create	(&ledThread,
					LED_STACK,										        
					LED_STACK_SIZE,
					(k_thread_entry_t)led_controller,
					NULL,
					NULL,
					NULL,
					LED_PRIORITY,
					0,
					K_NO_WAIT);	

	 k_thread_name_set(&ledThread, "ledThread");
	 k_thread_start(&ledThread);

	 #ifdef DEBUG_MODE
	 	printk("led_thread_init\n");
	 #endif
}
