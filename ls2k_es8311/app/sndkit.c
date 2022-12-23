/*
采用OSS进行音频编程时应当遵循的一般性框架：

（1）打开音频设备；
（2）设置采样格式（即量化位数）；
（3）设置声道数（单声道或者立体声）；
（4）设置采样率（即采样速度）；
（5）从设备读入数据或者向设备写入数据；
（6）关闭音频设备；
上述过程中，第（6）可选，因为在进程正常或非正常退出时都会关闭它所打开的文件描述字，这样也就把音频设备的资源释放了。但是，除此之外的五个步骤是不能缺少的，并且顺序也不能颠倒（OSS编程指南对此的说明是：在指定声道数之前设置采样率不能在所有的音频硬件上工作——如果在将声卡设置为高采样率模式之后更改了声道数，应用程序将出现不正确的行为。同时，必须在使用read()或者write()调用读写数据之前设置采样率）。
例如：录音 ./sndkit -c 2 -b 16 -f 44 -r rev.wav
播放 ./sndkit -c 2 -b 16 -f 44 -p rev.wav
 注意：rev.wav文件得先创建，如：touch rev.wav
*/
 

/*
Name: SndKit.c
Copyright: GPLv2
Author: rockins( ybc2084@163.com)
Date: 15-10-06 18:22
Description: implent raw sound record/play
run: ./SndKit [-h] [-d device] [-c channel] [-b bits] [-f hz] [-l len] <-r|-p file>
e.g.:
./SndKit -h      show help information
./SndKit -r record.wav                  record audio from microphone(/dev/dsp)
./SndKit -d /dev/dsp -p record.wav       playback record.wav via /dev/dsp
./SndKit -b 8 -f 22 -r reacord.wav       record audio in 8 bit & 22k hz
./SndKit -d /dev/dsp -c 2 -r record.wav record audio in /dev/dsp and stereo
./SndKit -r -l 40 record.wav    record 40k audio data
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h> /*for OSS style sound programing */
#define TRUE   1
#define FALSE   0
#define FMT8BITS AFMT_U8 /*unsigned 8 bits(for almost PC) */
#define FMT16BITS AFMT_S16_LE /*signed 16 bits,little endian */
#define FMT8K   8 /*default sampling rate */
#define FMT11K 11 /*11,22,44,48 are three pop rate */
#define FMT12K 12
#define FMT16K 16
#define FMT22K 22
#define FMT24K 24
#define FMT32K 32
#define FMT44K 44
#define FMT48K 48
#define FMT64K   64
#define MONO    1
#define STEREO 2
#define ACT_RECORD 0
#define ACT_PLAY 1
#define DFT_SND_DEV "/dev/dsp"
#define DFT_SND_FMT FMT8BITS
#define DFT_SND_SPD FMT8K
#define DFT_SND_CHN MONO
#define DFT_LEN   1024 /*default record length:40k */
#define BUFF_SIZE 512 /*buffer size:512 Bytes */
/************** function prototype ********************/
void Usage(void);
int OpenDevice(const char *, unsigned int);
int OpenFile(const char *, unsigned int);
int CloseDevice(unsigned int);
int CloseFile(unsigned int);
/************** function implementation **************/
int main(int argc, char *argv[])
{
    unsigned int snd_fmt = DFT_SND_FMT; /*sound format,bits */
    unsigned int snd_chn = DFT_SND_CHN; /*sound channel number */
    unsigned int snd_spd = DFT_SND_SPD; /*sound speed,frequency */
    unsigned char *s_file = NULL; /*file hold sound data */
    unsigned char *snd_device = DFT_SND_DEV; /*sound device */
    unsigned int recd_or_play = ACT_PLAY; /*flag(default play) */
    unsigned char buff[BUFF_SIZE]; /*sound buffer */
    unsigned long len = DFT_LEN * 1024; /*record or play length(k) */
    int snd_fd;    /*sound device descriptor */
    int s_fd;    /*sound data file descrit */
    ssize_t n;    /*bytes already in or out */
    ssize_t nRD;   /*bytes readin */
    ssize_t nWR;   /*bytes write out */
    unsigned int opt_chr; /*hold cli option */
    unsigned int opt_err = FALSE; /*indicate whether parse option error */
    /*parse cli option via getopt routine */
    optind = 0;     /*set optindex to 0 */
    while ((opt_chr = getopt(argc, argv, "hd:c:b:f:l:r:p:")) != -1) {
switch (opt_chr) {
case 'h':
     Usage();
     return (0);    /*return, 0 denote success */
     break;
case 'd':
     snd_device = optarg; /*sound device name */
     break;
case 'c':
     if (atoi(optarg) == 1) /*select channel number */
   snd_chn = MONO;
     else if (atoi(optarg) == 2)
   snd_chn = STEREO;
     else {
   opt_err = 1;   /*error occur */
   fprintf(stderr, "wrong channel setting,"
    "should be 1 or 2(default 1,MONO)/n");
     }
     break;
case 'b':
     if (atoi(optarg) == 8) /*select data bit */
   snd_fmt = FMT8BITS;
     else if (atoi(optarg) == 16)
   snd_fmt = FMT16BITS;
     else {
   opt_err = 1;   /*error occur */
   fprintf(stderr, "wrong bits setting,"
    "should be 8 or 16(default 8)/n");
     }
     break;
case 'f':
     if (atoi(optarg) == 8)
   snd_spd = FMT8K; /*sampling rate:8kbps */
     else if (atoi(optarg) == 11)
   snd_spd = FMT11K; /*smapling rate:11.025k */
     else if (atoi(optarg) == 12)
   snd_spd = FMT12K; /*smapling rate:12k */
     else if (atoi(optarg) == 16)
   snd_spd = FMT16K; /*smapling rate:16k */
     else if (atoi(optarg) == 22)
   snd_spd = FMT22K; /*sampling rate:22.050k */
     else if (atoi(optarg) == 44)
   snd_spd = FMT44K; /*sampling rate:44.100k */
     else if (atoi(optarg) == 48)
   snd_spd = FMT48K; /*sampling rate:48k */
     else {
   opt_err = 1;   /*error occur */
   fprintf(stderr, "wrong sampling rate,"
    "should be 8,11,12,16,22,or 44(default 8kbps)/n");
     }
     break;
case 'l':
     len = atoi(optarg) * 1024; /*record length(k) */
     break;
case 'r':
     recd_or_play = ACT_RECORD;
     s_file = optarg;   /*file to record sound */
     break;
case 'p':
     recd_or_play = ACT_PLAY;
     s_file = optarg;   /*file to play sound */
     break;
case '?':
     opt_err = 1;
     fprintf(stderr, "unknown option:%c/n", optopt);
     break;
default:
     opt_err = 1;
     fprintf(stderr, "getopt error:%d/n", opterr);
     break;
}
    }
    /*check if cli option parsed right*/
    if (opt_err || argc < 2) {
fprintf(stderr, "parse command option failed!!!/n"
   "run ./SndKit -h for help/n");
return (-1);
    }
    /*open device */
    if ((snd_fd = OpenDevice(snd_device, recd_or_play)) < 0) {
fprintf(stderr, "cannot open device %s:%s/n", snd_device,
   strerror(errno));
return (-1);
    }
    /*open sound data file */
    if ((s_fd = OpenFile(s_file, recd_or_play)) < 0) {
fprintf(stderr, "cannot open sound file %s:%s/n", s_file,
   strerror(errno));
return (-1);
    }
    /*set sound format */
    if (SetFormat(snd_fd, snd_fmt, snd_chn, snd_spd) < 0) {
fprintf(stderr, "cannot set %s in bit %d, channel %d, speed %d/n",
   snd_device, snd_fmt, snd_chn, snd_spd);
return (-1);
    }
    /*do real action:record or playback */
    if (recd_or_play == ACT_RECORD) {   /*record sound into data file*/
n = 0;
while (n < len) {
     nRD = 0;   /*amount read from sound device */
     if ((nRD = read(snd_fd, buff, BUFF_SIZE)) < 0) {
   perror("read sound device failed");
   return (-1);
     }
     if (n + nRD <= len) /*the len is not full */
   nWR = nRD; /*write amount to sound data file */
     else
   nWR = len - n; /*len will be overflow */
     unsigned long old_nWR = nWR; /*s hold nWR's old value */
     unsigned long t = 0L;   /*temp counter */
    
     while (nWR > 0) {
   if ((t = write(s_fd, buff, nWR)) < 0) {
      perror("write sound data file failed");
      return (-1);
   }
   nWR -= t;
     }
     n += old_nWR;
}
    } else if (recd_or_play == ACT_PLAY) { /*write sound data to device*/
while (TRUE) {
     nRD = 0L;    /*read amount from sound device */
     if ((nRD = read(s_fd, buff, BUFF_SIZE)) < 0) {
   perror("read sound data file failed");
   return (-1);
     } else if (nRD == 0) { /*nRD == 0 means all sound stream output */
   printf("sound data play complete!/n");
   exit(0);
     }
     nWR = nRD;
     while (nWR > 0) {   /*write into device*/
   if ((n = write(snd_fd, buff, nWR)) < 0) {
      perror("write sound device file failed");
      return (-1);
   }
   nWR -= n;
     }
}
    }
    /*close sound device and sound data file*/
    CloseDevice(snd_fd);
    CloseFile(s_fd);
    return (0);
}
/*
* OpenDevice():open sound device
* params:
* dev_name -- device name,such as /dev/dsp
* flag -- flag(ACT_RECORD or ACT_PLAY)
* returns:
* file descriptor of sound device if sucess
* -1 if failed
*/
int OpenDevice(const char *dev_name, unsigned int flag)
{
    int dev_fd;
    /*open sound device */
    if (flag == ACT_RECORD) {
if ((dev_fd = open(dev_name, O_RDONLY)) < 0) {
     return (-1);
}
    } else if (flag == ACT_PLAY) {
if ((dev_fd = open(dev_name, O_WRONLY)) < 0) {
     return (-1);
}
    }
    return (dev_fd);
}
/*
* CloseDevice():close the sound device
* params:
* dev_fd -- the sound device's file descriptor
* returns:
* 0 if success
* -1 if error occured
*/
int CloseDevice(unsigned int dev_fd)
{
    return (close(dev_fd));
}
/*
* OpenFile():open sound data file
* params:
* file_name -- file name,e.g,record.wav
* flag -- flag(ACT_RECORD or ACT_PLAY)
* returns:
* file descriptor of sound data file if sucess
* -1 if failed
*/
int OpenFile(const char *file_name, unsigned int flag)
{
    int file_fd;
    /*open sound data file */
    if (flag == ACT_RECORD) {
if ((file_fd = open(file_name, O_WRONLY)) < 0) {
     return (-1);
}
    } else if (flag == ACT_PLAY) {
if ((file_fd = open(file_name, O_RDONLY)) < 0) {
     return (-1);
}
    }
    return (file_fd);
}
/*
* CloseFile():close the sound data file
* params:
* file_fd -- the sound data file's descriptor
* returns:
* 0 if success
* -1 if error occured
*/
int CloseFile(unsigned int file_fd)
{
    return (close(file_fd));
}
/*
* SetFormat():Set Record and Playback format
* params;
* fd -- device file descriptor
* chn -- channel(MONO or STEREO)
* bits -- FMT8BITS(8bits), FMT16BITS(16bits)
* hz -- FMT8K(8000HZ), FMT16K(16000HZ), FMT22K(22000HZ),
* FMT44K(44000HZ),FMT48K(48000HZ)
* returns:
* return 0 if success, else return -1
* notes:
* parameter setting order should be like as:
* 1.sample format(number of bits)
* 2.number of channels(mono or stereo)
* 3.sampling rate(speed)
*/
int SetFormat(unsigned int fd, unsigned int bits, unsigned int chn,
       unsigned int hz)
{
    int ioctl_val;
    /* set bit format */
    ioctl_val = bits;
    if (ioctl(fd, SNDCTL_DSP_SETFMT, &ioctl_val) == -1) {
fprintf(stderr, "Set fmt to bit %d failed:%s/n", bits,
   strerror(errno));
return (-1);
    }
    if (ioctl_val != bits) {
fprintf(stderr, "do not support bit %d, supported %d/n", bits,
   ioctl_val);
return (-1);
    }
    /*set channel */
    ioctl_val = chn;
    if ((ioctl(fd, SNDCTL_DSP_CHANNELS, &ioctl_val)) == -1) {
fprintf(stderr, "Set Audio Channels %d failed:%s/n", chn,
   strerror(errno));
return (-1);
    }
    if (ioctl_val != chn) {
fprintf(stderr, "do not support channel %d,supported %d/n", chn,
   ioctl_val);
return (-1);
    }
    /*set speed */
    ioctl_val = hz;
    if (ioctl(fd, SNDCTL_DSP_SPEED, &ioctl_val) == -1) {
fprintf(stderr, "Set speed to %d failed:%s/n", hz,
   strerror(errno));
return (-1);
    }
    if (ioctl_val != hz) {
fprintf(stderr, "do not support speed %d,supported is %d/n", hz,
   ioctl_val);
return (-1);
    }
    return (0);
}
/*
* Usage():print help information
* params:(none)
* returns:(none)
*/
void Usage(void)
{
    printf
("run: ./SndKit [-h] [-d device] [-c channel] [-b bits] [-f hz] [-l len] <-r|-p file>/n"
"Description: implent raw sound record/play/n" "option:/n"
"/t -h: print help informations/n"
"/t -d device: assign sound device to record or playback(default /dev/dsp)/n"
"/t -c channel: indicate in MONO or STEREO channel(default MONO)/n"
"/t -b bits: assign sampling bits depth(default 8bits unsigned)/n"
"/t -f hz: indicate sampling rate,i.e,frequence(default 8kbps)/n"
"/t -l len: indicate recording sound's length(default 1024k)/n"
"/t -r|-p file: indicate record in or playback(no default,must give out explicitly)/n"
"e.g.:/n"
"/t ./SndKit -h     show help information/n"
"/t ./SndKit -r record.wav    record audio from microphone(/dev/dsp)/n"
"/t ./SndKit -d /dev/dsp -p record.wav playback record.wav via /dev/dsp/n"
"/t ./SndKit -b 8 -f 22 -r reacord.wav record audio in 8 bit & 22k hz/n"
"/t ./SndKit -d /dev/dsp -c 2 -r record.wav record audio in /dev/dsp and stereo/n"
"/t ./SndKit -r -l 40 record.wav   record 40k audio data in record.wav/n");
}
 
/*
这是作者编写的一个读写/dev/dsp设备的小程序，可以用它来录制一段声音，也可以用来将录制到文件中的声音播放出来。除此之外，还可以通过命令行参数指定设备参数，如：采样率、量化位数、声道数、要录制的声音长度，等等
*/
