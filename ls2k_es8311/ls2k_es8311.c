/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/sound.h>
#include <linux/soundcard.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
//#include <linux/ls2k_uda1342.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <irq.h>
#include <linux/i2c.h>
#include <asm/mach-loongson2/irq.h>
#include <linux/dmaengine.h>
#include <linux/of_dma.h>
#include <linux/regmap.h>//mgl
//#include "sound/soc.h"//mg

#include "ls2k_es8311.h"
#define readl(addr)             (*(volatile unsigned int *)CKSEG1ADDR(addr))
#define readq(addr)             (*(volatile unsigned long *)CKSEG1ADDR(addr))
#define writel(val, addr)       *(volatile unsigned int *)CKSEG1ADDR(addr) = (val)
#define writeq(val, addr)       *(volatile unsigned long *)CKSEG1ADDR(addr) = (val)

/*
 * es8311 register cache
 */
/*
struct reg_default {
	unsigned int reg;
	unsigned int def;
};
 */
static struct reg_default  es8311_reg_defaults[] = {
	{ 0x00, 0x1f },
	{ 0x01, 0x00 },
	{ 0x02, 0x00 },
	{ 0x03, 0x10 },
	{ 0x04, 0x10 },
	{ 0x05, 0x00 },
	{ 0x06, 0x03 },
	{ 0x07, 0x00 },
	{ 0x08, 0xff },
	{ 0x09, 0x00 },
	{ 0x0a, 0x00 },
	{ 0x0b, 0x00 },
	{ 0x0c, 0x20 },
	{ 0x0d, 0xfc },
	{ 0x0e, 0x6a },
	{ 0x0f, 0x00 },

	{ 0x10, 0x13 },
	{ 0x11, 0x7c },
	{ 0x12, 0x02 },
	{ 0x13, 0x40 },
	{ 0x14, 0x10 },
	{ 0x15, 0x00 },
	{ 0x16, 0x04 },
	{ 0x17, 0x00 },
	{ 0x18, 0x00 },
	{ 0x19, 0x00 },
	{ 0x1a, 0x00 },
	{ 0x1b, 0x0c },
	{ 0x1c, 0x4c },
	{ 0x1d, 0x00 },
	{ 0x1e, 0x00 },
	{ 0x1f, 0x00 },

	{ 0x20, 0x00 },
	{ 0x21, 0x00 },
	{ 0x22, 0x00 },
	{ 0x23, 0x00 },
	{ 0x24, 0x00 },
	{ 0x25, 0x00 },
	{ 0x26, 0x00 },
	{ 0x27, 0x00 },
	{ 0x28, 0x00 },
	{ 0x29, 0x00 },
	{ 0x2a, 0x00 },
	{ 0x2b, 0x00 },
	{ 0x2c, 0x00 },
	{ 0x2d, 0x00 },
	{ 0x2e, 0x00 },
	{ 0x2f, 0x00 },

	{ 0x30, 0x00 },
	{ 0x31, 0x00 },
	{ 0x32, 0x00 },
	{ 0x33, 0x00 },
	{ 0x34, 0x00 },
	{ 0x35, 0x00 },
	{ 0x36, 0x00 },
	{ 0x37, 0x08 },
	{ 0x38, 0x00 },
	{ 0x39, 0x00 },
	{ 0x3a, 0x00 },
	{ 0x3b, 0x00 },
	{ 0x3c, 0x00 },
	{ 0x3d, 0x00 },
	{ 0x3e, 0x00 },
	{ 0x3f, 0x00 },

	{ 0x40, 0x00 },
	{ 0x41, 0x00 },
	{ 0x42, 0x00 },
	{ 0x43, 0x00 },
	{ 0x44, 0x00 },
	{ 0x45, 0x00 },
	
};

