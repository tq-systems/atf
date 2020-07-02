#
# Copyright (c) 2018-2019, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PROGRAMMABLE_RESET_ADDRESS	:= 0
COLD_BOOT_SINGLE_CPU		:= 1
ARM_CCI_PRODUCT_ID		:= 500
TRUSTED_BOARD_BOOT		:= 1
RESET_TO_BL31			:= 1
GENERATE_COT			:= 1
BL2_AT_EL3			:= 1
ENABLE_SVE_FOR_NS		:= 0
MULTI_CONSOLE_API		:= 1

CRASH_REPORTING			:= 1
HANDLE_EA_EL3_FIRST		:= 1

$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

ifeq (${SPD},none)
  SPD_NONE:=1
  $(eval $(call add_define,SPD_NONE))
endif

# LSI setting common define
RCAR_M3:=1
RCAR_M3N:=2
RCAR_E3:=3
RCAR_H3N:=4
RCAR_AUTO:=99
$(eval $(call add_define,RCAR_M3))
$(eval $(call add_define,RCAR_M3N))
$(eval $(call add_define,RCAR_E3))
$(eval $(call add_define,RCAR_H3N))
$(eval $(call add_define,RCAR_AUTO))
RCAR_CUT_10:=0
RCAR_CUT_11:=1
RCAR_CUT_13:=3
RCAR_CUT_20:=10
RCAR_CUT_30:=20
$(eval $(call add_define,RCAR_CUT_10))
$(eval $(call add_define,RCAR_CUT_11))
$(eval $(call add_define,RCAR_CUT_13))
$(eval $(call add_define,RCAR_CUT_20))
$(eval $(call add_define,RCAR_CUT_30))

ifndef LSI
  $(error "Error: Unknown LSI. Please use LSI=<LSI name> to specify the LSI")
else
  ifeq (${LSI},AUTO)
    RCAR_LSI:=${RCAR_AUTO}
  else ifeq (${LSI},G2H)
    RCAR_LSI:=${RCAR_H3N}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},30)
        RCAR_LSI_CUT:=20
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},G2M)
    RCAR_LSI:=${RCAR_M3}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      else ifeq (${LSI_CUT},11)
        RCAR_LSI_CUT:=1
      else ifeq (${LSI_CUT},13)
        RCAR_LSI_CUT:=3
      else ifeq (${LSI_CUT},30)
        RCAR_LSI_CUT:=20
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},G2N)
    RCAR_LSI:=${RCAR_M3N}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      else ifeq (${LSI_CUT},11)
        RCAR_LSI_CUT:=1
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},G2E)
    RCAR_LSI:=${RCAR_E3}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      else ifeq (${LSI_CUT},11)
        RCAR_LSI_CUT:=1
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else
    $(error "Error: ${LSI} is not supported.")
  endif
  $(eval $(call add_define,RCAR_LSI))
endif

# lock RPC HYPERFLASH access by default
# unlock to repogram the ATF firmware from u-boot
ifndef RCAR_RPC_HYPERFLASH_LOCKED
RCAR_RPC_HYPERFLASH_LOCKED := 1
endif
$(eval $(call add_define,RCAR_RPC_HYPERFLASH_LOCKED))

# Process RZG2_SECURE_BOOT flag
ifndef RZG2_SECURE_BOOT
RZG2_SECURE_BOOT := 0
endif
$(eval $(call add_define,RZG2_SECURE_BOOT))

# Process RCAR_QOS_TYPE flag
ifndef RCAR_QOS_TYPE
RCAR_QOS_TYPE := 0
endif
$(eval $(call add_define,RCAR_QOS_TYPE))

# Process RCAR_DRAM_SPLIT flag
ifndef RCAR_DRAM_SPLIT
RCAR_DRAM_SPLIT := 0
endif
$(eval $(call add_define,RCAR_DRAM_SPLIT))

# Process RCAR_BL33_EXECUTION_EL flag
ifndef RCAR_BL33_EXECUTION_EL
RCAR_BL33_EXECUTION_EL := 0
endif
$(eval $(call add_define,RCAR_BL33_EXECUTION_EL))

# Process RCAR_AVS_SETTING_ENABLE flag
ifeq (${RCAR_AVS_SETTING_ENABLE},0)
AVS_SETTING_ENABLE := 0
else
AVS_SETTING_ENABLE := 1
endif
$(eval $(call add_define,AVS_SETTING_ENABLE))

