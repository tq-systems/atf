#
# Copyright 2018 NXP
# Copyright 2020 TQ-Systems GmbH
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Author Ruchika Gupta <ruchika.gupta@nxp.com>

# board-specific build parameters
BOOT_MODE	:= 	flexspi_nor
BOARD		:=	tqmls1028a

 # get SoC common build parameters
include plat/nxp/soc-ls1028/soc.mk

BOARD_PATH	:=	${PLAT_SOC_PATH}/${BOARD}
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