struct _coeff_div {
	u32 mclk;       /* mclk frequency */
	u32 rate;       /* sample rate */
	u8 prediv;      /* the pre divider with range from 1 to 8 */
	u8 premulti;    /* the pre multiplier with x1, x2, x4 and x8 selection */
	u8 adcdiv;      /* adcclk divider */
	u8 dacdiv;      /* dacclk divider */
	u8 fsmode;      /* double speed or single speed, =0, ss, =1, ds */
	u8 lrck_h;      /* adclrck divider and daclrck divider */
	u8 lrck_l;
	u8 bclkdiv;     /* sclk divider */
	u8 adcosr;      /* adc osr */
	u8 dacosr;      /* dac osr */
};

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
	//mclk     rate   prediv  mult  adcdiv dacdiv fsmode lrch  lrcl  bckdiv  adcosr  dacosr
	/* 8k */
	{2048000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

	/* 11.025k */
	{2822400 , 11025, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

	/* 12k */
	{3072000 , 12000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

	/* 16k */
	{4096000 , 16000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

	/* 22.05k */
	{5644800 , 22050, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

	/* 24k */
	{6144000 , 24000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

	/* 32k */
	{8192000 , 32000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
	
        //mclk     rate   prediv  mult  adcdiv dacdiv fsmode lrch  lrcl  bckdiv adcosr  dacosr
	/* 44.1k */
	{11289600, 44100, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

	/* 48k */
	{12288000, 48000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
	
	/* 64k */
	{16384000, 64000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
};
u32 lrck_freq[] = {8, 11, 12, 16, 22, 24, 32, 44, 48, 64};
struct sp_config {
	u8 spc, mmcc, spfs;
	u32 srate;
	u8 lrcdiv;
	u8 sclkdiv;
};

/* codec private data */

struct	es8311_private {
	struct snd_soc_component *codec;
	struct regmap *regmap;
	u32 mclk;
	bool sclkinv;
	bool mclkinv;
	bool dmic_enable;
};
enum snd_soc_bias_level {
	SND_SOC_BIAS_OFF = 0,
	SND_SOC_BIAS_STANDBY = 1,
	SND_SOC_BIAS_PREPARE = 2,
	SND_SOC_BIAS_ON = 3,
};
static inline int get_coeff(u32 sample_rate)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lrck_freq); i++) {
		if (lrck_freq[i] == sample_rate)
			return i;
	}
	return -EINVAL;
}

static bool es8311_volatile_register(struct device *dev,
			unsigned int reg)
{
	if ((reg  <= 0xff)) {
		return true;
	}
	 else {
		return false;
	}
}

static bool es8311_readable_register(struct device *dev,
			unsigned int reg)
{
	if ((reg  <= 0xff)) {
		return true;
	} 
	else {
		return false;
	}
}
static bool es8311_writable_register(struct device *dev,
			unsigned int reg)
{
	if ((reg  <= 0xff)) {
		return true;
	} 
	else {
		return false;
	}
}

struct es8311_private *es8311_data;
static struct regmap_config es8311_regmap = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = ES8311_MAX_REGISTER,
	.reg_defaults = es8311_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(es8311_reg_defaults),
	.volatile_reg = es8311_volatile_register,
	.writeable_reg = es8311_writable_register,
	.readable_reg  = es8311_readable_register,
	.cache_type = REGCACHE_RBTREE,
};



static int irq0;
static int irq1;

void __iomem *order_addr_in0;
void __iomem *order_addr_in1;
void __iomem *dma_cfg_addr0;
void __iomem *dma_cfg_addr1;
void __iomem *int_sta_addr;
void __iomem *int_cfg_addr;
static int codec_reset = 1;

//static int uda1342_volume;

static irqreturn_t ac97_dma_write_intr(int irq, void *private);
static irqreturn_t ac97_dma_read_intr(int irq, void *private);

static int i2c_read_codec(struct i2c_client *client, unsigned char reg, unsigned char *val);
static int i2c_write_codec(struct i2c_client *client, unsigned char reg, unsigned char val);
static int ls2k_audio_sync(struct file *file);

#define DMA64 0
#if DMA64
struct dma_desc {
	volatile u32 ordered;
	volatile u32 saddr;
	volatile u32 daddr;
	volatile u32 length;
	volatile u32 step_length;
	volatile u32 step_times;
	volatile u32 cmd;
	volatile u32 stats;
	volatile u32 ordered_hi;
	volatile u32 saddr_hi;
};
#else
struct dma_desc {
	volatile u32 ordered;
	volatile u32 saddr;
	volatile u32 daddr;
	volatile u32 length;
	volatile u32 step_length;
	volatile u32 step_times;
	volatile u32 cmd;
	volatile u32 stats;
};
#endif
struct audio_dma_desc {
	struct dma_desc snd;
	struct dma_desc null;
	struct list_head link;
	struct list_head all;
	dma_addr_t snd_dma_handle;
	dma_addr_t snd_dma;
	u32 pos;
	char *snd_buffer;
};

struct audio_stream {
	struct dma_desc *ask_dma;
	struct list_head free_list;
	struct list_head run_list;
	struct list_head done_list;
	struct list_head all_list;
	spinlock_t lock;
	u32 nbfrags;
	u32 fragsize;
	struct semaphore sem;
	wait_queue_head_t frag_wq;
	dma_addr_t ask_dma_handle;

	int num_channels;
	u32 output;
	u32 sample_rate;
	u32 sample_size;
	u32 rate;
	u32 state;
};

static struct audio_stream input_stream = {
	output: 0,
	fragsize:0x10000,
};

static struct audio_stream output_stream = {
	output: 1,
	fragsize:0x10000,
};

unsigned short reg1[7] = {
        0x1402,
        0x0014,
        0xff03,
        0x0000,
        0x0000,
        0x0030,
        0x0030,
};


#define uda1342_base    0xffffffffbfe0d000
#define DMA_BUF		0x00800000

#define IISVERSION	(volatile unsigned int *)(uda1342_base + 0x0)
#define IISCONFIG	(volatile unsigned int *)(uda1342_base + 0x4)
#define IISSTATE	(volatile unsigned int *)(uda1342_base + 0x8)
#define IISRxData	(0x1fe0d000 + 0xc)
#define	IISTxData	(0x1fe0d000 + 0x10)

#define I2C_SINGLE	0
#define I2C_BLOCK	1
#define I2C_SMB_BLOCK 2
#define I2C_HW_SINGLE 3

#define GS_SOC_I2C_BASE    0xffffffffbfe01000
#define GS_SOC_I2C_PRER_LO (volatile unsigned char *)(GS_SOC_I2C_BASE + 0x0)
#define GS_SOC_I2C_PRER_HI (volatile unsigned char *)(GS_SOC_I2C_BASE + 0x1)
#define GS_SOC_I2C_CTR     (volatile unsigned char *)(GS_SOC_I2C_BASE + 0x2)
#define GS_SOC_I2C_TXR     (volatile unsigned char *)(GS_SOC_I2C_BASE + 0x3)
#define GS_SOC_I2C_RXR     (volatile unsigned char *)(GS_SOC_I2C_BASE + 0x3)
#define GS_SOC_I2C_CR      (volatile unsigned char *)(GS_SOC_I2C_BASE + 0x4)
#define GS_SOC_I2C_SR      (volatile unsigned char *)(GS_SOC_I2C_BASE + 0x4)

#define CR_START 0x80
#define CR_WRITE 0x10
#define SR_NOACK 0x80
#define CR_STOP  0x40
#define SR_BUSY  0x40
#define CR_START 0x80
#define SR_TIP   0x2

#define CR_READ  0x20
#define I2C_WACK 0x8

enum {
	STOP = 0,
	RUN = 1
};

/* Boot options
 * 0 = no VRA, 1 = use VRA if codec supports it
 */
static int      vra = 1;
module_param(vra, bool, 0);
MODULE_PARM_DESC(vra, "if 1 use VRA if codec supports it");

static struct ls2k_audio_state {
	void __iomem *base;
	/* soundcore stuff */
	int dev_audio;
	int dev_mixer;

	struct ac97_codec *codec;
	unsigned codec_base_caps; /* AC'97 reg 00h, "Reset Register" */
	unsigned codec_ext_caps;  /* AC'97 reg 28h, "Extended Audio ID" */
	int no_vra;		/* do not use VRA */

	spinlock_t lock;
	struct mutex open_mutex;
	struct mutex mutex;
	fmode_t open_mode;
	wait_queue_head_t open_wait;

	struct audio_stream	 *input_stream;
	struct audio_stream	 *output_stream;

	u32 rd_ref:1;
	u32 wr_ref:1;
	struct semaphore sem;
} ls2k_audio_state;


unsigned int  es8311_soc_component_read32(struct regmap *regmap, unsigned int reg)
{
    unsigned int val  = 0;
    val = regmap_read(regmap, reg, &val);
    return val;
}
int regmap_bits_update(struct regmap *map, unsigned int reg, unsigned int mask, unsigned int val)
{
	bool change;
	int ret = 0;
	if (map)
		ret = regmap_update_bits_check(map, reg, mask,
			val, &change);
	if (ret < 0)
		return ret;
	return change;
}
static void dma_enable_trans(struct audio_stream * s, struct audio_dma_desc *desc)
{
#if DMA64
	u64 val;
#else
	u32 val;
#endif
	int timeout = 20000;
	unsigned long flags;

	val = desc->snd_dma_handle;
	val |= 0x8;
#if DMA64
	val |= 1;
	local_irq_save(flags);
	writeq(val, (s->output ? order_addr_in0 : order_addr_in1));
#else
	local_irq_save(flags);
	writel(val, (s->output ? order_addr_in0 : order_addr_in1));
#endif
	while ((readl(s->output ? order_addr_in0 : order_addr_in1) & 0x8) && (timeout-- > 0)) {
		udelay(5);
	}
	local_irq_restore(flags);
}

void audio_clear_buf(struct audio_stream *s)
{
	struct audio_dma_desc *desc;

	while (!list_empty(&s->all_list)) {
		desc = list_entry(s->all_list.next, struct audio_dma_desc, all);
		list_del(&desc->all);
		list_del(&desc->link);

		if (desc->snd_buffer)
			free_pages((unsigned long)desc->snd_buffer, get_order(s->fragsize));
		dma_free_coherent(NULL, sizeof(struct audio_dma_desc), desc, 0);
	}

	if (s->ask_dma)
		dma_free_coherent(NULL, sizeof(struct audio_dma_desc), s->ask_dma, 0);

	s->ask_dma = NULL;
}

static void inline link_dma_desc(struct audio_stream *s, struct audio_dma_desc *desc)
{
	spin_lock_irq(&s->lock);

	if(!list_empty(&s->run_list)) {
		struct audio_dma_desc *desc0;
		desc0 = list_entry(s->run_list.prev, struct audio_dma_desc, link);
#if DMA64
		desc0->snd.ordered = (desc->snd_dma_handle | DMA_ORDERED_EN) & 0xffffffff;
		desc0->snd.ordered_hi = (desc->snd_dma_handle | DMA_ORDERED_EN) >> 32;
		desc0->null.ordered = (desc->snd_dma_handle | DMA_ORDERED_EN) & 0xffffffff;
		desc0->null.ordered_hi = (desc->snd_dma_handle | DMA_ORDERED_EN) >> 32;
#else
		desc0->snd.ordered = desc->snd_dma_handle | DMA_ORDERED_EN;
		desc0->null.ordered = desc->snd_dma_handle | DMA_ORDERED_EN;
#endif
		list_add_tail(&desc->link, &s->run_list);
		if(s->state == STOP) {
			s->state = RUN;
			dma_enable_trans(s, desc0);
		}
	}
	else {
		list_add_tail(&desc->link,&s->run_list);
		dma_enable_trans(s, desc);
	}

	spin_unlock_irq(&s->lock);
}

static void ls2k_init_dmadesc(struct audio_stream *s, struct audio_dma_desc *desc, u32 count)
{
	struct dma_desc *_desc;
	u32 control;

	control = s->output ? IISTxData: IISRxData;
	desc->snd.daddr = desc->null.daddr = control;
	_desc = &desc->snd;
#if DMA64
	_desc->ordered = ((desc->snd_dma_handle + sizeof(struct dma_desc)) | DMA_ORDERED_EN) & 0xffffffff;
	_desc->ordered_hi = ((desc->snd_dma_handle + sizeof(struct dma_desc)) | DMA_ORDERED_EN) >> 32;
	_desc->saddr = (desc->snd_dma & 0xffffffff);
	_desc->saddr_hi = desc->snd_dma >> 32;
#else
	_desc->ordered = (desc->snd_dma_handle + sizeof(struct dma_desc)) | DMA_ORDERED_EN;
	_desc->saddr = desc->snd_dma;
#endif
	_desc->length = 8;
	_desc->step_length = 0;
	_desc->step_times = count >> 5;
	_desc->cmd = s->output ? 0x00001001 : 0x00000001;

	_desc = &desc->null;
#if DMA64
	_desc->ordered = ((desc->snd_dma_handle + sizeof(struct dma_desc)) | DMA_ORDERED_EN) & 0xffffffff;
	_desc->ordered_hi = ((desc->snd_dma_handle + sizeof(struct dma_desc)) | DMA_ORDERED_EN) >> 32;
	_desc->saddr = (desc->snd_dma & 0xffffffff);
	_desc->saddr_hi = desc->snd_dma >> 32;
#else
	_desc->ordered =  (desc->snd_dma_handle + sizeof(struct dma_desc)) | DMA_ORDERED_EN;
	_desc->saddr = desc->snd_dma;
#endif
	_desc->length = 8;
	_desc->step_length = 0;
	_desc->step_times = 1;
	_desc->cmd = s->output ? 0x00001000 : 0x00000000;
}

int ls2k_setup_buf(struct audio_stream * s)
{
	int i;
	dma_addr_t dma_phyaddr;

	if (s->ask_dma)
		return -EBUSY;

	for (i=0; i <s->nbfrags; i++) {
		struct audio_dma_desc *desc;

		desc = dma_alloc_coherent(NULL, sizeof(struct audio_dma_desc),
					(dma_addr_t *)&dma_phyaddr, GFP_KERNEL);
		if (!desc)
			goto err;

		memset(desc, 0, sizeof(struct audio_dma_desc));

		desc->snd_dma_handle = dma_phyaddr;
#if DMA64
		desc->null.ordered = ((dma_phyaddr + sizeof(struct dma_desc)) | DMA_ORDERED_EN) & 0xffffffff;
		desc->null.ordered_hi = ((dma_phyaddr + sizeof(struct dma_desc)) | DMA_ORDERED_EN) >> 32;
		desc->snd.ordered = ((dma_phyaddr + sizeof(struct dma_desc)) | DMA_ORDERED_EN) & 0xffffffff;
		desc->snd.ordered_hi = ((dma_phyaddr + sizeof(struct dma_desc)) | DMA_ORDERED_EN) >> 32;
#else
		desc->null.ordered = (dma_phyaddr + sizeof(struct dma_desc)) | DMA_ORDERED_EN;
		desc->snd.ordered = (dma_phyaddr + sizeof(struct dma_desc)) | DMA_ORDERED_EN;
#endif
		list_add_tail(&desc->link,&s->free_list);
		list_add_tail(&desc->all,&s->all_list);

		desc->snd_buffer = dma_alloc_coherent(NULL, s->fragsize,
					(dma_addr_t *)&dma_phyaddr, GFP_KERNEL);
		if (!desc->snd_buffer)
			goto err;

		desc->snd_dma = dma_phyaddr;
	}

	/* dma desc for ask_valid one per struct audio_stream */
	s->ask_dma = dma_alloc_coherent(NULL, sizeof(struct dma_desc),
			&dma_phyaddr, GFP_KERNEL);
	if(!s->ask_dma)
		goto err;

	memset(s->ask_dma, 0, sizeof(struct dma_desc));
	s->ask_dma_handle = dma_phyaddr;

	sema_init(&s->sem, 1);

	return 0;

err:
	audio_clear_buf(s);
	printk(KERN_ERR "unable to allocate audio memory\n");
	return -ENOMEM;
}

static irqreturn_t ac97_dma_read_intr(int irq, void *private)
{
	struct audio_stream *s = (struct audio_stream *)private;
	struct audio_dma_desc *desc;
	unsigned long flags;
	if (list_empty(&s->run_list))
		return IRQ_HANDLED;

	local_irq_save(flags);
	writel(s->ask_dma_handle | 0x4, order_addr_in1);
	while (readl(order_addr_in1) & 4) {
	}
	local_irq_restore(flags);

	do {
		desc = list_entry(s->run_list.next, struct audio_dma_desc, link);
		if (s->ask_dma->ordered == desc->snd.ordered)
			break;

		list_del(&desc->link);
		list_add_tail(&desc->link, &s->done_list);
	} while(!list_empty(&s->run_list));

	if (!list_empty(&s->done_list))
		wake_up(&s->frag_wq);
	return IRQ_HANDLED;
}

static irqreturn_t ac97_dma_write_intr(int irq, void *private)
{
	struct audio_stream *s = (struct audio_stream *)private;
	struct audio_dma_desc *desc;
	unsigned long flags;
	if (list_empty(&s->run_list))
		return IRQ_HANDLED;

	local_irq_save(flags);
	writel(s->ask_dma_handle | 0x4, order_addr_in0);
	while (readl(order_addr_in0) & 4) {
	}
	local_irq_restore(flags);

	do {
		desc = list_entry(s->run_list.next, struct audio_dma_desc, link);
		/*first desc's ordered may be null*/
		if(s->ask_dma->ordered == desc->snd.ordered || s->ask_dma->ordered == 
			((desc->snd_dma_handle + sizeof(struct dma_desc)) | DMA_ORDERED_EN))
			break;
		list_del(&desc->link);
		desc->pos = 0;
		list_add_tail(&desc->link, &s->free_list);
	} while(!list_empty(&s->run_list));

	if (!list_empty(&s->free_list))
		wake_up(&s->frag_wq);
	return IRQ_HANDLED;
}

static u32 fill_play_buffer(struct audio_stream *s, const char *buf, u32 count)
{
	struct audio_dma_desc *desc;
	u32 copy_bytes;

	desc = list_entry(s->free_list.next, struct audio_dma_desc, link);
	if(s->num_channels == 1)
	{
		int i,j;
		unsigned char c;
		copy_bytes = min((s->fragsize - desc->pos), count*2)/2;
		
		for(i = 0, j = 0;i < copy_bytes;i += 2, j += 4)
		{
			get_user(c,buf+i);
			*(desc->snd_buffer + desc->pos + j) = c;
			*(desc->snd_buffer + desc->pos + j + 2) = c;
			get_user(c,buf+i+1);
			*(desc->snd_buffer + desc->pos + j + 1) = c;
			*(desc->snd_buffer + desc->pos + j +  3) = c;
		}
		desc->pos += j;
	}
	else
	{
		copy_bytes = min((s->fragsize - desc->pos), count);
		copy_from_user((void *)(desc->snd_buffer + desc->pos), buf, copy_bytes);
		desc->pos += copy_bytes;
	}
	

	if (desc->pos == s->fragsize) {
		list_del(&desc->link);
		ls2k_init_dmadesc(s, desc, s->fragsize);
		link_dma_desc(s, desc);
/*
		u64 value =  readq(order_addr_in0);
		value &= 0xfffffffffffffff0;
		u64 add = ioremap(value, 0x4);
*/
	}

	return copy_bytes;
}

static int ls2k_audio_write(struct file *file, const char *buffer, size_t count, loff_t *ppos)
{
	struct ls2k_audio_state *state = (struct ls2k_audio_state *)file->private_data;
	struct audio_stream *s = state->output_stream;
	const char *buffer0 = buffer;
	unsigned int i, ret = 0;
	if (*ppos != file->f_pos) {
		return -ESPIPE;
	}
	if (!s->ask_dma && ls2k_setup_buf(s)) {
		return -ENOMEM;
	}

	if (file->f_flags & O_NONBLOCK) {
		if (down_trylock(&s->sem))
			return -EAGAIN;
	} else {
		if (down_interruptible(&s->sem))
			return -ERESTARTSYS;
	}
	while (count > 0) {
		if(list_empty(&s->free_list)) {
			if(file->f_flags & O_NONBLOCK)
				return -EAGAIN;
			if(wait_event_interruptible(s->frag_wq, !list_empty(&s->free_list))) {
				up(&s->sem);
				return -ERESTARTSYS;
			}
		}
		/* Fill data , if the ring is not full */
		ret = fill_play_buffer(s, buffer, count);
		count -= ret;
		buffer += ret;
	}

	up(&s->sem);

	return (buffer - buffer0);
}
static int ls2k_copy_to_user(struct audio_stream *s, char *buffer, u32 count)
{
	struct audio_dma_desc *desc;
	int ret = 0;

	while (!list_empty(&s->done_list) && count) {
		u32 left;
		desc = list_entry(s->done_list.next, struct audio_dma_desc, link);
		left = min(s->fragsize - desc->pos,count);
		copy_to_user(buffer, (void *)(desc->snd_buffer + desc->pos), left);
		desc->pos += left;
		count -= left;
		buffer += left;
		ret += left;
		if (desc->pos == s->fragsize) {
			list_del(&desc->link);
			desc->pos = 0;
			list_add_tail(&desc->link, &s->free_list);
		}
	}

	return ret;
}

static int es8311_set_level(struct es8311_private *private,
			enum snd_soc_bias_level level)
{
	int regv;
	//struct es8311_private *es8311 = (struct es8311_private *)snd_soc_component_get_drvdata(codec);
    struct es8311_private *es8311 = private;
    printk("Enter into %s(), level = %d\n", __func__, level);
	switch (level) {
	case SND_SOC_BIAS_ON:
		regmap_write(es8311->regmap, ES8311_GP_REG45, 0x00);
		regmap_write(es8311->regmap, ES8311_ADC_REG16, 0x24);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG0B, 0x00);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG0C, 0x00);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG10, 0x1F);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG11, 0x7F);
		regmap_write(es8311->regmap, ES8311_RESET_REG00, 0x80);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG0D, 0x01);
		regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x3F);
		if(es8311->mclkinv == true) {
                        
			regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG01,
					0x40, 0x40);
		}
		else {
			regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG01,
					0x40, 0x00);
		}
		if(es8311->sclkinv == true) {
			regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG06,
					0x20, 0x20);
		}
		else {
			regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG06,
					0x20, 0x00);
		}
		regv = es8311_soc_component_read32(es8311->regmap, ES8311_SYSTEM_REG14) & 0xCF;
		regv |= 0x1A;
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG14, regv);
		if(es8311->dmic_enable == true) {
			regmap_bits_update(es8311->regmap, ES8311_SYSTEM_REG14,
					0x40, 0x40);
		}
		else {
			regmap_bits_update(es8311->regmap, ES8311_SYSTEM_REG14,
					0x40, 0x00);
		}
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG13, 0x10);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG0E, 0x02);
		regmap_write(es8311->regmap, ES8311_ADC_REG15, 0x40);
		regmap_write(es8311->regmap, ES8311_ADC_REG1B, 0x0A);
		regmap_write(es8311->regmap, ES8311_ADC_REG1C, 0x6A);
		regmap_write(es8311->regmap, ES8311_DAC_REG37, 0x48);
		regmap_write(es8311->regmap, ES8311_GPIO_REG44, 0x08);
		//regmap_write(es8311->regmap, ES8311_ADC_REG17, 0xBF);
		//regmap_write(es8311->regmap, ES8311_DAC_REG32, 0xBF);
		break;
	case SND_SOC_BIAS_PREPARE:
		break;
	case SND_SOC_BIAS_STANDBY:
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG12, 0x02);
		regmap_write(es8311->regmap, ES8311_DAC_REG32, 0xBF);
		regmap_write(es8311->regmap, ES8311_ADC_REG17, 0xBF);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG0E, 0xFF);
		regmap_write(es8311->regmap, ES8311_SYSTEM_REG0D, 0xFA);
		regmap_write(es8311->regmap, ES8311_ADC_REG15, 0x00);
		regmap_write(es8311->regmap, ES8311_DAC_REG37, 0x08);
		regmap_write(es8311->regmap, ES8311_RESET_REG00, 0x00);
		regmap_write(es8311->regmap, ES8311_RESET_REG00, 0x1F);
		regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x30);
		regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x00);
		regmap_write(es8311->regmap, ES8311_GP_REG45, 0x01);
		break;
	case SND_SOC_BIAS_OFF:
		break;
	}
	//codec->dapm.bias_level = level;
	return 0;
}

