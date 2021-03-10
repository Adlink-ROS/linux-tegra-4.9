old-dtb := $(dtb-y)
old-dtbo := $(dtbo-y)
dtb-y :=
dtbo-y :=
makefile-path := platform/t210/rqp_nano/kernel-dts

dtb-$(CONFIG_ARCH_TEGRA_210_SOC) += tegra210-rqp-nano.dtb
dtb-$(CONFIG_ARCH_TEGRA_210_SOC) += tegra210-rqp-nano-for-rcm-boot.dtb

ifneq ($(dtb-y),)
dtb-y := $(addprefix $(makefile-path)/,$(dtb-y))
endif
ifneq ($(dtbo-y),)
dtbo-y := $(addprefix $(makefile-path)/,$(dtbo-y))
endif

dtb-y += $(old-dtb)
dtbo-y += $(old-dtbo)
