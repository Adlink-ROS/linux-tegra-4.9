old-dtb := $(dtb-y)
dtb-y :=
makefile-path := platform/t19x/nxn/kernel-dts

dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += adlink-roscube-pico-p3668-0001.dtb

ifneq ($(dtb-y),)
dtb-y := $(addprefix $(makefile-path)/,$(dtb-y))
endif

dtb-y += $(old-dtb)