static int es8311_mute(struct regmap *map, int mute)
{
	if (mute) {
		regmap_write(map, ES8311_SYSTEM_REG12, 0x02);
                regmap_bits_update(map, ES8311_DAC_REG31, 0x60, 0x60);
		regmap_write(map, ES8311_DAC_REG32, 0x00);
		regmap_write(map, ES8311_DAC_REG37, 0x08);
	} 
	else {
		regmap_bits_update(map, ES8311_DAC_REG31, 0x60, 0x00);
		regmap_write(map, ES8311_SYSTEM_REG12, 0x00);
	}
	return 0;
}
void config_es8311(struct audio_stream *stream, struct es8311_private *es8311)
{
        unsigned char rat_cddiv;
        unsigned char rat_bitdiv;
	u8 coeff, regv, datmp, premulti;
        
        /*
        set  IIS clock parammeters
        loongson2k1000 datasheet: rat_bitdiv= Freq_APB / (RES_DEPTH x 2 x fs) /2 \A8C 1; 
        loongson2k1000 datasheet: rat_cddiv= Freq_APB / (256 x fs) /2 \A8C 1
        Freq_APB :125MHz
        RES_DEPTH:sample_size
        fs:LRCK
        */
	//rat_bitdiv = 0x2b;//= 125M / (16 * 2 * 44.1k) / 2 - 1 =  0x2b
        //rat_cddiv = 0x5;//= 125M / (256 * 44.1k)/2 - 1
        rat_bitdiv = 125 * 1000 / (16 * 2 * stream->sample_rate) / 2 - 1;
        rat_cddiv = 125 * 1000 / (256 * stream->sample_rate) / 2 - 1;
        * IISCONFIG = (stream->sample_size<<24) | (stream->sample_size<<16) | (rat_bitdiv<<8) | (rat_cddiv<<0) ;
        
	/*
	* set  es8311 clock parammeters
	*/
        coeff = get_coeff(stream->sample_rate);
        if (coeff < 0) {
            printk("Unable to configure sample rate %dkHz\n", stream->sample_rate);
        }
	regv = es8311_soc_component_read32(es8311->regmap, ES8311_CLK_MANAGER_REG02) & 0x07;
	regv |= (coeff_div[coeff].prediv- 1) << 5;
	datmp = 0;

	switch(coeff_div[coeff].premulti) 
	{
	case 1:
		datmp = 0;
		break;
	case 2:
		datmp = 1;
		break;
	case 4:
		datmp = 2;
		break;
	case 8:
		datmp = 3;
		break;
	default:
		break;
	}
	regv |= (datmp) << 3;
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG02, regv);

	regv = es8311_soc_component_read32(es8311->regmap, ES8311_CLK_MANAGER_REG05) & 0x00;
	regv |= (coeff_div[coeff].adcdiv -1)<< 4;
	regv |= (coeff_div[coeff].dacdiv-1)<< 0;
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG05, regv);

	regv = es8311_soc_component_read32(es8311->regmap, ES8311_CLK_MANAGER_REG03) & 0x80;
	regv |= coeff_div[coeff].fsmode << 6;
	regv |= coeff_div[coeff].adcosr << 0;
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG03, regv);

	regv = es8311_soc_component_read32(es8311->regmap, ES8311_CLK_MANAGER_REG04) & 0x80;
	regv |= coeff_div[coeff].dacosr << 0;
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG04, regv);

	regv = es8311_soc_component_read32(es8311->regmap, ES8311_CLK_MANAGER_REG07) & 0xf0;
	regv |= coeff_div[coeff].lrck_h << 0;
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG07, regv);

	regv = es8311_soc_component_read32(es8311->regmap, ES8311_CLK_MANAGER_REG08) & 0x00;
	regv |= coeff_div[coeff].lrck_l << 0;
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG08, regv);

	regv = es8311_soc_component_read32(es8311->regmap, ES8311_CLK_MANAGER_REG06) & 0xE0;
	regv |= coeff_div[coeff].bclkdiv << 0;
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG06, regv);
	return 0;
}

