/*
 * Copyright 2018-2022 NXP
 * Copyright 2022-2025 TQ-Systems GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <fsl_mmdc.h>

#include "platform_def.h"

long long init_ddr(void)
{
	/* 1GiB variant */
	static const struct fsl_mmdc_info mparam = {
		.mdctl = U(0x05180000),
		.mdpdc = U(0x00030036),
		.mdotc = U(0x12444040),
		.mdcfg0 = U(0xAAB47994),
		.mdcfg1 = U(0xDB328F84),
		.mdcfg2 = U(0x01FF0124),
		.mdmisc = U(0x00200680),
		.mdref = U(0x079E8000),
		.mdrwd = U(0x00002000),
		.mdor = 0x00B41023,
		.mdasp = U(0x0000003f),
		.mpodtctrl = U(0x0000022A),
		.mpzqhwctrl = U(0xA1390003),
	};

	mmdc_init(&mparam, NXP_DDR_ADDR);
	NOTICE("DDR 1 GiB Init Done\n");

	return NXP_DRAM0_SIZE;
}
