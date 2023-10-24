# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2018-2021 NXP
# Copyright (c) 2023 TQ-Systems GmbH <oss@tq-group.com>, D-82229 Seefeld, Germany.
# Author: Gregor Herburger

# board-specific build parameters
BOOT_MODE	?= 	qspi
BOARD		:=	tqmls1088a_4gb

# DDR Compilation Configs
NUM_OF_DDRC	:=	1
DDRC_NUM_DIMM	:=      1
DDR_ECC_EN	:=      yes
CONFIG_STATIC_DDR 	:=	1

# On-Board Flash Details
QSPI_FLASH_SZ	:=      0x4000000

# Adding Platform files build files
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

SUPPORTED_BOOT_MODE	:=	qspi \
				sd \
				emmc \
				mmcsd \
				auto

# Adding platform board build info
include plat/nxp/common/plat_common_def.mk

# Adding SoC build info
include plat/nxp/soc-ls1088a/soc.mk