static int ls2k_audio_read(struct file *file, char *buffer, size_t count, loff_t * ppos)
{
	struct ls2k_audio_state *state = file->private_data;
	struct audio_stream *s = state->input_stream;
	struct audio_dma_desc *desc;
	char *buffer0 = buffer;
	if (*ppos != file->f_pos)
		return -ESPIPE;

	if (!s->ask_dma && ls2k_setup_buf(s))
		return -ENOMEM;

	if (file->f_flags & O_NONBLOCK) {
		if (down_trylock(&s->sem))
			return -EAGAIN;
	} else {
		if (down_interruptible(&s->sem))
			return -ERESTARTSYS;
	}

	while (count > 0) {
		int ret;

		while (!list_empty(&s->free_list)) {
			desc = list_entry(s->free_list.next, struct audio_dma_desc, link);
			list_del(&desc->link);
			ls2k_init_dmadesc(s, desc, s->fragsize);
			link_dma_desc(s, desc);
		}

		/* record's buffer is empty */
		while (list_empty(&s->done_list)) {
			if (file->f_flags & O_NONBLOCK) {
				up(&s->sem);
				return -EAGAIN;
			}
			if(wait_event_interruptible(s->frag_wq, !list_empty(&s->done_list))) {
				up(&s->sem);
				return -ERESTARTSYS;
			}
		}

		/* data is ready now , so copy it */
		ret = ls2k_copy_to_user(s, buffer, count);
		count -= ret;
		buffer += ret;
	}

	while (!list_empty(&s->free_list)) {
		desc = list_entry(s->free_list.next, struct audio_dma_desc, link);
		list_del(&desc->link);
		ls2k_init_dmadesc(s, desc, s->fragsize);
		link_dma_desc(s, desc);
	}

	up(&s->sem);
	return (buffer - buffer0);
}

