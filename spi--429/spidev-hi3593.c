/*
 * Simple synchronous userspace interface to SPI devices
 *
 * Copyright (C) 2006 SWAPP
 *	Andrea Paterniani <a.paterniani@swapp-eng.it>
 * Copyright (C) 2007 David Brownell (simplification, cleanup)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#include <asm/uaccess.h>


/*
 * This supports access to SPI devices using normal userspace I/O calls.
 * Note that while traditional UNIX/POSIX I/O semantics are half duplex,
 * and often mask message boundaries, full SPI support requires full duplex
 * transfers.  There are several kinds of internal message boundaries to
 * handle chipselect management and other protocol options.
 *
 * SPI has a character major number assigned.  We allocate minor numbers
 * dynamically using a bitmask.  You must use hotplug tools, such as udev
 * (or mdev with busybox) to create and destroy the /dev/spidevB.C device
 * nodes, since there is no fixed association of minor numbers with any
 * particular SPI bus or device.
 */
#define SPIDEV_MAJOR			154	/* assigned */
#define N_SPI_MINORS			32	/* ... up to 256 */
#define SPIDEV_NUM                      4    /* hi3593 device num for 4*/

#define GENERAL_ADDR                  0x1fe10420   /* general register0 addr*/
#define GPIO_DIR_ADDR                0x1fe10500  /* hi3593 spi cs_gpio direction addr*/
#define GPO_ADDR                          0x1fe10510    /* hi3593 spi cs_gpio GPO addr*/

static DECLARE_BITMAP(minors, N_SPI_MINORS);
static struct mutex mutex;

/* Bit masks for spi_device.mode management.  Note that incorrect
 * settings for some settings can cause *lots* of trouble for other
 * devices on a shared bus:
 *
 *  - CS_HIGH ... this device will be active when it shouldn't be
 *  - 3WIRE ... when active, it won't behave as it should
 *  - NO_CS ... there will be no explicit message boundaries; this
 *	is completely incompatible with the shared bus model
 *  - READY ... transfers may proceed when they shouldn't.
 *
 * REVISIT should changing those flags be privileged?
 */
#define SPI_MODE_MASK		(SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
				| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
				| SPI_NO_CS | SPI_READY)

struct spidev_data {
	dev_t			devt;
	spinlock_t		spi_lock;
	struct spi_device	*spi;
	struct list_head	device_entry;

	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex		buf_lock;
	unsigned		users;
	u8			*buffer;
        u8			gpio_cs;
};

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static unsigned bufsiz = 4096;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

/*-------------------------------------------------------------------------*/

/*
 * We can't use the standard synchronous wrappers for file I/O; we
 * need to protect against async removal of the underlying spi_device.
 */
/*
void set_iobit(volatile void __iomem *addr, u64 n)
{
    u64 val;
    val = readq(addr);
    val = val | (1<<n); 
    writeq(val, addr);
}
void clear_iobit(volatile void __iomem *addr, u64 n)
{
    u64 val;
    val = readq(addr);
    val = val & ~(1<<n);  
    writeq(val, addr);
}
*/
static void spidev_complete(void *arg)
{
	complete(arg);
}

static ssize_t
spidev_sync(struct spidev_data *spidev, struct spi_message *message)
{
	DECLARE_COMPLETION_ONSTACK(done);
	int status;

	message->complete = spidev_complete;
	message->context = &done;

	spin_lock_irq(&spidev->spi_lock);
	if (spidev->spi == NULL)
		status = -ESHUTDOWN;
	else
		status = spi_async(spidev->spi, message);
	spin_unlock_irq(&spidev->spi_lock);

	if (status == 0) {
		wait_for_completion(&done);
		status = message->status;
		if (status == 0)
			status = message->actual_length;
	}
	return status;
}

