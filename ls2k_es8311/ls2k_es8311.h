/*
* LS2K_ES8311.h  --  ES8311 ALSA SoC Audio Codec
*
* Authors:
*
* Based on ES8374.h by David Yang
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#ifndef _LS2K_ES8311_H
#define _LS2K_ES8311_H

#include <ls2k.h>

#include <linux/types.h>
#include <linux/soundcard.h>
/* Control Status Register (CSR) */
#define CSR		0x00
#define AC97_CSR_RESUME		(1 << 1)
#define AC97_CSR_RST_FORCE	(1 << 0)
/* Output Channel Configuration Registers (OCCn) */
#define OCC0	0x04
#define OCC1	0x08
#define OCC2	0x0c
#define OCH1_CFG_R_OFFSET	(8)
#define OCH0_CFG_L_OFFSET	(0)
#define OCH1_CFG_R_MASK		(0xFF)
#define OCH0_CFG_L_MASK		(0xFF)

/* Input Channel Configuration (ICC) */
#define ICC		0x10
#define ICH2_CFG_MIC_OFFSET	(16)
#define ICH1_CFG_R_OFFSET	(8)
#define ICH0_CFG_L_OFFSET	(0)
#define ICH2_CFG_MIC_MASK	(0xFF)
#define ICH1_CFG_R_MASK		(0xFF)
#define ICH0_CFG_L_MASK		(0xFF)

/* Channel Configurations (Sub-field) */
#define DMA_EN	(1 << 6)
#define FIFO_THRES_OFFSET	(4)
#define FIFO_THRES_MASK		(0x3)
#define SS_OFFSET		(2)	/* Sample Size 00:8bit 10:16bit */
#define SS_MASK		(0x3)
#define SR		(1 << 1)	/* Sample Rate 1:Variable 0:Fixed */
#define CH_EN	(1 << 0)


#define CODECID	0x14

/* Codec Register Access Command (CRAC) */
#define CRAC	0x18
#define CODEC_WR			(1 << 31)
#define CODEC_ADR_OFFSET	(16)
#define CODEC_DAT_OFFSET	(0)
#define CODEC_ADR_MASK		(0xFF)
#define CODEC_DAT_MASK		(0xFF)

/* OCHn and ICHn Registers
   OCHn are the output fifos (data that will be send to the codec), ICHn are the
   input fifos (data received from the codec).
 */
#define OCH0	0x20
#define OCH1	0x24
#define OCH2	0x28
#define OCH3	0x2c
#define OCH4	0x30
#define OCH5	0x34
#define OCH6	0x38
#define OCH7	0x3c
#define OCH8	0x40
#define ICH0	0x44
#define ICH1	0x48
#define ICH2	0x4c

/* Interrupt Status Register (INTS) */
#define INTRAW	0x54
#define ICH_FULL		(1 << 31)
#define ICH_TH_INT		(1 << 30)
#define OCH1_FULL		(1 << 7)
#define OCH1_EMPTY		(1 << 6)
#define OCH1_TH_INT		(1 << 5)
#define OCH0_FULL		(1 << 4)
#define OCH0_EMPTY		(1 << 3)
#define OCH0_TH_INT		(1 << 2)
#define CW_DONE		(1 << 1)
#define CR_DONE		(1 << 0)

#define INTM	0x58

#define INT_CLR	0x5c
#define INT_OCCLR	0x60
#define INT_ICCLR	0x64
#define INT_CWCLR	0x68
#define INT_CRCLR	0x6c

#define ORDER_ADDR_IN0	0x1fe10c00
#define ORDER_ADDR_IN1	0x1fe10c10
#define DMA_CFG_ADDR0	0x1fe10420
#define DMA_CFG_ADDR1	0x1fe10438
#define INT_STA_ADDR	0x1fe11464
#define INT_CFG_ADDR	0x1fe11468