static long ls2k_audio_ioctl(struct file *file, uint cmd, ulong arg)
{
	struct ls2k_audio_state *state = file->private_data;
	struct audio_stream *os = state->output_stream;
	struct audio_stream *is = state->input_stream;
	uint rd=0, wr=0, val=0, temp=0, confg_data=0;
        struct es8311_private *es8311 = es8311_data;
        //long    var;
        int     ret,reg;
        //unsigned char dev_add[1];

	if (file->f_mode & FMODE_WRITE)
		wr = 1;
	if (file->f_mode & FMODE_READ)
		rd = 1;

	switch (cmd) {
	case SNDCTL_DSP_RESET:
		if (file->f_mode & FMODE_WRITE)
		{
			ls2k_audio_sync(file);
			audio_clear_buf(os);
		}
		if (file->f_mode & FMODE_READ)
		{
			ls2k_audio_sync(file);
			audio_clear_buf(is);
		}
		return 0;
	case SOUND_MIXER_READ_VOLUME:
                regmap_read(es8311->regmap, ES8311_ADC_REG17, &val);
                temp = val<<8 &&0xff00;
                regmap_read(es8311->regmap, ES8311_DAC_REG32, &val);
                temp = temp |(val&0x00ff);
                val = temp;
		return put_user(val, (long*)arg);		
		return 0;
	case SOUND_MIXER_WRITE_VOLUME:
  		ret = get_user(val, (ulong *) arg);
		if(ret)
			return ret;
                temp = (val&0xff00)>>8;
                val = val&0x00ff;
                regmap_write(es8311->regmap, ES8311_ADC_REG17, temp);
                regmap_write(es8311->regmap, ES8311_DAC_REG32, val);
		return 0;
	case OSS_GETVERSION:
		return put_user(SOUND_VERSION, (int *) arg);

	case SNDCTL_DSP_SETDUPLEX:
		return 0;

	case SNDCTL_DSP_GETCAPS:
		val = DSP_CAP_REALTIME | DSP_CAP_TRIGGER | DSP_CAP_MMAP;
		if (is && os)
			val |= DSP_CAP_DUPLEX;
		return put_user(val, (int *) arg);

	case SNDCTL_DSP_SYNC:
		return ls2k_audio_sync(file);

	case SNDCTL_DSP_SPEED: 
		if (get_user(val, (int *) arg))
			return -EFAULT;
		if (val != 0) {
                        if (get_coeff(val) < 0) 
                            return -EINVAL;
			if (file->f_mode & FMODE_READ) {
				is->sample_rate= val;
                                config_es8311(is, es8311);
			}
			if (file->f_mode & FMODE_WRITE) {
				os->sample_rate= val;
                                config_es8311(os, es8311);
        		}
		}
		return put_user(val, (int *) arg);

	case SNDCTL_DSP_STEREO:
		if (get_user(val, (int *)arg)) {
			return -EFAULT;
			break;
		}

		if (wr)
			os->num_channels = val?2:1;
		else 
			is->num_channels = val?2:1;
		put_user(val, (int *) arg);
		return 0;

	case SNDCTL_DSP_CHANNELS:
		if (get_user(val, (int *) arg))
			return -EFAULT;
		if (val != 0) {
			if (file->f_mode & FMODE_READ) {
				if (val < 0 || val > 2)
					return -EINVAL;
				is->num_channels = val;
			}
			if (file->f_mode & FMODE_WRITE) {
				switch (val) {
				case 1:
				case 2:
					break;
				case 3:
				case 5:
					return -EINVAL;
				case 4:
					if (!(state->codec_ext_caps &
					      AC97_EXTID_SDAC))
						return -EINVAL;
					break;
				case 6:
					if ((state->codec_ext_caps &
					     AC97_EXT_DACS) != AC97_EXT_DACS)
						return -EINVAL;
					break;
				default:
					return -EINVAL;
				}


				os->num_channels = val;
			}
		}
		return put_user(val, (int *) arg);

	case SNDCTL_DSP_GETFMTS:
		return put_user(AFMT_S16_LE | AFMT_U8, (int *) arg);

	case SNDCTL_DSP_SETFMT:
		if (get_user(val, (int *) arg))
			return -EFAULT;
		if (val != AFMT_QUERY) {
			if (file->f_mode & FMODE_READ) {
				if (val == AFMT_S16_LE)
					state->input_stream->sample_size = 16;
				else {
					val = AFMT_U8;
					state->input_stream->sample_size = 8;
				}
			}
			if (file->f_mode & FMODE_WRITE) {
				if (val == AFMT_S16_LE)
					state->output_stream->sample_size = 16;
				else {
					val = AFMT_U8;
					state->output_stream->sample_size = 8;
				}
			}
		} else {
			if (file->f_mode & FMODE_READ)
				val = (state->input_stream->sample_size == 16) ?
					AFMT_S16_LE : AFMT_U8;
			else
				val = (state->output_stream->sample_size == 16) ?
					AFMT_S16_LE : AFMT_U8;
		}
		printk("case SNDCTL_DSP_SETFMT, *IISCONFIG :0x%llx, state->output_stream->sample_size<<24: 0x%x\n", *IISCONFIG, (state->output_stream->sample_size<<24));
		*IISCONFIG = (*IISCONFIG&0x00ffffff)|(state->output_stream->sample_size<<24);

		return put_user(val, (int *) arg);

	case SNDCTL_DSP_POST:
		return 0;

	case SNDCTL_DSP_SETFRAGMENT:
		if (get_user(val, (int *)arg)) {
			return -EFAULT;
		}

		if (rd) {
			is->fragsize = 1 << (val & 0xFFFF);
			if (is->fragsize < 1024) is->fragsize = 1024;
			is->nbfrags = (val >> 16) & 0xFFFF;
			if (is->nbfrags < 4) is->nbfrags = 4;
		}

		if (wr) {
			os->fragsize = 1 << (val & 0xFFFF);
			if (os->fragsize < 1024) os->fragsize = 1024;
			os->nbfrags = (val >> 16) & 0xFFFF;
			if (os->nbfrags < 4) os->nbfrags = 4;
			if (os->num_channels) {
				os->nbfrags >>= 2;
				if (os->nbfrags < 2) os->nbfrags = 2;
			}
		}

		return 0;

	case SNDCTL_DSP_GETBLKSIZE:
		if (file->f_mode & FMODE_WRITE)
			return put_user(os->fragsize, (int *) arg);
		else
			return put_user(is->fragsize, (int *) arg);

	case SNDCTL_DSP_NONBLOCK:
		spin_lock(&file->f_lock);
		file->f_flags |= O_NONBLOCK;
		spin_unlock(&file->f_lock);
		return 0;

	case SOUND_PCM_READ_RATE:
		return put_user((file->f_mode & FMODE_READ) ?
				state->input_stream->sample_rate :
				state->output_stream->sample_rate,
				(int *)arg);

	case SOUND_PCM_READ_CHANNELS:
		if (file->f_mode & FMODE_READ)
			return put_user(state->input_stream->num_channels, (int *)arg);
		else
			return put_user(state->output_stream->num_channels, (int *)arg);

	case SOUND_PCM_READ_BITS:
		if (file->f_mode & FMODE_READ)
			return put_user(state->input_stream->sample_size, (int *)arg);
		else
			return put_user(state->output_stream->sample_size, (int *)arg);

	case SOUND_PCM_WRITE_FILTER:
	case SNDCTL_DSP_SETSYNCRO:
	case SOUND_PCM_READ_FILTER:
		return -EINVAL;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ls2k_audio_sync(struct file *file)
{
        struct ls2k_audio_state *state = file->private_data;
        struct audio_stream *is = state->input_stream;
        struct audio_stream *os = state->output_stream;

        if (file->f_mode & FMODE_READ) {
                if (is->state == STOP && !list_empty(&is->run_list)) {
                        struct audio_dma_desc *desc;
                        desc = list_entry(is->run_list.next, struct audio_dma_desc, link);
                        dma_enable_trans(is, desc);
                        is->state = RUN;
                }

                if (!list_empty(&is->run_list))
                        schedule_timeout(CONFIG_HZ*2);

                /* stop write ac97 dma */
                writel(0x10, order_addr_in1);
        }
        if (file->f_mode & FMODE_WRITE) {
                if (os->state == STOP && !list_empty(&os->run_list)) {
                        struct audio_dma_desc *desc;
                        desc = list_entry(os->run_list.next, struct audio_dma_desc, link);
                        dma_enable_trans(os, desc);
                        os->state = RUN;
                }

                if (!list_empty(&os->run_list))
                        schedule_timeout(CONFIG_HZ*2);

                /* stop read ac97 dma */
                writel(0x10, order_addr_in0);
        }

        return 0;
}

static int ls2k_audio_release(struct inode *inode, struct file *file)
{
        struct ls2k_audio_state *state = file->private_data;

	u64 value =  readq(order_addr_in0);
	value &= 0xfffffffffffffff0;
	u64 add = ioremap(value, 0x4);

        down(&state->sem);

        if (file->f_mode & FMODE_READ) {
                ls2k_audio_sync(file);
                audio_clear_buf(state->input_stream);
                state->rd_ref = 0;
                free_irq(irq1, state->input_stream);
        }

        if (file->f_mode & FMODE_WRITE) {
                ls2k_audio_sync(file);
                audio_clear_buf(state->output_stream);
                state->wr_ref = 0;
                free_irq(irq0, state->output_stream);
        }

        up(&state->sem);
        return 0;
}

static int ls2k_audio_open(struct inode *inode, struct file *file)
{
	struct ls2k_audio_state *state = &ls2k_audio_state;
	struct audio_stream *is = state->input_stream;
	struct audio_stream *os = state->output_stream;
	int minor = MINOR(inode->i_rdev);
	int err;
	u32 x, conf;
        uint var = 0;//test. read register
	u16 iface;
        u8 i;
        struct es8311_private *es8311 = es8311_data;
    	u32 value1 = readl(dma_cfg_addr0);
	writel((value1 & ~(7 << 4)) | (1 << 6), dma_cfg_addr0);
    
	down(&state->sem);
	/* access control */
	err = -ENODEV;
	if ((file->f_mode & FMODE_WRITE) && !os)
		goto out;
	if ((file->f_mode & FMODE_READ) && !is)
		goto out;
	err = -EBUSY;

	if ((file->f_mode & FMODE_WRITE) && state->wr_ref)
		goto out;
	if ((file->f_mode & FMODE_READ) && state->rd_ref)
		goto out;

	file->private_data = state;

	if ((file->f_mode & FMODE_WRITE)) {
        	* IISSTATE = 0x0d280;
		state->wr_ref = 1;
		os->fragsize = AUDIO_FRAGSIZE_DEFAULT;
		os->nbfrags = PLAY_NBFRAGS;
		os->output = 1;
		os->num_channels = 2;
		os->sample_rate= 44;
		os->sample_size = 16;
		if ((minor & 0xf) == SND_DEV_DSP16)
			os->sample_size = 16;
		init_waitqueue_head(&os->frag_wq);
		os->ask_dma = NULL;
		INIT_LIST_HEAD(&os->free_list);
		INIT_LIST_HEAD(&os->run_list);
		INIT_LIST_HEAD(&os->done_list);
		INIT_LIST_HEAD(&os->all_list);
		spin_lock_init(&os->lock);
		request_irq(irq0, ac97_dma_write_intr, IRQF_SHARED,
				"ac97dma-write", os);
        
		iface = es8311_soc_component_read32(es8311->regmap, ES8311_SDPIN_REG09) & 0xE3;
		/* bit size */
        /*
		switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S16_LE:
			iface |= 0x0c;
			break;
		case SNDRV_PCM_FORMAT_S20_3LE:
			iface |= 0x04;
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
			break;
		case SNDRV_PCM_FORMAT_S32_LE:
			iface |= 0x10;
			break;
		}*/
		//if(os->sample_size == 16)//mgl add
		    iface |= 0x0c;
		/* set iface */
		regmap_write(es8311->regmap, ES8311_SDPIN_REG09, iface);
	}

	if (file->f_mode & FMODE_READ) {
		* IISSTATE = 0x0e800;
		state->rd_ref = 1;
		is->fragsize = AUDIO_FRAGSIZE_DEFAULT;
		is->nbfrags = REC_NBFRAGS;
		is->output = 0;
		is->num_channels = 2;
		is->sample_rate= 44;
		is->sample_size = 16;
		if ((minor & 0xf) == SND_DEV_DSP16)
			is->sample_size = 16;
		init_waitqueue_head(&is->frag_wq);
		is->ask_dma = NULL;
		INIT_LIST_HEAD(&is->free_list);
		INIT_LIST_HEAD(&is->run_list);
		INIT_LIST_HEAD(&is->done_list);
		INIT_LIST_HEAD(&is->all_list);
		spin_lock_init(&is->lock);
		request_irq(irq1, ac97_dma_read_intr, IRQF_SHARED,
				"ac97dma-read", is);
        
		iface = es8311_soc_component_read32(es8311->regmap, ES8311_SDPOUT_REG0A) & 0xE3;
		/* bit size */
		//if(os->sample_size == 16)//mgl add
		    iface |= 0x0c;
		/* set iface */
		regmap_write(es8311->regmap, ES8311_SDPOUT_REG0A, iface);
	}
	if ((file->f_mode & FMODE_WRITE)) {
        	* IISSTATE = 0x0d280;
                config_es8311(os, es8311);
       }
        else {
		* IISSTATE = 0x0e800;
                config_es8311(is, es8311);
            }

        es8311_mute(es8311->regmap, 0);
        es8311_set_level(es8311, SND_SOC_BIAS_ON);//test
    /*
        for (i = 0; i<0x46; i++){
            regmap_read(es8311->regmap, i, &var);//test. read register
            printk("REG0x%x: 0x%x. \n",i, var);
            }
            regmap_read(es8311->regmap, 0XFA, &var);//test. read register
            printk("REG0XFA: 0x%x. \n", var);
        
            regmap_read(es8311->regmap, 0XFC, &var);//test. read register
            printk("REG0XFC: 0x%x. \n", var);
            regmap_read(es8311->regmap, 0XFD, &var);//test. read register
            printk("REG0XFD: 0x%x. \n", var);
            regmap_read(es8311->regmap, 0XFE, &var);//test. read register
            printk("REG0XFE: 0x%x. \n", var);
    */
	err = 0;

out:
	up(&state->sem);
	return err;
}

static long ls2k_ioctl_mixdev(struct file *file, unsigned int cmd, unsigned long arg)
{
        //struct ls2k_audio_state *state = (struct ls2k_audio_state *)file->private_data;
        //struct ac97_codec *codec = state->codec;
        struct es8311_private *es8311 = (struct es8311_private *)file->private_data;
	int ret;
	int val = 0;
    uint temp, var = 0;
	unsigned char data;

	/* We must snoop for some commands to provide our own extra processing */
	switch (cmd) {
		case SOUND_MIXER_READ_STEREODEVS:
		case SOUND_MIXER_READ_DEVMASK:
			val = (SOUND_MASK_MIC | SOUND_MASK_VOLUME);
			put_user(val, (int *)arg);
			break;
		case SOUND_MIXER_WRITE_PCM:
		case SOUND_MIXER_WRITE_VOLUME:
			if (get_user(var, (uint*)arg)) {
				return -EFAULT;
			}
                        temp = (var&0xff00)>>8;
        		var = var&0x00ff;
                        regmap_write(es8311->regmap, ES8311_ADC_REG17, temp);
                        regmap_write(es8311->regmap, ES8311_DAC_REG32, var);
			break;

		case SOUND_MIXER_READ_PCM:
		case SOUND_MIXER_READ_VOLUME:
			regmap_read(es8311->regmap, ES8311_ADC_REG17, &var);
                        temp = var<<8 &&0xff00;
			regmap_read(es8311->regmap, ES8311_DAC_REG32, &var);
                        temp = temp |(var&0x00ff);
                        var = temp;
			return put_user(var, (long*)arg);		
			break;
		case SOUND_MIXER_WRITE_MUTE:
			if (get_user(val, (int*)arg)) {
				return -EFAULT;
			}
			regmap_read(es8311->regmap, 0, &data);
			if(val)
				data &= ~(2<<8);
			else
				data |= (2<<8);
			regmap_write(es8311->regmap, 0, data);		
			break;
		case SOUND_MIXER_WRITE_MIC:
			if (get_user(val, (int*)arg)) {
				return -EFAULT;
			}
			val =(((100-(val&0xff))*256/100))|(((100-(((val&0xff00)>>8)))*256/100)<<8) ;
			regmap_write(es8311->regmap, 0x12, val);		
			break;
		case SOUND_MIXER_READ_MIC:
			regmap_read(es8311->regmap, 0x12, &val);
			val = (((256-(val&0xff))*100/256))|(((256-((val&0xff00)>>8))*100/256)<<8);
			return put_user(val, (long*)arg);		
			break;
		case SOUND_MIXER_WRITE_RECSRC:
		case SOUND_MIXER_WRITE_IGAIN:
		default:
			break;
	}

        return 0;
}


static int ls2k_open_mixdev(struct inode *inode, struct file *file)
{

        file->private_data = es8311_data;
        return 0;
}

static int ls2k_release_mixdev(struct inode *inode, struct file *file)
{
        return 0;
}

static struct file_operations ls2k_dsp_fops = {
	owner:			THIS_MODULE,
	read:			ls2k_audio_read,
	write:			ls2k_audio_write,
	unlocked_ioctl:	ls2k_audio_ioctl,
	open:			ls2k_audio_open,
	release:		ls2k_audio_release,
};

static struct file_operations ls2k_mixer_fops = {
        .owner          = THIS_MODULE,
        .unlocked_ioctl = ls2k_ioctl_mixdev,
        .open           = ls2k_open_mixdev,
        .release        = ls2k_release_mixdev,
};

static DEFINE_SEMAPHORE(ls2k_ac97_mutex);

static int codec_i2c_init();
static int ls2k_audio_probe(struct platform_device *pdev)
{
	struct ls2k_audio_state *state = &ls2k_audio_state;
	struct resource *res;
	int rc;
	unsigned int ls2k_apbdma_cfg;
	int ls2k_apbdma_sel;
#ifdef CONFIG_OF
	struct dma_chan *chan;
	int data;
	__be32 *dma_mask_p = NULL;

	if (pdev->dev.of_node) {
		dma_mask_p = (__be32 *)of_get_property(pdev->dev.of_node, "dma-mask", NULL);
		if (dma_mask_p != 0){
			pdev->dev.coherent_dma_mask = of_read_number(dma_mask_p,2);
			if (pdev->dev.dma_mask)
				*(pdev->dev.dma_mask) = pdev->dev.coherent_dma_mask;
			else
				pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
		}
	}
#endif


	res =  platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "cannot find IO resource\n");
		return -ENOENT;
	}

	if (!request_mem_region(res->start, resource_size(res), "ls2k-audio")){
		printk("request mem error\n");
		return -EBUSY;
	}

	state->base = ioremap(res->start, resource_size(res));
	if (!state->base) {
		dev_err(&pdev->dev, "ls2k-audio - failed to map controller\n");
		printk("error iomap\n");
		rc = -ENOMEM;
		goto err_dev0;
	}

	dma_cfg_addr0 = DMA_CFG_ADDR0;
	int_sta_addr = INT_STA_ADDR;
	int_cfg_addr = INT_CFG_ADDR;
	memset(state, 0, sizeof(struct ls2k_audio_state));

	state->input_stream = &input_stream;
	state->output_stream = &output_stream;
	sema_init(&state->sem, 1);

#ifdef CONFIG_OF
	chan = dma_request_slave_channel(&pdev->dev,"i2s_play");
	if(chan == NULL){
		dev_err(&pdev->dev, "no APBDMA 2 resource defined\n");
		return -ENOENT;
	}   
	of_property_read_u32(chan->device->dev->of_node, "reg", &data);
	res->start = data;
#else
	res = platform_get_resource(pdev, IORESOURCE_DMA , 0);
	if (res == NULL) {
	    dev_err(&pdev->dev, "no DMA 2 resource defined\n");
	    return -ENOENT;
	}
#endif
	order_addr_in0 = res->start;

	ls2k_apbdma_cfg = ls2k_readl(LS2K_APBDMA_CFG_REG);
	ls2k_apbdma_cfg &=  ~(LS2K_APBDMA_MASK << LS2K_I2SS_DMA_SHIFT);
	ls2k_apbdma_sel = (res->start - LS2K_DMA0_REG) >> 4;
	ls2k_apbdma_cfg |= (ls2k_apbdma_sel & LS2K_APBDMA_MASK) << LS2K_I2SS_DMA_SHIFT;
	ls2k_writel(ls2k_apbdma_cfg,LS2K_APBDMA_CFG_REG);

#ifdef CONFIG_OF
	chan = dma_request_slave_channel(&pdev->dev,"i2s_record");
	if(chan == NULL){
		dev_err(&pdev->dev, "no APBDMA 3 resource defined\n");
		return -ENOENT;
	}   
	of_property_read_u32(chan->device->dev->of_node, "reg", &data);
	res->start = data;
#else
	res = platform_get_resource(pdev, IORESOURCE_DMA , 1);
	if (res == NULL) {
	    dev_err(&pdev->dev, "no DMA 3 resource defined\n");
	    return -ENOENT;
	}
#endif
	order_addr_in1 = res->start;

	ls2k_apbdma_cfg = ls2k_readl(LS2K_APBDMA_CFG_REG);
	ls2k_apbdma_cfg &=  ~(LS2K_APBDMA_MASK << LS2K_I2SR_DMA_SHIFT);
	ls2k_apbdma_sel = (res->start - LS2K_DMA0_REG) >> 4;
	ls2k_apbdma_cfg |= (ls2k_apbdma_sel & LS2K_APBDMA_MASK) << LS2K_I2SR_DMA_SHIFT;
	ls2k_writel(ls2k_apbdma_cfg,LS2K_APBDMA_CFG_REG);

	irq0 = platform_get_irq(pdev, 0);
	if (irq0 == 0) {
	    dev_err(&pdev->dev, "failed to get interrupt 2 resouce.\n");
	    return -ENOENT;
	}

	irq1 = platform_get_irq(pdev, 1);
	if (irq1 == 0) {
	    dev_err(&pdev->dev, "failed to get interrupt 3 resouce.\n");
	    return -ENOENT;
	}

	if ((state->dev_audio = register_sound_dsp(&ls2k_dsp_fops, -1)) < 0){
		printk("error!\n");
		goto err_dev1;
	}
	if ((state->dev_mixer = register_sound_mixer(&ls2k_mixer_fops, -1)) < 0){
		printk("error !\n");
		goto err_dev1;
	}

	if(codec_i2c_init())
		goto err_dev3;

	printk("register mixer success\n");
	printk("register dsp success\n");
	printk("SOUND_MIXER_WRITE_VOLUME is %x\n",SOUND_MIXER_WRITE_VOLUME);
	return 0;

err_dev3:
	unregister_sound_mixer(state->codec->dev_mixer);
err_dev2:
	unregister_sound_dsp(state->dev_audio);
err_dev1:
	iounmap(state->base);
	release_mem_region(res->start, resource_size(res));
err_dev0:
	return rc;
}

