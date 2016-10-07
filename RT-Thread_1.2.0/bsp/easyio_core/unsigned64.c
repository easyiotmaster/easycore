/*
    File Name: main.c 
    File Desc: Test for u64 function with gcc 3.4.5
    File Version: 1.0.0
    Authour: Tody Kwok
    
    (c) Tody 2010, T-ware Inc.
    
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
*/
#include <stdio.h>
#include <stdlib.h>
#include "unsigned64.h"
#include <rtthread.h>

struct abc{
//    unsigned int  y;
//    unsigned long z;
    char x;
    int y;
    };

int test_u64(int argc, char* argv[])
{
    u64 a , b, c;
    struct abc d;
    
    a = u64hilo(0x00e0, 0x12345678);
    b = u64hilo(0x0000, 0xF0000000);
    c = u64plus(a , b);
    rt_kprintf("%08x%08x", c.hi, c.lo);  // Result: 000000e012345681
    rt_kprintf("/nchar: %d, int: %d, long: %d/n abc: %d, u64: %d",
            sizeof(char),
            sizeof(int),
            sizeof(long),
            sizeof(d),
            sizeof(u64));
//    _getch();
	return 0;
}