# Process RCAR_LOSSY_ENABLE flag
ifndef RCAR_LOSSY_ENABLE
RCAR_LOSSY_ENABLE := 0
endif
$(eval $(call add_define,RCAR_LOSSY_ENABLE))

# Process LIFEC_DBSC_PROTECT_ENABLE flag
ifndef LIFEC_DBSC_PROTECT_ENABLE
LIFEC_DBSC_PROTECT_ENABLE := 1
endif
$(eval $(call add_define,LIFEC_DBSC_PROTECT_ENABLE))

# Process PMIC_ROHM_BD9571 flag
ifndef PMIC_ROHM_BD9571
PMIC_ROHM_BD9571 := 1
endif
$(eval $(call add_define,PMIC_ROHM_BD9571))

# Process PMIC_LEVEL_MODE flag
ifndef PMIC_LEVEL_MODE
PMIC_LEVEL_MODE := 1
endif
$(eval $(call add_define,PMIC_LEVEL_MODE))

# Process RCAR_GEN3_ULCB flag
ifndef RCAR_GEN3_ULCB
RCAR_GEN3_ULCB := 0
endif
ifeq (${RCAR_GEN3_ULCB},1)
 BOARD_DEFAULT := 0x10
 $(eval $(call add_define,BOARD_DEFAULT))
endif
$(eval $(call add_define,RCAR_GEN3_ULCB))

# Process RCAR_REF_INT flag
ifndef RCAR_REF_INT
RCAR_REF_INT :=0
endif
$(eval $(call add_define,RCAR_REF_INT))

# Process RCAR_REWT_TRAINING flag
ifndef RCAR_REWT_TRAINING
RCAR_REWT_TRAINING := 1
endif
$(eval $(call add_define,RCAR_REWT_TRAINING))

# Process RCAR_SYSTEM_SUSPEND flag
ifndef RCAR_SYSTEM_SUSPEND
RCAR_SYSTEM_SUSPEND := 1
endif
$(eval $(call add_define,RCAR_SYSTEM_SUSPEND))

# SYSTEM_SUSPEND requires power control of PMIC etc.
# When executing SYSTEM_SUSPEND other than Salvator-X, Salvator-XS and Ebisu,
# processing equivalent to that implemented in PMIC_ROHM_BD9571 is necessary.
ifeq (${RCAR_SYSTEM_SUSPEND},1)
  ifeq (${PMIC_ROHM_BD9571},0)
    $(error "Error: When you want RCAR_SYSTEM_SUSPEND to be enable, please also set PMIC_ROHM_BD9571 to enable.")
  endif
endif

# Process RCAR_DRAM_LPDDR4_MEMCONF flag
ifndef RCAR_DRAM_LPDDR4_MEMCONF
RCAR_DRAM_LPDDR4_MEMCONF :=1
endif
$(eval $(call add_define,RCAR_DRAM_LPDDR4_MEMCONF))

# Process RCAR_DRAM_DDR3L_MEMCONF flag
ifndef RCAR_DRAM_DDR3L_MEMCONF
RCAR_DRAM_DDR3L_MEMCONF :=1
endif
$(eval $(call add_define,RCAR_DRAM_DDR3L_MEMCONF))

# Process RCAR_DRAM_DDR3L_MEMDUAL flag
ifndef RCAR_DRAM_DDR3L_MEMDUAL
RCAR_DRAM_DDR3L_MEMDUAL :=1
endif
$(eval $(call add_define,RCAR_DRAM_DDR3L_MEMDUAL))

# Process RZG_DRAM_ECC flag
ifndef RZG_DRAM_ECC
RZG_DRAM_ECC :=0
endif
$(eval $(call add_define,RZG_DRAM_ECC))

# Process RZG_DRAM_ECC_FULL flag
# 0 : ECC Full mode will not be applied
# 1 : ECC Full mode dual channel will be applied
# 2 : ECC Full mode single channel will be applied
ifndef RZG_DRAM_ECC_FULL
RZG_DRAM_ECC_FULL :=0
endif
$(eval $(call add_define,RZG_DRAM_ECC_FULL))

# Process RCAR_BL33_ARG0 flag
ifdef RCAR_BL33_ARG0
$(eval $(call add_define,RCAR_BL33_ARG0))
endif

#Process RCAR_BL2_DCACHE flag
ifndef RCAR_BL2_DCACHE
RCAR_BL2_DCACHE := 0
endif
$(eval $(call add_define,RCAR_BL2_DCACHE))

# Process RCAR_DRAM_CHANNEL flag
ifndef RCAR_DRAM_CHANNEL
RCAR_DRAM_CHANNEL :=15
endif
$(eval $(call add_define,RCAR_DRAM_CHANNEL))