/* dma regs */
#define DMA_ORDERED		0x0
#define DMA_SADDR		0x1
#define DMA_DADDR		0x2
#define DMA_LENGTH		0x3
#define DMA_STEP_LENGTH		0x4
#define DMA_STEP_TIMES		0x5
#define	DMA_CMD			0x6

#define DMA_ORDERED_EN	(1 << 0)

#define DMA_AC97_WRITE_EN	(0x1 << 31)
#define DMA_AC97_STERO	(0x1 << 30)
#define DMA_AC97_WRITE_MODE	(0x3 << 28)
#define DMA_AC97_WRITE_MODE_SHIFT 28
#define DMA_AC97_WRITE_1B	0
#define DMA_AC97_WRITE_2B	1
#define DMA_AC97_WRITE_4B	2

#define AC97_EXT_DACS (AC97_EXTID_SDAC | AC97_EXTID_CDAC | AC97_EXTID_LDAC)

#define DMA_DESC		0x5
#define REC_NBFRAGS		DMA_DESC
#define PLAY_NBFRAGS	DMA_DESC
#define AUDIO_FRAGSIZE_DEFAULT	0x2000
/* AC97 1.0 */
#define  AC97_RESET               0x0000
#define  AC97_MASTER_VOL_STEREO   0x0002
#define  AC97_HEADPHONE_VOL       0x0004
#define  AC97_MASTER_VOL_MONO     0x0006
#define  AC97_MASTER_TONE         0x0008
#define  AC97_PCBEEP_VOL          0x000a
#define  AC97_PHONE_VOL           0x000c
#define  AC97_MIC_VOL             0x000e
#define  AC97_LINEIN_VOL          0x0010
#define  AC97_CD_VOL              0x0012
#define  AC97_VIDEO_VOL           0x0014
#define  AC97_AUX_VOL             0x0016
#define  AC97_PCMOUT_VOL          0x0018
#define  AC97_RECORD_SELECT       0x001a
#define  AC97_RECORD_GAIN         0x001c
#define  AC97_RECORD_GAIN_MIC     0x001e
#define  AC97_GENERAL_PURPOSE     0x0020
#define  AC97_3D_CONTROL          0x0022
#define  AC97_MODEM_RATE          0x0024
#define  AC97_POWER_CONTROL       0x0026

/* AC'97 2.0 */
#define AC97_EXTENDED_ID          0x0028       /* Extended Audio ID */
#define AC97_EXTENDED_STATUS      0x002A       /* Extended Audio Status */
#define AC97_PCM_FRONT_DAC_RATE   0x002C       /* PCM Front DAC Rate */
#define AC97_PCM_SURR_DAC_RATE    0x002E       /* PCM Surround DAC Rate */
#define AC97_PCM_LFE_DAC_RATE     0x0030       /* PCM LFE DAC Rate */
#define AC97_PCM_LR_ADC_RATE      0x0032       /* PCM LR ADC Rate */
#define AC97_PCM_MIC_ADC_RATE     0x0034       /* PCM MIC ADC Rate */
#define AC97_CENTER_LFE_MASTER    0x0036       /* Center + LFE Master Volume */
#define AC97_SURROUND_MASTER      0x0038       /* Surround (Rear) Master Volume */
#define AC97_RESERVED_3A          0x003A       /* Reserved in AC '97 < 2.2 */

/* AC'97 2.2 */
#define AC97_SPDIF_CONTROL        0x003A       /* S/PDIF Control */

/* range 0x3c-0x58 - MODEM */
#define AC97_EXTENDED_MODEM_ID    0x003C
#define AC97_EXTEND_MODEM_STAT    0x003E
#define AC97_LINE1_RATE           0x0040
#define AC97_LINE2_RATE           0x0042
#define AC97_HANDSET_RATE         0x0044
#define AC97_LINE1_LEVEL          0x0046
#define AC97_LINE2_LEVEL          0x0048
#define AC97_HANDSET_LEVEL        0x004A
#define AC97_GPIO_CONFIG          0x004C
#define AC97_GPIO_POLARITY        0x004E
#define AC97_GPIO_STICKY          0x0050
#define AC97_GPIO_WAKE_UP         0x0052
#define AC97_GPIO_STATUS          0x0054
#define AC97_MISC_MODEM_STAT      0x0056
#define AC97_RESERVED_58          0x0058

