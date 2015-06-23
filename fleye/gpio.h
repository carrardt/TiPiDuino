#ifndef __FLEYE_GPIO_H
#define __FLEYE_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

int init_gpio();
void gpio_write_theta_phi(float theta, float phi, int laserSwitch);
void gpio_write_xy_f(float xf, float yf, int laserSwitch);
void gpio_write_xy_i(unsigned int xi, unsigned int yi, int laserSwitch);

#ifdef __cplusplus
}
#endif


#endif

