
#include "hi3593.h"

int hi3593LoopTest(int fd)
{
	uint8_t op = 0;
	unsigned char op1[2] = {0};

	op = READ_OP_CODE(HI3593_OP_R_TX_CTRL);
	spiCmdRead(fd, op, op1, 1);

	op = WRITE_OP_CODE(HI3593_OP_W_TX_CTRL);
	op1[0] |= 0x10;
	spiCmdWrite(fd, op, op1, 1);

	return 0;
}

unsigned char hi3593ChnelStatus(int fd, char ch)
{
	unsigned char op;
	unsigned char data[2];
	
	switch((enum CHANNEL_TYPE)ch)
	{
		case TX: 	
		    op = READ_OP_CODE(HI3593_OP_R_TX_ST); 
		    break;
		    
		case RX1: 	
		    op = READ_OP_CODE(HI3593_OP_R_RX1_ST); 
		    break;
		    
		case RX2: 	
		    op = READ_OP_CODE(HI3593_OP_R_RX2_ST); 
		    break;
		    
		default:	
		    op = READ_OP_CODE(HI3593_OP_NULL); 
		    break;
	}

    spiCmdRead(fd, op, data, 1);
    
	return data[0];
}



// ÐŽÒ»Ìõ429ÏûÏ¢£¬ÐèÍâ²¿±£Ö€FIFO·Ç¿Õ
void hi3593WriteFifo(int fd, unsigned int data)
{
	unsigned char op = WRITE_OP_CODE(HI3593_OP_W_TX_FIFO);
	unsigned char buf[4];

	buf[3] = data & 0xff;
	buf[2] = (data >> 8) & 0xff;
	buf[1] = (data >> 16) & 0xff;
	buf[0] = (data >> 24) & 0xff;
        /*
        buf[0] = data & 0xff;
	buf[1] = (data >> 8) & 0xff;
	buf[2] = (data >> 16) & 0xff;
	buf[3] = (data >> 24) & 0xff;
            *///big and smal Endian大小端??
        spiCmdWrite(fd, op, buf, 4);
}

// ¶ÁÒ»Ìõ429ÏûÏ¢£¬ÐèÍâ²¿±£Ö€FIFO·Ç¿Õ
uint32_t hi3593ReadFifo(int fd, char ch)
{
    uint8_t data[32] = {0};
    uint32_t ret = 0;
    uint8_t op = 0;
    if (1 == ch)
    {
	op = READ_OP_CODE(HI3593_OP_R_RX1_FIFO);
    }
    if (2 == ch)
    {
	op = READ_OP_CODE(HI3593_OP_R_RX2_FIFO);
    }

    spiCmdRead(fd, op, data, 4);
	//printf("spiCmdRead. %x %x %x %x\n", data[0], data[1], data[2], data[3]);
    ret = ((data[3]) | ((data[2] & 0xff) << 8) | ((data[1] & 0xff) << 16) | ((data[0] & 0xff) << 24));//ŽóÐ¡¶Ë??
    //ret = ((data[0]) | ((data[1] & 0xff) << 8) | ((data[2] & 0xff) << 16) | ((data[3] & 0xff) << 24));
    return ret;
}



int hi3593Read429(int fd, char ch, uint32_t *p)
{
	RX_STATUS_REG st;
	int i;
	
	for (i = 0; i < NUM; ++i)
	{
		st.bytes = hi3593ChnelStatus(fd, ch);
		if (st.it.FFEMPTY)	break;		//test	
		p[i] = hi3593ReadFifo(fd, ch);
	}
	
	return i;
}

// ÐŽ429ÊýŸÝµœFIFO,
// p ÐŽÊýŸÝÖžÕë£¬len ÐŽÈëµÄ429ÏûÏ¢Êý
// ·µ»ØÊµŒÊÐŽÈëµÄÌõÊý
int hi3593Write429(int fd, unsigned int *p, int num)
{
	TX_STATUS_REG st;
	int i;
	
	for (i = 0; i < num; ++i)
	{
		st.bytes = hi3593ChnelStatus(fd, TX);
		if(st.it.FFFULL == 1) break;		//test		
		hi3593WriteFifo(fd, *(p+i));
	}
	return i;
}