/* registers 0x005a - 0x007a are vendor reserved */

#define AC97_VENDOR_ID1           0x007c
#define AC97_VENDOR_ID2           0x007e

/* volume control bit defines */
#define AC97_MUTE                 0x8000
#define AC97_MICBOOST             0x0040
#define AC97_LEFTVOL              0x3f00
#define AC97_RIGHTVOL             0x003f

/* record mux defines */
#define AC97_RECMUX_MIC           0x0000
#define AC97_RECMUX_CD            0x0101
#define AC97_RECMUX_VIDEO         0x0202
#define AC97_RECMUX_AUX           0x0303
#define AC97_RECMUX_LINE          0x0404
#define AC97_RECMUX_STEREO_MIX    0x0505
#define AC97_RECMUX_MONO_MIX      0x0606
#define AC97_RECMUX_PHONE         0x0707

/* general purpose register bit defines */
#define AC97_GP_LPBK              0x0080       /* Loopback mode */
#define AC97_GP_MS                0x0100       /* Mic Select 0=Mic1, 1=Mic2 */
#define AC97_GP_MIX               0x0200       /* Mono output select 0=Mix, 1=Mic */
#define AC97_GP_RLBK              0x0400       /* Remote Loopback - Modem line codec */
#define AC97_GP_LLBK              0x0800       /* Local Loopback - Modem Line codec */
#define AC97_GP_LD                0x1000       /* Loudness 1=on */
#define AC97_GP_3D                0x2000       /* 3D Enhancement 1=on */
#define AC97_GP_ST                0x4000       /* Stereo Enhancement 1=on */
#define AC97_GP_POP               0x8000       /* Pcm Out Path, 0=pre 3D, 1=post 3D */

/* extended audio status and control bit defines */
#define AC97_EA_VRA               0x0001       /* Variable bit rate enable bit */
#define AC97_EA_DRA               0x0002       /* Double-rate audio enable bit */
#define AC97_EA_SPDIF             0x0004       /* S/PDIF Enable bit */
#define AC97_EA_VRM               0x0008       /* Variable bit rate for MIC enable bit */
#define AC97_EA_CDAC              0x0040       /* PCM Center DAC is ready (Read only) */
#define AC97_EA_SDAC              0x0040       /* PCM Surround DACs are ready (Read only) */
#define AC97_EA_LDAC              0x0080       /* PCM LFE DAC is ready (Read only) */
#define AC97_EA_MDAC              0x0100       /* MIC ADC is ready (Read only) */
#define AC97_EA_SPCV              0x0400       /* S/PDIF configuration valid (Read only) */
#define AC97_EA_PRI               0x0800       /* Turns the PCM Center DAC off */
#define AC97_EA_PRJ               0x1000       /* Turns the PCM Surround DACs off */
#define AC97_EA_PRK               0x2000       /* Turns the PCM LFE DAC off */
#define AC97_EA_PRL               0x4000       /* Turns the MIC ADC off */
#define AC97_EA_SLOT_MASK         0xffcf       /* Mask for slot assignment bits */
#define AC97_EA_SPSA_3_4          0x0000       /* Slot assigned to 3 & 4 */
#define AC97_EA_SPSA_7_8          0x0010       /* Slot assigned to 7 & 8 */
#define AC97_EA_SPSA_6_9          0x0020       /* Slot assigned to 6 & 9 */
#define AC97_EA_SPSA_10_11        0x0030       /* Slot assigned to 10 & 11 */

