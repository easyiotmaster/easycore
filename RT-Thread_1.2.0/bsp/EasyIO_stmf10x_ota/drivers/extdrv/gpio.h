/*
 * File      : led.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#ifndef __GP_H__
#define __GP_H__

#include <rtthread.h>

//GPIO0 RESET
//CPIO1 BOOT

void init_gpio(void);
void set_gpio(unsigned char num);
void reset_gpio(unsigned char num);

#endif