static const struct i2c_device_id codec_ids[] = {
   { "es8311,ls-pcm-audio", 0 },//codec_uda1342
   { /* END OF LIST */ }
};

MODULE_DEVICE_TABLE(i2c, codec_ids);

static int es8311_suspend(struct i2c_client *client)
{
	struct es8311_private *es8311 = i2c_get_clientdata(client);
	printk("Enter into %s()\n", __func__);
	regmap_write(es8311->regmap, ES8311_DAC_REG32, 0x00);
	regmap_write(es8311->regmap, ES8311_ADC_REG17, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0E, 0xFF);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG12, 0x02);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG14, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0D, 0xFA);
	regmap_write(es8311->regmap, ES8311_ADC_REG15, 0x00);
	regmap_write(es8311->regmap, ES8311_DAC_REG37, 0x08);
	regmap_write(es8311->regmap, ES8311_RESET_REG00, 0x00);
	regmap_write(es8311->regmap, ES8311_RESET_REG00, 0x1F);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x30);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x00);
	regmap_write(es8311->regmap, ES8311_GP_REG45, 0x01);
	return 0;
}

static int es8311_resume(struct i2c_client *client)
{

	struct es8311_private *es8311 = i2c_get_clientdata(client);
	printk("Enter into %s()\n", __func__);
	regmap_write(es8311->regmap, ES8311_GP_REG45, 0x00);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x30);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG02, 0x00);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG03, 0x10);
	regmap_write(es8311->regmap, ES8311_ADC_REG16, 0x24);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG04, 0x10);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG05, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0B, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0C, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG10, 0x1F);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG11, 0x7F);
	regmap_write(es8311->regmap, ES8311_RESET_REG00, 0x80);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0D, 0x01);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x3F);
	if(es8311->mclkinv == true) {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG01,
				0x40, 0x40);
	}
	else {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG01,
				0x40, 0x00);
	}
	if(es8311->sclkinv == true) {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG06,
				0x20, 0x20);
	}
	else {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG06,
				0x20, 0x00);
	}
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG14, 0x1A);
	if(es8311->dmic_enable == true) {
		regmap_bits_update(es8311->regmap, ES8311_SYSTEM_REG14,
				0x40, 0x40);
	}
	else {
		regmap_bits_update(es8311->regmap, ES8311_SYSTEM_REG14,
				0x40, 0x00);
	}
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG12, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG13, 0x10);
	regmap_write(es8311->regmap, ES8311_SDPIN_REG09, 0x00);
	regmap_write(es8311->regmap, ES8311_SDPOUT_REG0A, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0E, 0x02);
	regmap_write(es8311->regmap, ES8311_ADC_REG15, 0x40);
	regmap_write(es8311->regmap, ES8311_ADC_REG1B, 0x0A);
	regmap_write(es8311->regmap, ES8311_ADC_REG1C, 0x6A);
	regmap_write(es8311->regmap, ES8311_DAC_REG37, 0x48);
	regmap_write(es8311->regmap, ES8311_GPIO_REG44, 0x08);
	regmap_write(es8311->regmap, ES8311_ADC_REG17, 0xBF);
	regmap_write(es8311->regmap, ES8311_DAC_REG32, 0xBF);

	return 0;
}