/* S/PDIF control bit defines */
#define AC97_SC_PRO               0x0001       /* Professional status */
#define AC97_SC_NAUDIO            0x0002       /* Non audio stream */
#define AC97_SC_COPY              0x0004       /* Copyright status */
#define AC97_SC_PRE               0x0008       /* Preemphasis status */
#define AC97_SC_CC_MASK           0x07f0       /* Category Code mask */
#define AC97_SC_L                 0x0800       /* Generation Level status */
#define AC97_SC_SPSR_MASK         0xcfff       /* S/PDIF Sample Rate bits */
#define AC97_SC_SPSR_44K          0x0000       /* Use 44.1kHz Sample rate */
#define AC97_SC_SPSR_48K          0x2000       /* Use 48kHz Sample rate */
#define AC97_SC_SPSR_32K          0x3000       /* Use 32kHz Sample rate */
#define AC97_SC_DRS               0x4000       /* Double Rate S/PDIF */
#define AC97_SC_V                 0x8000       /* Validity status */

/* powerdown control and status bit defines */

/* status */
#define AC97_PWR_MDM              0x0010       /* Modem section ready */
#define AC97_PWR_REF              0x0008       /* Vref nominal */
#define AC97_PWR_ANL              0x0004       /* Analog section ready */
#define AC97_PWR_DAC              0x0002       /* DAC section ready */
#define AC97_PWR_ADC              0x0001       /* ADC section ready */

/* control */
#define AC97_PWR_PR0              0x0100       /* ADC and Mux powerdown */
#define AC97_PWR_PR1              0x0200       /* DAC powerdown */
#define AC97_PWR_PR2              0x0400       /* Output mixer powerdown (Vref on) */
#define AC97_PWR_PR3              0x0800       /* Output mixer powerdown (Vref off) */
#define AC97_PWR_PR4              0x1000       /* AC-link powerdown */
#define AC97_PWR_PR5              0x2000       /* Internal Clk disable */
#define AC97_PWR_PR6              0x4000       /* HP amp powerdown */
#define AC97_PWR_PR7              0x8000       /* Modem off - if supported */

/* extended audio ID register bit defines */
#define AC97_EXTID_VRA            0x0001
#define AC97_EXTID_DRA            0x0002
#define AC97_EXTID_SPDIF          0x0004
#define AC97_EXTID_VRM            0x0008
#define AC97_EXTID_DSA0           0x0010
#define AC97_EXTID_DSA1           0x0020
#define AC97_EXTID_CDAC           0x0040
#define AC97_EXTID_SDAC           0x0080
#define AC97_EXTID_LDAC           0x0100
#define AC97_EXTID_AMAP           0x0200
#define AC97_EXTID_REV0           0x0400
#define AC97_EXTID_REV1           0x0800
#define AC97_EXTID_ID0            0x4000
#define AC97_EXTID_ID1            0x8000

/* extended status register bit defines */
#define AC97_EXTSTAT_VRA          0x0001
#define AC97_EXTSTAT_DRA          0x0002
#define AC97_EXTSTAT_SPDIF        0x0004
#define AC97_EXTSTAT_VRM          0x0008
#define AC97_EXTSTAT_SPSA0        0x0010
#define AC97_EXTSTAT_SPSA1        0x0020
#define AC97_EXTSTAT_CDAC         0x0040
#define AC97_EXTSTAT_SDAC         0x0080
#define AC97_EXTSTAT_LDAC         0x0100
#define AC97_EXTSTAT_MADC         0x0200
#define AC97_EXTSTAT_SPCV         0x0400
#define AC97_EXTSTAT_PRI          0x0800
#define AC97_EXTSTAT_PRJ          0x1000
#define AC97_EXTSTAT_PRK          0x2000
#define AC97_EXTSTAT_PRL          0x4000