static inline ssize_t
spidev_sync_write(struct spidev_data *spidev, size_t len)
{
	struct spi_transfer	t = {
			.tx_buf		= spidev->buffer,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spidev_sync(spidev, &m);
}

static inline ssize_t
spidev_sync_read(struct spidev_data *spidev, size_t len)
{
	struct spi_transfer	t = {
			.rx_buf		= spidev->buffer,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spidev_sync(spidev, &m);
}

/*-------------------------------------------------------------------------*/
#define READ_OP_CODE(x) ((x | 0x80) & 0xFC) // 写操作 bit7置一
/* Read-only message with current device setup */
static ssize_t
spidev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct spidev_data	*spidev;
        u8                           i, cs, bufreg[1], bufread[2];
	ssize_t			status = 0;
        static volatile void __iomem *chipselect_addr;
	struct inode *inode = file_inode(filp);
	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz)
		return -EMSGSIZE;

	spidev = filp->private_data;

        spidev->spi->master->num_chipselect = 8;
        chipselect_addr = ioremap(GPO_ADDR, 8);
        cs = spidev->gpio_cs;
	mutex_lock(&mutex);
        copy_from_user(spidev->buffer, buf, 1);//read 1byte from buf[0]: register code
        clear_bit(59 -cs, chipselect_addr); /*GPO3-0 at register 56:59 bit, GPI3-0 at register 60:63 bit*/
        status = spidev_sync_write(spidev, 1);//send register code.
		status = spidev_sync_read(spidev, count);
        set_bit(59 -cs, chipselect_addr);
		//printk("spi-hi3593.%d.count=%d, status=%d.read:\n", iminor(inode),count, status);
	if (status > 0) {
		unsigned long	missing;
		/*
		for(i=0; i<status; i++)
			printk("0x%x ", *(spidev->buffer+i));
		printk("\n");
		*/
		missing = copy_to_user(buf+1, spidev->buffer, status);//copy_to_user begin buf[1].buf[0] is register code.
		if (missing == status)
			status = -EFAULT;
		else
			status = status - missing;
	}
	mutex_unlock(&mutex);
	/*
	bufreg[0] = 0XD4;
	spidev->buffer = bufreg;
	clear_bit(59 -cs, chipselect_addr); //GPO3-0 at register 56:59 bit, GPI3-0 at register 60:63 bit
    status = spidev_sync_write(spidev, 1);//send register code.
	status = spidev_sync_read(spidev, 1);
    set_bit(59 -cs, chipselect_addr);
	printk("spi-hi3593.%d.cs=%d,0XD4reg= 0x%x\n", iminor(inode),cs,*(spidev->buffer));
	*/
	return status;
}

/* Write-only message with current device setup */
static ssize_t
spidev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct spidev_data	*spidev;
        u8                            i, cs;
	ssize_t			status = 0;
	unsigned long		missing;
        static volatile void __iomem *chipselect_addr;
	struct inode *inode = file_inode(filp);
	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz)
		return -EMSGSIZE;

	spidev = filp->private_data;
        chipselect_addr = ioremap(GPO_ADDR, 8);
        cs = spidev->gpio_cs;
        
	mutex_lock(&mutex);
	missing = copy_from_user(spidev->buffer, buf, count);
	if (missing == 0) {
                clear_bit(59 -cs, chipselect_addr); /*GPO3-0 at register 56:59 bit, GPI3-0 at register 60:63 bit*/
		status = spidev_sync_write(spidev, count);
                set_bit(59 -cs, chipselect_addr);
		/*		
        printk("spi-hi3593.%d.count=%d, write:\n", iminor(inode),count);
		for(i=0; i<count; i++)
			printk("0x%x ", *(buf+i));
		printk("\n");
		*/
	} else
		status = -EFAULT;
	mutex_unlock(&mutex);

	return status;
}

static int spidev_message(struct spidev_data *spidev,
		struct spi_ioc_transfer *u_xfers, unsigned n_xfers)
{
	struct spi_message	msg;
	struct spi_transfer	*k_xfers;
	struct spi_transfer	*k_tmp;
	struct spi_ioc_transfer *u_tmp;
	unsigned		n, total;
	u8			*buf;
	int			status = -EFAULT;

	spi_message_init(&msg);
	k_xfers = kcalloc(n_xfers, sizeof(*k_tmp), GFP_KERNEL);
	if (k_xfers == NULL)
		return -ENOMEM;

