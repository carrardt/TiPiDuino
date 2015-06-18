#ifndef __FLEYE_GPIO_H
#define __FLEYE_GPIO_H

int init_gpio();
void gpio_write_theta_phi(float theta, float phi);
void gpio_write_xy_f(float xf, float yf);
void gpio_write_xy_i(unsigned int xi, unsigned int yi);


#endif