/* extended audio ID register bit defines */
#define AC97_EXTID_VRA            0x0001
#define AC97_EXTID_DRA            0x0002
#define AC97_EXTID_SPDIF          0x0004
#define AC97_EXTID_VRM            0x0008
#define AC97_EXTID_DSA0           0x0010
#define AC97_EXTID_DSA1           0x0020
#define AC97_EXTID_CDAC           0x0040
#define AC97_EXTID_SDAC           0x0080
#define AC97_EXTID_LDAC           0x0100
#define AC97_EXTID_AMAP           0x0200
#define AC97_EXTID_REV0           0x0400
#define AC97_EXTID_REV1           0x0800
#define AC97_EXTID_ID0            0x4000
#define AC97_EXTID_ID1            0x8000

/* extended status register bit defines */
#define AC97_EXTSTAT_VRA          0x0001
#define AC97_EXTSTAT_DRA          0x0002
#define AC97_EXTSTAT_SPDIF        0x0004
#define AC97_EXTSTAT_VRM          0x0008
#define AC97_EXTSTAT_SPSA0        0x0010
#define AC97_EXTSTAT_SPSA1        0x0020
#define AC97_EXTSTAT_CDAC         0x0040
#define AC97_EXTSTAT_SDAC         0x0080
#define AC97_EXTSTAT_LDAC         0x0100
#define AC97_EXTSTAT_MADC         0x0200
#define AC97_EXTSTAT_SPCV         0x0400
#define AC97_EXTSTAT_PRI          0x0800
#define AC97_EXTSTAT_PRJ          0x1000
#define AC97_EXTSTAT_PRK          0x2000
#define AC97_EXTSTAT_PRL          0x4000

/* useful power states */
#define AC97_PWR_D0               0x0000      /* everything on */
#define AC97_PWR_D1              AC97_PWR_PR0|AC97_PWR_PR1|AC97_PWR_PR4
#define AC97_PWR_D2              AC97_PWR_PR0|AC97_PWR_PR1|AC97_PWR_PR2|AC97_PWR_PR3|AC97_PWR_PR4
#define AC97_PWR_D3              AC97_PWR_PR0|AC97_PWR_PR1|AC97_PWR_PR2|AC97_PWR_PR3|AC97_PWR_PR4
#define AC97_PWR_ANLOFF          AC97_PWR_PR2|AC97_PWR_PR3  /* analog section off */

/* Total number of defined registers.  */
#define AC97_REG_CNT 64


/* OSS interface to the ac97s.. */
#define AC97_STEREO_MASK (SOUND_MASK_VOLUME|SOUND_MASK_PCM|\
	SOUND_MASK_LINE|SOUND_MASK_CD|\
	SOUND_MASK_ALTPCM|SOUND_MASK_IGAIN|\
	SOUND_MASK_LINE1|SOUND_MASK_VIDEO)

#define AC97_SUPPORTED_MASK (AC97_STEREO_MASK | \
	SOUND_MASK_BASS|SOUND_MASK_TREBLE|\
	SOUND_MASK_SPEAKER|SOUND_MASK_MIC|\
	SOUND_MASK_PHONEIN|SOUND_MASK_PHONEOUT)

#define AC97_RECORD_MASK (SOUND_MASK_MIC|\
	SOUND_MASK_CD|SOUND_MASK_IGAIN|SOUND_MASK_VIDEO|\
	SOUND_MASK_LINE1| SOUND_MASK_LINE|\
	SOUND_MASK_PHONEIN)

/* original check is not good enough in case FOO is greater than
 * SOUND_MIXER_NRDEVICES because the supported_mixers has exactly
 * SOUND_MIXER_NRDEVICES elements.
 * before matching the given mixer against the bitmask in supported_mixers we
 * check if mixer number exceeds maximum allowed size which is as mentioned
 * above SOUND_MIXER_NRDEVICES */
#define supported_mixer(CODEC,FOO) ((FOO >= 0) && \
                                    (FOO < SOUND_MIXER_NRDEVICES) && \
                                    (CODEC)->supported_mixers & (1<<FOO) )