	/* Construct spi_message, copying any tx data to bounce buffer.
	 * We walk the array of user-provided transfers, using each one
	 * to initialize a kernel version of the same transfer.
	 */
	buf = spidev->buffer;
	total = 0;
	for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers;
			n;
			n--, k_tmp++, u_tmp++) {
		k_tmp->len = u_tmp->len;

		total += k_tmp->len;
		if (total > bufsiz) {
			status = -EMSGSIZE;
			goto done;
		}

		if (u_tmp->rx_buf) {
			k_tmp->rx_buf = buf;
			if (!access_ok(VERIFY_WRITE, (u8 __user *)
						(uintptr_t) u_tmp->rx_buf,
						u_tmp->len))
				goto done;
		}
		if (u_tmp->tx_buf) {
			k_tmp->tx_buf = buf;
			if (copy_from_user(buf, (const u8 __user *)
						(uintptr_t) u_tmp->tx_buf,
					u_tmp->len))
				goto done;
		}
		buf += k_tmp->len;

		k_tmp->cs_change = !!u_tmp->cs_change;
		k_tmp->bits_per_word = u_tmp->bits_per_word;
		k_tmp->delay_usecs = u_tmp->delay_usecs;
		k_tmp->speed_hz = u_tmp->speed_hz;
#ifdef VERBOSE
		dev_dbg(&spidev->spi->dev,
			"  xfer len %zd %s%s%s%dbits %u usec %uHz\n",
			u_tmp->len,
			u_tmp->rx_buf ? "rx " : "",
			u_tmp->tx_buf ? "tx " : "",
			u_tmp->cs_change ? "cs " : "",
			u_tmp->bits_per_word ? : spidev->spi->bits_per_word,
			u_tmp->delay_usecs,
			u_tmp->speed_hz ? : spidev->spi->max_speed_hz);
#endif
		spi_message_add_tail(k_tmp, &msg);
	}

	status = spidev_sync(spidev, &msg);
	if (status < 0)
		goto done;

	/* copy any rx data out of bounce buffer */
	buf = spidev->buffer;
	for (n = n_xfers, u_tmp = u_xfers; n; n--, u_tmp++) {
		if (u_tmp->rx_buf) {
			if (__copy_to_user((u8 __user *)
					(uintptr_t) u_tmp->rx_buf, buf,
					u_tmp->len)) {
				status = -EFAULT;
				goto done;
			}
		}
		buf += u_tmp->len;
	}
	status = total;

done:
	kfree(k_xfers);
	return status;
}

