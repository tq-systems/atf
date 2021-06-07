/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author York Sun <york.sun@nxp.com>
 */

#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <debug.h>
#include <errno.h>
#include <string.h>
#include <io.h>
#include <fsl_mmdc.h>

long long _init_ddr(void)
{
	/* 1GB variant */
	static const struct fsl_mmdc_info mparam = {
		.mdctl = 0x05180000,
		.mdpdc = 0x00030036,
		.mdotc = 0x12444040,
		.mdcfg0 = 0xAAB47994,
		.mdcfg1 = 0xDB328F84,
		.mdcfg2 = 0x01FF0124,
		.mdmisc = 0x00200680,
		.mdref = 0x079E8000,
		.mdrwd = 0x00002000,
		.mdor = 0x00B41023,
		.mdasp = 0x0000003f,
		.mpodtctrl = 0x0000022A,
		.mpzqhwctrl = 0xA1390003,
	};

	mmdc_init(&mparam);
	NOTICE("DDR 1 GB Init Done\n");

	/* Need to see there is any other way to
	 * deduce the total DRAM size rather
	 * hard-coding it.
	 */
	return NXP_DRAM0_SIZE;
}
