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
	{ 0x1976aa06, __VMLINUX_SYMBOL_STR(param_ops_bool) },
	{ 0x71c10636, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0xdeed7eed, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0xd56d9756, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0x1f9178c3, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x2072ee9b, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x28e5abc7, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x10394cf8, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irq) },
	{ 0xfba14fec, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irq) },
	{ 0xe4dd5b3e, __VMLINUX_SYMBOL_STR(down_trylock) },
	{ 0x354ef288, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0x7efb2701, __VMLINUX_SYMBOL_STR(prepare_to_wait) },
	{ 0xc8b57c27, __VMLINUX_SYMBOL_STR(autoremove_wake_function) },
	{ 0x5397c01e, __VMLINUX_SYMBOL_STR(__copy_user) },
	{ 0xe2cc62c8, __VMLINUX_SYMBOL_STR(down_interruptible) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0x5a6c709e, __VMLINUX_SYMBOL_STR(up) },
	{ 0xf2d2059c, __VMLINUX_SYMBOL_STR(down) },
	{ 0x4302d0eb, __VMLINUX_SYMBOL_STR(free_pages) },
	{ 0xdad297c3, __VMLINUX_SYMBOL_STR(mips_dma_map_ops) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xa4b639fb, __VMLINUX_SYMBOL_STR(devm_regmap_init_i2c) },
	{ 0xd53e4d0, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0x85cdb080, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x4c433586, __VMLINUX_SYMBOL_STR(regmap_update_bits_check) },
	{ 0x1c9b92cb, __VMLINUX_SYMBOL_STR(i2c_unregister_device) },
	{ 0xd62c833f, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0x9e7d6bd0, __VMLINUX_SYMBOL_STR(__udelay) },
	{ 0x18bc7516, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0x93e351e3, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x2c9c0ab0, __VMLINUX_SYMBOL_STR(i2c_register_driver) },
	{ 0x50f6cb4, __VMLINUX_SYMBOL_STR(register_sound_mixer) },
	{ 0x4bd11dbf, __VMLINUX_SYMBOL_STR(register_sound_dsp) },
	{ 0x1a5b4b17, __VMLINUX_SYMBOL_STR(platform_get_irq) },
	{ 0xa5f389eb, __VMLINUX_SYMBOL_STR(of_property_read_u32_array) },
	{ 0x659a929a, __VMLINUX_SYMBOL_STR(dma_request_slave_channel) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x1fedf0f4, __VMLINUX_SYMBOL_STR(__request_region) },
	{ 0x260cf8f8, __VMLINUX_SYMBOL_STR(of_get_property) },
	{ 0xb7ceeb99, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x5a025f7b, __VMLINUX_SYMBOL_STR(arch_local_irq_restore) },
	{ 0x4fec81be, __VMLINUX_SYMBOL_STR(arch_local_irq_save) },
	{ 0x6dfb37b6, __VMLINUX_SYMBOL_STR(regmap_write) },
	{ 0xcb55ee3e, __VMLINUX_SYMBOL_STR(regmap_read) },
	{ 0x7c61340c, __VMLINUX_SYMBOL_STR(__release_region) },
	{ 0x69a358a6, __VMLINUX_SYMBOL_STR(iomem_resource) },
	{ 0xed770447, __VMLINUX_SYMBOL_STR(platform_get_resource) },
	{ 0x7afc9d8a, __VMLINUX_SYMBOL_STR(unregister_sound_mixer) },
	{ 0xcd083b10, __VMLINUX_SYMBOL_STR(unregister_sound_dsp) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Ces8311,ls-pcm-audio*");
MODULE_ALIAS("i2c:es8311,ls-pcm-audio");
MODULE_INFO(rhelversion, "7.4");
#ifdef RETPOLINE
	MODULE_INFO(retpoline, "Y");
#endif