static long
spidev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int			err = 0;
	int			retval = 0;
	struct spidev_data	*spidev;
	struct spi_device	*spi;
	u32			tmp;
	unsigned		n_ioc;
	struct spi_ioc_transfer	*ioc;

	/* Check type and command number */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;

	/* Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	/* guard against device removal before, or while,
	 * we issue this ioctl.
	 */
	spidev = filp->private_data;
	spin_lock_irq(&spidev->spi_lock);
	spi = spi_dev_get(spidev->spi);
	spin_unlock_irq(&spidev->spi_lock);

	if (spi == NULL)
		return -ESHUTDOWN;

	/* use the buffer lock here for triple duty:
	 *  - prevent I/O (from us) so calling spi_setup() is safe;
	 *  - prevent concurrent SPI_IOC_WR_* from morphing
	 *    data fields while SPI_IOC_RD_* reads them;
	 *  - SPI_IOC_MESSAGE needs the buffer locked "normally".
	 */
	mutex_lock(&spidev->buf_lock);

	switch (cmd) {
	/* read requests */
	case SPI_IOC_RD_MODE:
		retval = __put_user(spi->mode & SPI_MODE_MASK,
					(__u8 __user *)arg);
		break;
	case SPI_IOC_RD_LSB_FIRST:
		retval = __put_user((spi->mode & SPI_LSB_FIRST) ?  1 : 0,
					(__u8 __user *)arg);
		break;
	case SPI_IOC_RD_BITS_PER_WORD:
		retval = __put_user(spi->bits_per_word, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_MAX_SPEED_HZ:
		retval = __put_user(spi->max_speed_hz, (__u32 __user *)arg);
		break;

	/* write requests */
	case SPI_IOC_WR_MODE:
		retval = __get_user(tmp, (u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->mode;

			if (tmp & ~SPI_MODE_MASK) {
				retval = -EINVAL;
				break;
			}

			tmp |= spi->mode & ~SPI_MODE_MASK;
			spi->mode = (u8)tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->mode = save;
			else
				dev_dbg(&spi->dev, "spi mode %02x\n", tmp);
		}
		break;
	case SPI_IOC_WR_LSB_FIRST:
		retval = __get_user(tmp, (__u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->mode;

			if (tmp)
				spi->mode |= SPI_LSB_FIRST;
			else
				spi->mode &= ~SPI_LSB_FIRST;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->mode = save;
			else
				dev_dbg(&spi->dev, "%csb first\n",
						tmp ? 'l' : 'm');
		}
		break;
	case SPI_IOC_WR_BITS_PER_WORD:
		retval = __get_user(tmp, (__u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->bits_per_word;

			spi->bits_per_word = tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->bits_per_word = save;
			else
				dev_dbg(&spi->dev, "%d bits per word\n", tmp);
		}
		break;
	case SPI_IOC_WR_MAX_SPEED_HZ:
		retval = __get_user(tmp, (__u32 __user *)arg);
		if (retval == 0) {
			u32	save = spi->max_speed_hz;

			spi->max_speed_hz = tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->max_speed_hz = save;
			else
				dev_dbg(&spi->dev, "%d Hz (max)\n", tmp);
		}
		break;

	default:
		/* segmented and/or full-duplex I/O request */
		if (_IOC_NR(cmd) != _IOC_NR(SPI_IOC_MESSAGE(0))
				|| _IOC_DIR(cmd) != _IOC_WRITE) {
			retval = -ENOTTY;
			break;
		}

		tmp = _IOC_SIZE(cmd);
		if ((tmp % sizeof(struct spi_ioc_transfer)) != 0) {
			retval = -EINVAL;
			break;
		}
		n_ioc = tmp / sizeof(struct spi_ioc_transfer);
		if (n_ioc == 0)
			break;

		/* copy into scratch area */
		ioc = kmalloc(tmp, GFP_KERNEL);
		if (!ioc) {
			retval = -ENOMEM;
			break;
		}
		if (__copy_from_user(ioc, (void __user *)arg, tmp)) {
			kfree(ioc);
			retval = -EFAULT;
			break;
		}

		/* translate to spi_message, execute */
		retval = spidev_message(spidev, ioc, n_ioc);
		kfree(ioc);
		break;
	}

	mutex_unlock(&spidev->buf_lock);
	spi_dev_put(spi);
	return retval;
}

#ifdef CONFIG_COMPAT
static long
spidev_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return spidev_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define spidev_compat_ioctl NULL
#endif /* CONFIG_COMPAT */

static int spidev_open(struct inode *inode, struct file *filp)
{
	struct spidev_data	*spidev;
	int			status = -ENXIO;

	mutex_lock(&device_list_lock);

	list_for_each_entry(spidev, &device_list, device_entry) {
		if (spidev->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		if (!spidev->buffer) {
			spidev->buffer = kmalloc(bufsiz, GFP_KERNEL);
			if (!spidev->buffer) {
				dev_dbg(&spidev->spi->dev, "open/ENOMEM\n");
				status = -ENOMEM;
			}
		}
		if (status == 0) {
			spidev->users++;
			filp->private_data = spidev;
			nonseekable_open(inode, filp);
		}
	} else
		pr_debug("spidev: nothing for minor %d\n", iminor(inode));
        printk("spi-hi3593.%d open.\n", iminor(inode));
	mutex_unlock(&device_list_lock);
	return status;
}

static int spidev_release(struct inode *inode, struct file *filp)
{
	struct spidev_data	*spidev;
	int			status = 0;

	mutex_lock(&device_list_lock);
	spidev = filp->private_data;
	filp->private_data = NULL;

	/* last close? */
	spidev->users--;
	if (!spidev->users) {
		int		dofree;

		kfree(spidev->buffer);
		spidev->buffer = NULL;

		/* ... after we unbound from the underlying device? */
		spin_lock_irq(&spidev->spi_lock);
		dofree = (spidev->spi == NULL);
		spin_unlock_irq(&spidev->spi_lock);

		if (dofree)
			kfree(spidev);
	}
	mutex_unlock(&device_list_lock);

	return status;
}

static const struct file_operations spidev_fops = {
	.owner =	THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
	.write =	spidev_write,
	.read =		spidev_read,
	//.unlocked_ioctl = spidev_ioctl,
	//.compat_ioctl = spidev_compat_ioctl,
	.open =		spidev_open,
	.release =	spidev_release,
	.llseek =	no_llseek,
};

/*-------------------------------------------------------------------------*/

/* The main reason to have this class is to make mdev/udev create the
 * /dev/spidevB.C character device nodes exposing our userspace API.
 * It also simplifies memory management.
 */

static struct class *spidev_class;


/*-------------------------------------------------------------------------*/

static int spidev_probe(struct spi_device *spi)
{
	struct spidev_data	*spidev[SPIDEV_NUM];
	int			i, status;
	unsigned long		minor;
        //static volatile void __iomem *general_addr;
        static volatile void __iomem *direction_addr;
        static volatile void __iomem *chipselect_addr;
#if 0
        if (!devm_request_mem_region(GPIO_DIR_ADDR, 8, "direction_addr"))
        {  
        pr_debug("request_mem_region direction_addr failed\n");  
        return -1;
        }
        if (!devm_request_mem_region(GPO_ADDR, 8, "chipselect_addr"))
        {  
        pr_debug("request_mem_region chipselect_addr failed\n");  
        return -1;
        }
#endif
        //general_addr = ioremap(GENERAL_ADDR, 8);
        direction_addr = devm_ioremap(&spi->dev, GPIO_DIR_ADDR, 8);
        chipselect_addr = devm_ioremap(&spi->dev, GPO_ADDR, 8);
        //clear_bit(9, general_addr);       /*general register0 [bit 9].   0 is gpio, 1 is NAND*/

		mutex_init(&mutex);
        for (i = 0; i < SPIDEV_NUM; i++) {
        	/* Allocate driver data */
        	spidev[i] = kzalloc(sizeof(*spidev[i]), GFP_KERNEL);
        	if (!spidev[i])
        		return -ENOMEM;

        	/* Initialize the driver data */
        	spidev[i]->spi = spi;
        	spin_lock_init(&spidev[i]->spi_lock);
        	mutex_init(&spidev[i]->buf_lock);

        	INIT_LIST_HEAD(&spidev[i]->device_entry);

        	/* If we can allocate a minor number, hook up this device.
        	 * Reusing minors is fine so long as udev or mdev is working.
        	 */
        	mutex_lock(&device_list_lock);
        	minor = find_first_zero_bit(minors, N_SPI_MINORS);
                
        	if (minor < N_SPI_MINORS) {
        		struct device *dev;

        		spidev[i]->devt = MKDEV(SPIDEV_MAJOR, minor);
        		dev = device_create(spidev_class, &spi->dev, spidev[i]->devt,
        				    spidev[i], "spi-hi3593.%d", i);
        		status = PTR_RET(dev);
        	} else {
        		dev_dbg(&spi->dev, "no minor number available!\n");
        		status = -ENODEV;
        	}
        	if (status == 0) {
        		set_bit(minor, minors);
        		list_add(&spidev[i]->device_entry, &device_list);
        	}
                spidev[i]->gpio_cs = i;
                clear_bit(56 + i, direction_addr);/*chipselect gpio_direction_addr clear bit. bit 0 is out*/
                set_bit(56 + i, chipselect_addr);/*chipselect gpo set bit*/
                
        	mutex_unlock(&device_list_lock);
        	if (status == 0)
        		spi_set_drvdata(spi, spidev );
        	else
        		kfree(spidev[i]);
                printk("spi-hi3593.%d set up. minor = %ld\n", i, minor);
        }
	return status;
}

static int spidev_remove(struct spi_device *spi)
{
	struct spidev_data	**spidev = spi_get_drvdata(spi);
        int i;
        for (i = 0; i < SPIDEV_NUM; i++) {
        	/* make sure ops on existing fds can abort cleanly */
        	spin_lock_irq(&(spidev[i]->spi_lock));
        	spidev[i]->spi = NULL;
        	spi_set_drvdata(spi, NULL);
        	spin_unlock_irq(&(spidev[i]->spi_lock));

        	/* prevent new opens */
        	mutex_lock(&device_list_lock);
        	list_del(&spidev[i]->device_entry);
        	device_destroy(spidev_class, spidev[i]->devt);
        	clear_bit(MINOR(spidev[i]->devt), minors);
        	if (spidev[i]->users == 0)
        		kfree(spidev[i]);
        	mutex_unlock(&device_list_lock);
        }
	return 0;
}

static const struct of_device_id spidev_dt_ids[] = {
	{ .compatible = "spi-hi3593" },
	{},
};

MODULE_DEVICE_TABLE(of, spidev_dt_ids);

static struct spi_driver spidev_spi_driver = {
	.driver = {
		.name =		"spi-hi3593",
		.owner =	THIS_MODULE,
		.of_match_table = of_match_ptr(spidev_dt_ids),
	},
	.probe =	spidev_probe,
	.remove =	spidev_remove,

	/* NOTE:  suspend/resume methods are not necessary here.
	 * We don't do anything except pass the requests to/from
	 * the underlying controller.  The refrigerator handles
	 * most issues; the controller driver handles the rest.
	 */
};

/*-------------------------------------------------------------------------*/
#define SPI_ADDR_BASE 0x1FC00000
static int __init spidev_init(void)
{
	int status;
//test, add spi_device
        static struct spi_board_info spi_board_info[] = {
        	{
        		.modalias = "spi-hi3593",
        		.max_speed_hz = 0x001e8480,
        		.bus_num = 0,
        		.chip_select = 2,
        		//.platform_data = &spi_flash_data,
        	},
        };
	/* register SPI device information */
	spi_register_board_info(spi_board_info,
				ARRAY_SIZE(spi_board_info));

#if 0
//void __iomem *spcr = ioremap(SPI_ADDR_BASE + 0x0, 1);
void __iomem *spsr = ioremap(SPI_ADDR_BASE + 0x1, 1);
//void __iomem *sper = ioremap(SPI_ADDR_BASE + 0x3, 1);
//void __iomem *softcs = ioremap(SPI_ADDR_BASE + 0x5, 1);
//void __iomem *TxRx = ioremap(SPI_ADDR_BASE + 0x2, 1);
	u8 ret;
	ret = ioread8(spsr);//重置状态寄存器
	printk("ioread8(spsr) : 0x%x\n", ret);
#endif
	/* Claim our 256 reserved device numbers.  Then register a class
	 * that will key udev/mdev to add/remove /dev nodes.  Last, register
	 * the driver which manages those device numbers.
	 */
	BUILD_BUG_ON(N_SPI_MINORS > 256);
	status = register_chrdev(SPIDEV_MAJOR, "spi-hi3593", &spidev_fops);
	if (status < 0)
		return status;

	spidev_class = class_create(THIS_MODULE, "spi-hi3593");
	if (IS_ERR(spidev_class)) {
		unregister_chrdev(SPIDEV_MAJOR, spidev_spi_driver.driver.name);
		return PTR_ERR(spidev_class);
	}

	status = spi_register_driver(&spidev_spi_driver);
	if (status < 0) {
		class_destroy(spidev_class);
		unregister_chrdev(SPIDEV_MAJOR, spidev_spi_driver.driver.name);
	}
	return status;
}
module_init(spidev_init);

static void __exit spidev_exit(void)
{
	spi_unregister_driver(&spidev_spi_driver);
	class_destroy(spidev_class);
	unregister_chrdev(SPIDEV_MAJOR, spidev_spi_driver.driver.name);
}
module_exit(spidev_exit);

MODULE_AUTHOR("XATC");
MODULE_DESCRIPTION("User mode SPI device interface for hi3593");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:spidev-hi3593");
