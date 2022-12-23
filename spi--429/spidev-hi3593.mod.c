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
	{ 0x65ffa89, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6d044c26, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0xf76d8e4, __VMLINUX_SYMBOL_STR(no_llseek) },
	{ 0x64bad7cb, __VMLINUX_SYMBOL_STR(driver_unregister) },
	{ 0xe6d864e8, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xe7b5078b, __VMLINUX_SYMBOL_STR(spi_register_driver) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xc990c07, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xc2091dc2, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x52ab4b26, __VMLINUX_SYMBOL_STR(spi_register_board_info) },
	{ 0x42a72e93, __VMLINUX_SYMBOL_STR(complete) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x5397c01e, __VMLINUX_SYMBOL_STR(__copy_user) },
	{ 0x8d5ed932, __VMLINUX_SYMBOL_STR(wait_for_completion) },
	{ 0xa5d39c07, __VMLINUX_SYMBOL_STR(spi_async) },
	{ 0x72f1b9d, __VMLINUX_SYMBOL_STR(nonseekable_open) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x6ceebdfd, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x479c3c86, __VMLINUX_SYMBOL_STR(find_next_zero_bit) },
	{ 0xffce3a1, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x5942bfd2, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x27faf83c, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x6d67711d, __VMLINUX_SYMBOL_STR(devm_ioremap) },
	{ 0x373c3ed, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xd53e4d0, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0x18bc7516, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0x10394cf8, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irq) },
	{ 0xfba14fec, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irq) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xa012ba66, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0xbd0d78df, __VMLINUX_SYMBOL_STR(mutex_lock) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cspi-hi3593*");
MODULE_INFO(rhelversion, "7.4");
#ifdef RETPOLINE
	MODULE_INFO(retpoline, "Y");
#endif
