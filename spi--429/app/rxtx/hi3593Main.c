#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h> 
#include "hi3593.h"
    char *filename1, *filename2;// = argv[1];                          // 以第一路429为例
int hi3593Test429(int fd1, int fd2, int ch)
{
    int i, ret;
    MSG_429 data[32];
    MSG_429 data_rx1[32], data_rx2[32],data_rx[32];
    int num = NUM;
	int data_num1 =0, data_num2 =0;
    for (i = 0; i < num; ++i)
    {
    	data[i].it.data = (i + 10000);
    	data[i].it.label = i + 11;
    }

    while (1)
    {
/*
	hi3593Write429(fd1, (unsigned int*)data, num);
*/	//usleep(5);

	memset(data_rx1, 0, 32 *sizeof(MSG_429));
	memset(data_rx2, 0, 32 *sizeof(MSG_429));

    	ret = hi3593Read429(fd1, 1, (unsigned int*)data_rx1);
		//printf("%s, channel 1. ret:%x\n", filename1, ret);
	if (0 != ret){
	    for (i = 0; i < ret; ++i)
	    {
			data_num1++;
		    printf("%s, data_rx1. data_num: %d, channel 1. %x %x\n", filename1, data_num1, data_rx1[i].it.label, data_rx1[i].it.data);
	    }
	}

	    memset(data_rx1, 0, 32 *sizeof(MSG_429));
	ret = hi3593Read429(fd1, 2, (unsigned int*)data_rx1);
	//printf("%s, channel 2. ret:%x\n", filename1, ret);
	if (0 != ret){
		for (i = 0; i < ret; ++i)
		{
			data_num2++;
		    printf("%s, data_rx1. data_num: %d, channel 2. %x %x\n", filename1, data_num2, data_rx1[i].it.label, data_rx1[i].it.data);
		}
	}
	if (0 != ret){
		hi3593Write429(fd1, (unsigned int*)data, num);
		printf("%s, Write complete.\n", filename1);
	}
/*
	    	ret = hi3593Read429(fd2, 1, (unsigned int*)data_rx2);
		if (0 != ret){
	    for (i = 0; i < ret; ++i)
	    {

	    {
		    printf("data_rx2. channel 2. %x %x\n", data_rx2[i].it.label, data_rx2[i].it.data);
		}
}



	    if (0 == ret) continue;
	    printf("channel %d rcv %d messages.\n", i + 1, ret);
	    for (i = 0; i < 6; i++)
	    {
	    	ret = hi3593Read429(fd2, i%2 + 1, (unsigned int*)data_rx2);
	    	if (0 != ret) break;
	    }

	    if (0 == ret) continue;
	    printf("channel %d rcv %d messages.\n", i + 1, ret);
	    for (i = 0; i < ret; ++i)
	    {
		    printf("%d %d\n", data_rx1[i].it.label, data_rx1[i].it.data);
		}
	    for (i = 0; i < ret; ++i)
	    {
		    printf("%d %d\n", data_rx2[i].it.label, data_rx2[i].it.data);
		}
	    continue; */

	}
	
	return 0;
}

int main(int argc, char *argv[])
{
    int fd1, fd2;
    hi3593Config_t hi3593Cfg = {ODD, R_100K, R_100K, R_100K};
   if (argc < 2) { 
       printf("Error Usage!\r\n"); 
       return -1; 
   }

    filename1 = argv[1];
    //filename2 = "/dev/spi-hi3593.3";
    printf("Welcome to the loongson Momentics IDE\n");
    //fd = spiOpen();
    fd1 = open(filename1, O_RDWR); 
    //fd2 = open(filename2, O_RDWR); 
    hi3593Init(fd1, hi3593Cfg);//test
    //hi3593Init(fd2, hi3593Cfg);//test
    hi3593Test429(fd1, 0, 2);


    return EXIT_SUCCESS;
}
