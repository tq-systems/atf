#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>
#	Pankaj Gupta <pankaj.gupta@nxp.com>

# board-specific build parameters
BOOT_MODE	:= 	flexspi_nor
BOARD		:=	tqmlx2160a
POVDD_ENABLE	:=	no
NXP_COINED_BB	:=	no

 # DDR Compilation Configs
NUM_OF_DDRC		:=	2
CONFIG_DDR_NODIMM	:=	1
DDRC_NUM_DIMM		:=	2
DDRC_NUM_CS		:=	4
DDR_ECC_EN		:=	yes
 #enable address decoding feature
DDR_ADDR_DEC	:=	yes
APPLY_MAX_CDD	:=	yes

 # On-Board Flash Details
FLASH_TYPE	:=	MT35XU512A
XSPI_FLASH_SZ	:=	0x10000000
NXP_XSPI_NOR_UNIT_SIZE		:=	0x20000
BL2_BIN_XSPI_NOR_END_ADDRESS	:=	0x100000
# CONFIG_FSPI_ERASE_4K is required to erase 4K sector sizes. This
# config is enabled for future use cases.
FSPI_ERASE_4K	:= 0

 # Platform specific features.
WARM_BOOT	:=	no

SUPPORTED_BOOT_MODE	:=	flexspi_nor	\
				sd		\
				emmc		\
				mmcsd		\
				auto

BOARD_PATH	:=	${PLAT_SOC_PATH}/${BOARD}
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

# Adding platform board build info
include plat/nxp/common/plat_common_def.mk

 # get SoC common build parameters
include plat/nxp/soc-lx2160a/soc.mk