static int es8311_probe(struct es8311_private *private)
{
	int ret = 0;
	struct es8311_private *es8311 = private;
	printk("Enter into %s()\n", __func__);
	//ret = snd_soc_component_set_cache_io(codec, 8, 8, 1);
	//if (ret < 0)
	//	return ret;

	regmap_write(es8311->regmap, ES8311_GP_REG45, 0x00);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x30);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG02, 0x00);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG03, 0x10);
	regmap_write(es8311->regmap, ES8311_ADC_REG16, 0x24);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG04, 0x10);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG05, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0B, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0C, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG10, 0x1F);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG11, 0x7F);
	regmap_write(es8311->regmap, ES8311_RESET_REG00, 0x80);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0D, 0x01);
	regmap_write(es8311->regmap, ES8311_CLK_MANAGER_REG01, 0x3F);
	if(es8311->mclkinv == true) {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG01, 
				0x40, 0x40);
	}
	else {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG01, 
				0x40, 0x00);
	}
	if(es8311->sclkinv == true) {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG06,
				0x20, 0x20); 
	}
	else {
		regmap_bits_update(es8311->regmap, ES8311_CLK_MANAGER_REG06,
				0x20, 0x00); 
	}
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG14, 0x1A);
	if(es8311->dmic_enable == true) {
		regmap_bits_update(es8311->regmap, ES8311_SYSTEM_REG14,
				0x40, 0x40);
	}
	else {
		regmap_bits_update(es8311->regmap, ES8311_SYSTEM_REG14,
				0x40, 0x00);
	} 
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG12, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG13, 0x10);
	regmap_write(es8311->regmap, ES8311_SDPIN_REG09, 0x00);
	regmap_write(es8311->regmap, ES8311_SDPOUT_REG0A, 0x00);
	regmap_write(es8311->regmap, ES8311_SYSTEM_REG0E, 0x02);
	regmap_write(es8311->regmap, ES8311_ADC_REG15, 0x40);
	regmap_write(es8311->regmap, ES8311_ADC_REG1B, 0x0A);
	regmap_write(es8311->regmap, ES8311_ADC_REG1C, 0x6A);
	regmap_write(es8311->regmap, ES8311_DAC_REG37, 0x48);
	regmap_write(es8311->regmap, ES8311_GPIO_REG44, 0x08);
	regmap_write(es8311->regmap, ES8311_ADC_REG17, 0xBF);
	regmap_write(es8311->regmap, ES8311_DAC_REG32, 0xBF);
	
	msleep(100);
	es8311_set_level(es8311, SND_SOC_BIAS_STANDBY);
	return ret;
}