#Process RCAR_SYSTEM_RESET_KEEPON_DDR flag
ifndef RCAR_SYSTEM_RESET_KEEPON_DDR
RCAR_SYSTEM_RESET_KEEPON_DDR := 0
endif
$(eval $(call add_define,RCAR_SYSTEM_RESET_KEEPON_DDR))

# RCAR_SYSTEM_RESET_KEEPON_DDR requires power control of PMIC etc.
# When executing SYSTEM_SUSPEND other than Salvator-X, Salvator-XS and Ebisu,
# processing equivalent to that implemented in PMIC_ROHM_BD9571 is necessary.
# Also, it is necessary to enable RCAR_SYSTEM_SUSPEND.
ifeq (${RCAR_SYSTEM_RESET_KEEPON_DDR},1)
  ifeq (${PMIC_ROHM_BD9571},0)
    $(error "Error: When you want RCAR_SYSTEM_RESET_KEEPON_DDR to be enable, please also set PMIC_ROHM_BD9571 to enable.")
  endif
  ifeq (${RCAR_SYSTEM_SUSPEND},0)
    $(error "Error: When you want RCAR_SYSTEM_RESET_KEEPON_DDR to be enable, please also set RCAR_SYSTEM_SUSPEND to enable.")
  endif
endif

#Process EK874 flag
ifndef RZG_EK874
RZG_EK874 := 0
endif
$(eval $(call add_define,RZG_EK874))

#Process HIHOPE RZ/G2M flag
ifndef RZG_HIHOPE_RZG2M
RZG_HIHOPE_RZG2M := 0
endif
$(eval $(call add_define,RZG_HIHOPE_RZG2M))

#Process HIHOPE RZ/G2N flag
ifndef RZG_HIHOPE_RZG2N
RZG_HIHOPE_RZG2N := 0
endif
$(eval $(call add_define,RZG_HIHOPE_RZG2N))

#Process HIHOPE RZ/G2H flag
ifndef RZG_HIHOPE_RZG2H
RZG_HIHOPE_RZG2H := 0
endif
$(eval $(call add_define,RZG_HIHOPE_RZG2H))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_835769  := 1
ifdef ERRATA_A53_843419
ERRATA_A53_843419  := 1
else
TF_LDFLAGS_aarch64	+= --fix-cortex-a53-843419
endif
ERRATA_A53_855873  := 1

# Enable workarounds for selected Cortex-A57 erratas.
ERRATA_A57_859972  := 1
ERRATA_A57_813419  := 1

include drivers/renesas/rzg/ddr/ddr.mk
include drivers/renesas/rzg/qos/qos.mk
include drivers/renesas/rzg/pfc/pfc.mk
include lib/libfdt/libfdt.mk

PLAT_INCLUDES	:=	-Idrivers/renesas/rzg/ddr		\
			-Idrivers/renesas/rzg/qos		\
			-Idrivers/renesas/rcar/iic_dvfs		\
			-Idrivers/renesas/rzg/board		\
			-Idrivers/renesas/rcar/cpld/		\
			-Idrivers/renesas/rcar/avs		\
			-Idrivers/renesas/rcar/delay		\
			-Idrivers/renesas/rcar/rom		\
			-Idrivers/renesas/rcar/scif		\
			-Idrivers/renesas/rzg/emmc		\
			-Idrivers/renesas/rcar/pwrc		\
			-Idrivers/renesas/rzg/io		\
			-Iplat/renesas/rzg/include		\
			-Iplat/renesas/rcar/include/registers	\
			-Iplat/renesas/rcar/include		\
			-Iplat/renesas/rcar

PLAT_BL_COMMON_SOURCES	:=	drivers/renesas/rcar/iic_dvfs/iic_dvfs.c \
				plat/renesas/rcar/rcar_common.c

RCAR_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