struct ac97_codec {
	/* Linked list of codecs */
	struct list_head list;

	/* AC97 controller connected with */
	void *private_data;

	char *name;
	int id;
	int dev_mixer;
	int type;
	u32 model;

	unsigned int modem:1;

	struct ac97_ops *codec_ops;

	/* controller specific lower leverl ac97 accessing routines.
	   must be re-entrant safe */
	u16  (*codec_read)  (struct ac97_codec *codec, u8 reg);
	void (*codec_write) (struct ac97_codec *codec, u8 reg, u16 val);

	/* Wait for codec-ready.  Ok to sleep here.  */
	void  (*codec_wait)  (struct ac97_codec *codec);

	/* callback used by helper drivers for interesting ac97 setups */
	void  (*codec_unregister) (struct ac97_codec *codec);

	struct ac97_driver *driver;
	void *driver_private;	/* Private data for the driver */

	spinlock_t lock;

	/* OSS mixer masks */
	int modcnt;
	int supported_mixers;
	int stereo_mixers;
	int record_sources;

	/* Property flags */
	int flags;

	int bit_resolution;

	/* OSS mixer interface */
	int  (*read_mixer) (struct ac97_codec *codec, int oss_channel);
	void (*write_mixer)(struct ac97_codec *codec, int oss_channel,
			    unsigned int left, unsigned int right);
	int  (*recmask_io) (struct ac97_codec *codec, int rw, int mask);
	int  (*mixer_ioctl)(struct ac97_codec *codec, unsigned int cmd, unsigned long arg);

	/* saved OSS mixer states */
	unsigned int mixer_state[SOUND_MIXER_NRDEVICES];

	/* Software Modem interface */
	int  (*modem_ioctl)(struct ac97_codec *codec, unsigned int cmd, unsigned long arg);
};

/*
 *	Operation structures for each known AC97 chip
 */

struct ac97_ops
{
	/* Initialise */
	int (*init)(struct ac97_codec *c);
	/* Amplifier control */
	int (*amplifier)(struct ac97_codec *codec, int on);
	/* Digital mode control */
	int (*digital)(struct ac97_codec *codec, int slots, int rate, int mode);
#define AUDIO_DIGITAL		0x8000
#define AUDIO_PRO		0x4000
#define AUDIO_DRS		0x2000
#define AUDIO_CCMASK		0x003F

#define AC97_DELUDED_MODEM	1	/* Audio codec reports its a modem */
#define AC97_NO_PCM_VOLUME	2	/* Volume control is missing 	   */
#define AC97_DEFAULT_POWER_OFF 4 /* Needs warm reset to power up */
};

extern int ac97_probe_codec(struct ac97_codec *);

extern struct ac97_codec *ac97_alloc_codec(void);
extern void ac97_release_codec(struct ac97_codec *codec);

struct ac97_driver {
	struct list_head list;
	char *name;
	u32 codec_id;
	u32 codec_mask;
	int (*probe) (struct ac97_codec *codec, struct ac97_driver *driver);
	void (*remove) (struct ac97_codec *codec, struct ac97_driver *driver);
};

/* quirk types */
enum {
	AC97_TUNE_DEFAULT = -1, /* use default from quirk list (not valid in list) */
	AC97_TUNE_NONE = 0,     /* nothing extra to do */
	AC97_TUNE_HP_ONLY,      /* headphone (true line-out) control as master only */
	AC97_TUNE_SWAP_HP,      /* swap headphone and master controls */
	AC97_TUNE_SWAP_SURROUND, /* swap master and surround controls */
	AC97_TUNE_AD_SHARING,   /* for AD1985, turn on OMS bit and use headphone */
	AC97_TUNE_ALC_JACK,     /* for Realtek, enable JACK detection */
};