static void es8311_remove(struct es8311_private *private)
{
        struct es8311_private *es8311 = private;
	es8311_set_level(es8311, SND_SOC_BIAS_OFF);
	
}

static int codec_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct es8311_private *es8311;
	int ret = -1;
	printk("Enter into %s\n", __func__);
	es8311 = devm_kzalloc(&client->dev, 
			sizeof(*es8311), GFP_KERNEL);
	if (es8311 == NULL)
		return -ENOMEM;

	printk("Enter into %s---1\n", __func__);
	es8311->dmic_enable = false;     // dmic interface disabled
	/* the edge of lrck is always at the falling edge of mclk */
	es8311->mclkinv = false; 
        /* the edge of lrck is always at the falling edge of sclk */         
	es8311->sclkinv = false; 

	printk("Enter into %s----2\n", __func__);
	i2c_set_clientdata(client, es8311);
	es8311->regmap = devm_regmap_init_i2c(client, &es8311_regmap);
	if (IS_ERR(es8311->regmap)) {
		ret = PTR_ERR(es8311->regmap);
		dev_err(&client->dev, "regmap_init() failed: %d\n", ret);
		return ret;
	}
	//dev_set_drvdata(&client->dev,  es8311);
        es8311_probe(es8311);
	es8311_data = es8311;
	return 0;
}

static int codec_remove(struct i2c_client *client)
{
	struct es8311_private *es8311 = i2c_get_clientdata(client);
        es8311_remove(es8311);
	i2c_unregister_device(client);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id ls_codec_dt_match[] = {
	{ .compatible = "es8311,ls-pcm-audio", },//mgl,codec_uda1342
	{},
};
MODULE_DEVICE_TABLE(of, ls_codec_dt_match);
#endif

static struct i2c_driver eep_driver = {
	.driver = {
		.name = "codec-edid",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ls_codec_dt_match),
#endif
	},
	.probe = codec_probe,
	.remove = codec_remove,
	.suspend = es8311_suspend,
	.resume = es8311_resume,
	.id_table = codec_ids,
};

static int i2c_read_codec(struct i2c_client *client, unsigned char reg, unsigned char *val)
{
	unsigned char start = reg;
	struct i2c_msg msgs[] = {
	    {
		.addr   = client->addr,
		.flags  = 0,
		.len    = 1,
		.buf    = &start,
	    }, {
		.addr   = client->addr,
		.flags  = I2C_M_RD,
		.len    = 0x1,
		.buf    = val,
	    }
	};

	if (i2c_transfer(client->adapter, msgs, 2) == 2) {
		return 0;
	}
	return -1;
}
static int i2c_write_codec(struct i2c_client *client, unsigned char reg, unsigned char val)
{
	unsigned char msg[2] = {reg, val};
	struct i2c_msg msgs[] = {
	    {
		.addr   = client->addr,
		.flags  = 0,
		.len    = 2,
		.buf    = &msg[0],
	    }
	};

	if (i2c_transfer(client->adapter, msgs, 1) == 1) {
		return 0;
	}
	return -1;
}

static int codec_i2c_init()
{
	if (i2c_add_driver(&eep_driver)) {
		pr_err("i2c-%ld No eeprom device register!",eep_driver.id_table->driver_data);
		return -1;
	}

	return 0;
}

static int ls2k_audio_remove(struct platform_device *pdev)
{
	struct ls2k_audio_state *state = &ls2k_audio_state;
	struct resource *res;

	unregister_sound_dsp(state->dev_audio);
	unregister_sound_mixer(state->codec->dev_mixer);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	iounmap(state->base);
	release_mem_region(res->start, resource_size(res));

	return 0;
}

#ifdef CONFIG_PM
static int ls2k_audio_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int ls2k_audio_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define ls2k_audio_suspend NULL
#define ls2k_audio_resume NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id ls_audio_dt_match[] = { 
    { .compatible = "loongson,ls-pcm-audio-es8311", },
    {}, 
};
MODULE_DEVICE_TABLE(of, ls_audio_dt_match);
#endif

static struct platform_driver ls2k_audio_driver = {
	.driver = {
		.name	= "ls2k-audio",
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ls_audio_dt_match),
#endif	
	},
	.probe		= ls2k_audio_probe,
	.remove		= ls2k_audio_remove,
	.suspend	= ls2k_audio_suspend,
	.resume		= ls2k_audio_resume,
};

static int __init ls2k_audio_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&ls2k_audio_driver);
	if (ret)
		printk(KERN_ERR "failed to register ls2k-audio\n");
	return ret;
}

static void __exit ls2k_audio_exit(void)
{
    platform_driver_unregister(&ls2k_audio_driver);
}

module_init(ls2k_audio_init);
module_exit(ls2k_audio_exit);

#ifndef MODULE

static int __init
ls2k_setup(char *options)
{
	char           *this_opt;

	if (!options || !*options)
		return 0;

	while ((this_opt = strsep(&options, ","))) {
		if (!*this_opt)
			continue;
		if (!strncmp(this_opt, "vra", 3)) {
			vra = 1;
		}
	}

	return 1;
}

__setup("ls2k_audio=", ls2k_setup);

#endif /* MODULE */

MODULE_LICENSE("GPL");

