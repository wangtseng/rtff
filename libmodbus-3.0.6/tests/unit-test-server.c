#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <sys/socket.h>
#include<sys/time.h>
#include "unit-test.h"
#include <unistd.h>
#include <pthread.h>

enum {
    TCP,
    TCP_PI,
    RTU
};

int global_acquisition_period = 20;
long start_timestamps = 0;

long get_current_timestamps(){
    struct timeval time_now={0};
    long time_sec=0;
    long time_mil=0;

    gettimeofday(&time_now,NULL);
    time_sec=time_now.tv_sec;
    time_mil=time_sec*1000+time_now.tv_usec/1000;
    return time_mil - start_timestamps;
}

void* acquire_by(void *ctx0){
    while(1){
	uint16_t calValue0[2];
	int rc = modbus_read_registers((modbus_t*)ctx0, 30, 2, calValue0);
	if(rc != -1){ 
	    int value0 = MODBUS_GET_INT32_FROM_INT16(calValue0, 0);
	    printf("timestamps:%ld(ms) ; force_feedback:%d(mN*m)\n",  get_current_timestamps(), value0);
	}
	usleep(global_acquisition_period*1000);
    }
}

modbus_t *init_modbus_descriptor(){
    modbus_t *ret = NULL;
    int rc0;

    ret=modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
    modbus_set_slave(ret,0x01);
    rc0 = modbus_connect(ret);

    struct timeval q0;
    q0.tv_sec=0;
    q0.tv_usec=100000;
    modbus_set_response_timeout(ret,&q0);

    if (rc0 == -1) {
	fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
	modbus_free(ret);
	return ret;
    }

    return ret;
}

int do_acquisition(){
    
    modbus_t* ctx0 = init_modbus_descriptor();
    start_timestamps = get_current_timestamps();
    if(ctx0 != NULL){
	int res; 
	pthread_t a_thread;
	void *thread_result;
	res = pthread_create(&a_thread, NULL, acquire_by, (void *)ctx0);
	if(res != 0)
	{
	  perror("Thread creation failed");
	  exit(EXIT_FAILURE);
	}
	res = pthread_join(a_thread, &thread_result);
	if(res != 0)
	{
	  perror("Thread join failed");
	  exit(EXIT_FAILURE);
	}
	printf("Thread joined, it returned %s\n", (char*)thread_result);
    }
    
    /* For RTU */
    //modbus_close(ctx0);
    //modbus_free(ctx0);
    return 0;
}

int main(int argc, char*argv[]){
    do_acquisition();
}
