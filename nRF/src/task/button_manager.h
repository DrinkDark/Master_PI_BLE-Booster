#ifndef _BUTTON_MANAGER_H_
#define _BUTTON_MANAGER_H_

void button_manager_init( void );

void button_manager_button_pressed(const struct device *dev, struct gpio_callback *cb,uint32_t pins);

void button_manager_debouncer(struct k_work *work);

#endif /*_BUTTON_MANAGER_H_*/