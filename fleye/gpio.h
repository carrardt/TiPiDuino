#ifndef __FLEYE_GPIO_H
#define __FLEYE_GPIO_H

int init_gpio();
void gpio_write_theta_phi(float theta, float phi, int laserSwitch=0);
void gpio_write_xy_f(float xf, float yf, int laserSwitch=0);
void gpio_write_xy_i(unsigned int xi, unsigned int yi, int laserSwitch=0);

#endif

