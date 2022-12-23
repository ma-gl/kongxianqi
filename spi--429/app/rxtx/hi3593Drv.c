/*
 * $QNXLicenseC:
 * Copyright 2011, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */



#include "hi3593Drv.h"

int spiCmdRead(int fd, uint8_t op, uint8_t* pBuf, int len)
{
    //mx_ecspi_t *dev = (mx_ecspi_t *)fd;
    //uint32_t data, ctrl = spiSetupExchange(dev, (len + 1), devNo);
    int i, remainder = len & 3; 
    uint8_t val, data[5];
    uint8_t cmd;

    data[0] = op;
    //write(fd, &cmd, sizeof(cmd));
    read(fd, data, len);

    memcpy(pBuf, data + 1, len);
if(len > 1){
    printf("Read data. len = %d. %x %x %x %x %x\n", len, data[0], data[1], data[2], data[3], data[4]);
    printf("Read pBuf. %x %x %x %x\n", pBuf[0], pBuf[1], pBuf[2], pBuf[3]);}
 #if 0
    switch(remainder) 
    {
        case 0:
            val = op;
            write(fd, (uint8_t *)&val, sizeof(val));
            break;
        case 1:
            val = op << 8;
            write(fd, (uint8_t *)&val, sizeof(val));
            break;
        case 2:
            val = op << 16;
            write(fd, (uint8_t *)&val, sizeof(val));
            break;
        case 3:
        default:
            val = op << 24;
            write(fd, (uint8_t *)&val, sizeof(val));
            break;
    }

    i = len >> 2;		// divide by 4
    while (i--) 
    {
        val = 0;
        write(fd, (uint8_t *)&val, sizeof(val));	// shift out zeroes; we don't care what
    }

    //spiStartExchange(dev, ctrl);
    //spiWait(dev, (len + 1));

    // discard first four bytes received (correspond to when command was sent)
    if (len >= 4)
    {
        //(void)in32(dev->vbase + MX_ECSPI_RXDATA);
        read(fd, (uint8_t *)&val, sizeof(val));
    }

    switch(remainder) 
    {
    	case 0:
    		break;
    	case 1:
                read(fd, pBuf, remainder);
    		//pBuf[0] = in32(dev->vbase + MX_ECSPI_RXDATA);
    		break;
    	case 2:
                read(fd, pBuf, remainder);
    		//data = in32(dev->vbase + MX_ECSPI_RXDATA);
    		//pBuf[0] = data >> 8;
    		//pBuf[1] = data;
    		break;
    	case 3:
                read(fd, pBuf, remainder);
    		//data = in32(dev->vbase + MX_ECSPI_RXDATA);
    		//pBuf[0] = data >> 16;
    		//pBuf[1] = data >>  8;
    		//pBuf[2] = data;
    		break;
    		default: break;
    }
    pBuf += remainder;

    i = len >> 2;	// divide by 4
    while(i--) 
    {
        read(fd, pBuf, 4);
    	//data = in32(dev->vbase + MX_ECSPI_RXDATA);
    	//pBuf[0] = data >> 24;
    	//pBuf[1] = data >> 16;
    	//pBuf[2] = data >>  8;
    	//pBuf[3] = data;
    	pBuf += 4;
    }
    //spiEndExchange(dev, ctrl);
#endif 
    return 0;
}

int spiCmdWrite(int fd, uint8_t op, uint8_t* pBuf, int len)
{
    //mx_ecspi_t *dev = (mx_ecspi_t *)fd;
	//uint32_t ctrl = spiSetupExchange(dev, (len + 1), devNo);
	int i, remainder = len & 3;
	uint32_t val;
	uint8_t cmd[100];

        cmd[0] = op;
        if (pBuf  != NULL)
        {
            memcpy(&cmd[1], pBuf, len);
        }
        write(fd, cmd, len + 1);
        //printf("spiCmdWrite len=%d write:", len);//test	
        for (i =0; i < len +1; i++){
            printf(" 0x%x ", cmd[i]);
        }
        printf("\n");

#if 0
	switch(remainder) 
	{
		case 0:
                        val = op;
			write(fd, (uint8_t *)&val, sizeof(val));
                        //out32(dev->vbase + MX_ECSPI_TXDATA, op);
			break;
		case 1:
                        val = (op << 8) | pBuf[0];
			write(fd, (uint8_t *)&val, sizeof(val));
			break;
		case 2:
                        val = (op << 16) | (pBuf[0] << 8) | pBuf[1];
			write(fd, (uint8_t *)&val, sizeof(val));
			break;
		case 3:
                        val = (op << 24) | (pBuf[0] << 16) | (pBuf[1] << 8) | pBuf[2];
			write(fd, (uint8_t *)&val, sizeof(val));
			break;
    		default: break;
	}
	
	pBuf += remainder;
	i = len >> 2;
	while (i--) 
	{
    	        val = ((pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8) | pBuf[3]);
		write(fd, (uint8_t *)&val, sizeof(val));
		pBuf += 4;
	}
#endif 
	//spiStartExchange(dev, ctrl);
	//spiWait(dev, (len + 1));
	//spiEndExchange(dev, ctrl);
	
	return 0;
}


