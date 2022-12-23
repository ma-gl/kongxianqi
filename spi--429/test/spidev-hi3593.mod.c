#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xba4952ac, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6d044c26, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xa65ce857, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xb59d8a, __VMLINUX_SYMBOL_STR(driver_unregister) },
	{ 0x52ab4b26, __VMLINUX_SYMBOL_STR(spi_register_board_info) },
	{ 0x1467f1b9, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x479c3c86, __VMLINUX_SYMBOL_STR(find_next_zero_bit) },
	{ 0x50338c2c, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x5942bfd2, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x27faf83c, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x75becfa4, __VMLINUX_SYMBOL_STR(devm_ioremap) },
	{ 0xb107ec1e, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xbc0df171, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x894a556f, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x10394cf8, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irq) },
	{ 0x86344924, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0xfba14fec, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irq) },
	{ 0x1306c792, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cspi-hi3593*");
MODULE_INFO(rhelversion, "7.4");
