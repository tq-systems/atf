#
# Copyright 2018-2021 NXP
# Copyright 2022 TQ-Systems GmbH
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Board-specific build parameters
BOOT_MODE	?=	flexspi_nor
BOARD		:=	tqmls1028a
POVDD_ENABLE	:=	no
WARM_BOOT	:=	no

# DDR build parameters
NUM_OF_DDRC		:=	1
CONFIG_STATIC_DDR	:=	1

FLASH_TYPE	:=	MX25U51245G
XSPI_FLASH_SZ	:=	0x4000000

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c \

SUPPORTED_BOOT_MODE	:=	flexspi_nor	\
				sd		\
				emmc

# Add platform board build info
include plat/nxp/common/plat_common_def.mk

# Add SoC build info
include plat/nxp/soc-ls1028a/soc.mk
