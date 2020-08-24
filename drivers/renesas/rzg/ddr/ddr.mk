#
# Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_E3})
    include drivers/renesas/rzg/ddr/ddr_a/ddr_a.mk
else
    include drivers/renesas/rzg/ddr/ddr_b/ddr_b.mk
endif

BL2_SOURCES += drivers/renesas/rzg/ddr/dram_sub_func.c
