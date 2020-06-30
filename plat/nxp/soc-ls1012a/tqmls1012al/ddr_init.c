/*
 * Copyright 2018, 2021 NXP
 * Copyright 2022 TQ-Systems GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <errno.h>
#include <fsl_mmdc.h>
#include <plat_common.h>
#include <platform_def.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long long init_ddr(void)
{
	/* 512MB variant */
	static const struct fsl_mmdc_info mparam = {
		.mdctl = 0x04180000,
		.mdpdc = 0x00030036,
		.mdotc = 0x12444040,
		.mdcfg0 = 0x82877994,
		.mdcfg1 = 0xDB328F84,
		.mdcfg2 = 0x01FF0124,
		.mdmisc = 0x00200680,
		.mdref = 0x079E8000,
		.mdrwd = 0x00002000,
		.mdor = 0x00871023,
		.mdasp = 0x0000003f,
		.mpodtctrl = 0x0000022A,
		.mpzqhwctrl = 0xA1390003,
	};

	mmdc_init(&mparam, NXP_DDR_ADDR);
	NOTICE("DDR Init Done\n");

	return NXP_DRAM0_SIZE;
}