struct ac97_quirk {
	unsigned short vendor;  /* PCI vendor id */
	unsigned short device;  /* PCI device id */
	unsigned short mask;    /* device id bit mask, 0 = accept all */
	const char *name;       /* name shown as info */
	int type;               /* quirk type above */
};

/*
*	ES8311_REGISTER NAME_REG_REGISTER ADDRESS
*/
#define ES8311_RESET_REG00			0x00  /*reset digital,csm,clock manager etc.*/

/*
* Clock Scheme Register definition
*/
#define ES8311_CLK_MANAGER_REG01		0x01 /* select clk src for mclk, enable clock for codec */
#define ES8311_CLK_MANAGER_REG02		0x02 /* clk divider and clk multiplier */
#define ES8311_CLK_MANAGER_REG03		0x03 /* adc fsmode and osr  */
#define ES8311_CLK_MANAGER_REG04		0x04 /* dac osr */
#define ES8311_CLK_MANAGER_REG05		0x05 /* clk divier for adc and dac */
#define ES8311_CLK_MANAGER_REG06		0x06 /* bclk inverter and divider */
#define ES8311_CLK_MANAGER_REG07		0x07 /* tri-state, lrck divider */
#define ES8311_CLK_MANAGER_REG08		0x08 /* lrck divider */
#define ES8311_SDPIN_REG09			0x09 /* dac serial digital port */
#define ES8311_SDPOUT_REG0A			0x0A /* adc serial digital port */
#define ES8311_SYSTEM_REG0B			0x0B /* system */
#define ES8311_SYSTEM_REG0C			0x0C /* system */
#define ES8311_SYSTEM_REG0D			0x0D /* system, power up/down */
#define ES8311_SYSTEM_REG0E			0x0E /* system, power up/down */
#define ES8311_SYSTEM_REG0F			0x0F /* system, low power */
#define ES8311_SYSTEM_REG10			0x10 /* system */
#define ES8311_SYSTEM_REG11			0x11 /* system */
#define ES8311_SYSTEM_REG12			0x12 /* system, Enable DAC */
#define ES8311_SYSTEM_REG13			0x13 /* system */
#define ES8311_SYSTEM_REG14			0x14 /* system, select DMIC, select analog pga gain */
#define ES8311_ADC_REG15			0x15 /* ADC, adc ramp rate, dmic sense */
#define ES8311_ADC_REG16			0x16 /* ADC */
#define ES8311_ADC_REG17			0x17 /* ADC, volume */
#define ES8311_ADC_REG18			0x18 /* ADC, alc enable and winsize */
#define ES8311_ADC_REG19			0x19 /* ADC, alc maxlevel */
#define ES8311_ADC_REG1A			0x1A /* ADC, alc automute */
#define ES8311_ADC_REG1B			0x1B /* ADC, alc automute, adc hpf s1 */
#define ES8311_ADC_REG1C			0x1C /* ADC, equalizer, hpf s2 */
#define ES8311_DAC_REG31			0x31 /* DAC, mute */
#define ES8311_DAC_REG32			0x32 /* DAC, volume */
#define ES8311_DAC_REG33			0x33 /* DAC, offset */
#define ES8311_DAC_REG34			0x34 /* DAC, drc enable, drc winsize */
#define ES8311_DAC_REG35			0x35 /* DAC, drc maxlevel, minilevel */
#define ES8311_DAC_REG37			0x37 /* DAC, ramprate */
#define ES8311_GPIO_REG44			0x44 /* GPIO, dac2adc for test */
#define ES8311_GP_REG45				0x45 /* GP CONTROL */
#define ES8311_CHD1_REGFD			0xFD /* CHIP ID1 */
#define ES8311_CHD2_REGFE			0xFE /* CHIP ID2 */
#define ES8311_CHVER_REGFF			0xFF /* VERSION */
#define ES8311_CHD1_REGFD			0xFD /* CHIP ID1 */

#define ES8311_MAX_REGISTER			0xFF
#endif