BL2_SOURCES	+=	${RCAR_GIC_SOURCES}				\
			lib/cpus/aarch64/cortex_a53.S			\
			lib/cpus/aarch64/cortex_a57.S			\
			${LIBFDT_SRCS}					\
			common/desc_image_load.c			\
			plat/renesas/rcar/aarch64/platform_common.c	\
			plat/renesas/rcar/aarch64/plat_helpers.S	\
			plat/renesas/rcar/bl2_interrupt_error.c		\
			plat/renesas/rcar/bl2_secure_setting.c		\
			plat/renesas/rzg/bl2_plat_setup.c		\
			plat/renesas/rzg/bl2_fusa.c			\
			plat/renesas/rzg/plat_storage.c			\
			plat/renesas/rzg/bl2_plat_mem_params_desc.c	\
			plat/renesas/rcar/plat_image_load.c		\
			plat/renesas/rcar/bl2_cpg_init.c		\
			drivers/renesas/rcar/console/rcar_printf.c	\
			drivers/renesas/rcar/scif/scif.S		\
			drivers/renesas/rcar/common.c			\
			drivers/renesas/rzg/io/io_emmcdrv.c		\
			drivers/renesas/rzg/io/io_memdrv.c		\
			drivers/renesas/rzg/io/io_rcar.c		\
			drivers/renesas/rzg/auth/auth_mod.c		\
			drivers/renesas/rcar/rpc/rpc_driver.c		\
			drivers/renesas/rcar/dma/dma_driver.c		\
			drivers/renesas/rcar/avs/avs_driver.c		\
			drivers/renesas/rcar/delay/micro_delay.c	\
			drivers/renesas/rzg/emmc/emmc_interrupt.c	\
			drivers/renesas/rzg/emmc/emmc_utility.c	\
			drivers/renesas/rzg/emmc/emmc_mount.c		\
			drivers/renesas/rzg/emmc/emmc_init.c		\
			drivers/renesas/rzg/emmc/emmc_read.c		\
			drivers/renesas/rzg/emmc/emmc_cmd.c		\
			drivers/renesas/rcar/watchdog/swdt.c		\
			drivers/renesas/rcar/rom/rom_api.c		\
			drivers/renesas/rzg/board/board.c		\
			drivers/io/io_storage.c

BL31_SOURCES	+=	${RCAR_GIC_SOURCES}				\
			lib/cpus/aarch64/cortex_a53.S			\
			lib/cpus/aarch64/cortex_a57.S			\
			plat/common/plat_psci_common.c			\
			plat/renesas/rcar/plat_topology.c		\
			plat/renesas/rcar/aarch64/plat_helpers.S	\
			plat/renesas/rcar/aarch64/platform_common.c	\
			plat/renesas/rcar/bl31_plat_setup.c		\
			plat/renesas/rcar/plat_pm.c			\
			drivers/renesas/rcar/console/rcar_console.S	\
			drivers/renesas/rcar/console/rcar_printf.c	\
			drivers/renesas/rcar/delay/micro_delay.c	\
			drivers/renesas/rcar/pwrc/call_sram.S		\
			drivers/renesas/rcar/pwrc/pwrc.c		\
			drivers/renesas/rcar/common.c			\
			drivers/arm/cci/cci.c

ifeq (${RCAR_GEN3_ULCB},1)
BL31_SOURCES		+=	drivers/renesas/rcar/cpld/ulcb_cpld.c
endif

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/auth/mbedtls/mbedtls_crypto.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

# build the layout images for the bootrom and the necessary srecords
rzg: rzg_layout_create rzg_srecord
distclean realclean clean: clean_layout_tool clean_srecord

# layout images
LAYOUT_TOOLPATH ?= tools/renesas/rzg_layout_create

clean_layout_tool:
	@echo "clean layout tool"
	${Q}${MAKE} -C ${LAYOUT_TOOLPATH} clean

.PHONY: rzg_layout_create
rzg_layout_create:
	@echo "generating layout srecs"
	${Q}${MAKE} CPPFLAGS="-D=AARCH64" ARCH=$(ARCH) --no-print-directory -C ${LAYOUT_TOOLPATH}

# srecords
SREC_PATH	= ${BUILD_PLAT}
BL2_ELF_SRC	= ${SREC_PATH}/bl2/bl2.elf
BL31_ELF_SRC	= ${SREC_PATH}/bl31/bl31.elf

clean_srecord:
	@echo "clean bl2 and bl31 srecs"
	rm -f ${SREC_PATH}/bl2.srec ${SREC_PATH}/bl31.srec

.PHONY: rzg_srecord
rzg_srecord: $(BL2_ELF_SRC) $(BL31_ELF_SRC)
	@echo "generating srec: ${SREC_PATH}/bl2.srec"
	$(Q)$(OC) -O srec --srec-forceS3 ${BL2_ELF_SRC}  ${SREC_PATH}/bl2.srec
	@echo "generating srec: ${SREC_PATH}/bl31.srec"
	$(Q)$(OC) -O srec --srec-forceS3 ${BL31_ELF_SRC} ${SREC_PATH}/bl31.srec

