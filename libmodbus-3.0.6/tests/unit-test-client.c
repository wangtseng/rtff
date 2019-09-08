/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <unistd.h>

#include "unit-test.h"

enum {
    TCP,
    TCP_PI,
    RTU
};

int main(int argc, char *argv[])
{
    uint16_t *tab_rp_registers;
    modbus_t *ctx;
//    int i;
    int nb_points;
    int rc;
    struct timeval response_timeout;
    ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);

    

    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }
    
    modbus_set_debug(ctx, TRUE);
   
    modbus_set_slave(ctx, 0x01);

    
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
    
    response_timeout.tv_sec=0;
    response_timeout.tv_usec=100000;   //设置modbus超时时间为1000毫秒
    modbus_set_response_timeout(ctx, &response_timeout);


    /* Allocate and initialize the memory to store the registers */
    nb_points = UT_REGISTERS_NB;
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

    printf("** UNIT TESTING **\n\n");
   

    /** HOLDING REGISTERS **/
    /* Many registers */
    while(1){
    
    usleep(1000000);
    rc = modbus_read_registers(ctx, 30, 2, tab_rp_registers);

//    printf("modbus_read_registers: ");
    if (rc != UT_REGISTERS_NB) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

/*
    for (i=0; i < UT_REGISTERS_NB; i++) {
        printf("output %0x\n", tab_rp_registers[i]);
    }
*/
    int value = MODBUS_GET_INT32_FROM_INT16(tab_rp_registers, 0);
    printf("output %d\n", value);

    nb_points = UT_REGISTERS_NB;
	
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));
}
close:
    /* Free the memory */
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
    }
