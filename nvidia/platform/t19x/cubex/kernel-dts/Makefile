old-dtb := $(dtb-y)
old-dtbo := $(dtbo-y)
dtb-y :=
dtbo-y :=
makefile-path := platform/t19x/cubex/kernel-dts

dtb-$(CONFIG_ARCH_TEGRA_19x_SOC) += tegra194-rqx-580.dtb
dtbo-$(CONFIG_ARCH_TEGRA_18x_SOC) += tegra194-rqx-580-hdr40.dtbo

ifneq ($(dtb-y),)
dtb-y := $(addprefix $(makefile-path)/,$(dtb-y))
endif
ifneq ($(dtbo-y),)
dtbo-y := $(addprefix $(makefile-path)/,$(dtbo-y))
endif

dtb-y += $(old-dtb)
dtbo-y += $(old-dtbo)