// ÐŸÆ¬žŽÎ»
void hi3593Reset(int fd)
{
	uint8_t op = WRITE_OP_CODE(HI3593_OP_MR);

	spiCmdWrite(fd, op, NULL, 0);
}

// ÈížŽÎ»
void hi3593SoftReset(int fd)
{
	uint8_t op = WRITE_OP_CODE(HI3593_OP_SR);
	spiCmdWrite(fd, op, NULL, 0);
}

// ÅäÖÃRX¿ØÖÆŒÄŽæÆ÷
void hi3593RxControl(int fd, int ch, enum RATE_TYPE rate)
{
	uint8_t op, buf[32] = {0};
	RX_CONTROL_REG reg;
	reg.it.RFLIP = FLIP;
	reg.it.SD9   = 0;
	reg.it.SD10  = 0;
	reg.it.SDON  = 0;
	reg.it.PARITY= 0;
	reg.it.LABREC= 0;
	reg.it.PLON  = 0;
	reg.it.RATE  = rate;

	if(ch == 1)
		op = WRITE_OP_CODE(HI3593_OP_W_RX1_CTRL);
	else if(ch == 2)
		op = WRITE_OP_CODE(HI3593_OP_W_RX2_CTRL);
	else
		op = WRITE_OP_CODE(HI3593_OP_NULL);

	buf[0] = reg.bytes;
	spiCmdWrite(fd, op, buf, 1);
}

// ÅäÖÃTX¿ØÖÆŒÄŽæÆ÷
void hi3593TxControl(int fd, enum CHECK_TYPE oddeven, enum RATE_TYPE rate)
{
	uint8_t op, buf[32] = {0};
	TX_CONTROL_REG reg;
	reg.it.HIZ      = 0;
	reg.it.TFLIP     = FLIP;
	reg.it.TMODE    = 1; // data sent immediately
	reg.it.SELFTEST = 0;
	reg.it.ODDEVEN  = oddeven;
	reg.it.TPARITY  = 0;
	reg.it.RATE  = rate;

	op = WRITE_OP_CODE(HI3593_OP_W_TX_CTRL);
	buf[0] = reg.bytes;
	spiCmdWrite(fd, op, buf, 1);
}

// ÅäÖÃTXÎªžß×èÌ¬
void hi3593TxResistance(int fd)
{
	uint8_t op, buf[32] = {0};
	TX_CONTROL_REG reg;
	reg.it.HIZ      = 1;
	reg.it.TFLIP    = 0;
	reg.it.TMODE    = 1; // data sent immediately
	reg.it.SELFTEST = 0;
	reg.it.ODDEVEN  = 0;
	reg.it.TPARITY  = 0;
	reg.it.RATE     = 0;

	op = WRITE_OP_CODE(HI3593_OP_W_TX_CTRL);
	buf[0] = reg.bytes;
	spiCmdWrite(fd, op, buf, 1);
}

// ÉèÖÃÊ±ÖÓ·ÖÆµ£º
// Íâ²¿È·±£ÊýÖµÎªÅŒÊý
void hi3593SetAclk(int fd, char aclk_div)
{
	uint8_t op, buf[32] = {0};
	ACLK_DIVISION_REG reg;
	reg.bytes = 0;
	reg.it.DIV = aclk_div;

	op = WRITE_OP_CODE(HI3593_OP_W_ACLK);
	buf[0] = reg.bytes;
	spiCmdWrite(fd, op, buf, 1);
}


// ³õÊŒ»¯ 3593
int hi3593Init(int fd, hi3593Config_t cfg)
{
	hi3593Reset(fd);
	hi3593RxControl(fd, 1, cfg.rx1Rate);
	hi3593RxControl(fd, 2, cfg.rx2Rate);
	hi3593TxControl(fd, cfg.oddeven, cfg.txRate);
	//hi3593TxResistance(fd);
	hi3593SetAclk(fd, A_CLOCK);

	return 0;
}




