/*
 * Copyright 2018 NXP
 * Copyright 2020-2025 TQ-Systems GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>

#include "platform_def.h"

static const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = U(0x80010412),
	.cs[0].bnds = U(0x7F),
	.timing_cfg[0] = U(0x90550018),
	.timing_cfg[1] = U(0xCCC60C52),
	.timing_cfg[2] = U(0x0048C11C),
	.timing_cfg[3] = U(0x01111000),
	.timing_cfg[4] = U(0x00000002),
	.timing_cfg[7] = U(0x13300000),
	.timing_cfg[8] = U(0x00006600),
	.sdram_cfg[0] = U(0xE50C0004),
	.sdram_cfg[1] = U(0x00401010),
	.sdram_cfg[2] = U(0x49000000),
	.sdram_mode[0] = U(0x01010214),
	.sdram_mode[8] = U(0x00000500),
	.sdram_mode[9] = U(0x04000000),
	.interval = U(0x0C300618),
	.data_init = U(0xDEADBEEF),
	.clk_cntl = U(0x02000000),
	.zq_cntl = U(0x8A090705),
	.wrlvl_cntl[0] = U(0x8675E605),
	.wrlvl_cntl[1] = U(0x05060600),
	.wrlvl_cntl[2] = U(0x00000006),
	.dq_map[0] = U(0x5B65B630),
	.dq_map[1] = U(0xB16D8000),
	.dq_map[3] = U(0x00100000),
	.cdr[0] = U(0x80040000),
	.cdr[1] = U(0x00000081),
};

long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_1600, sizeof(static_1600));

	return ULL(0x80000000);
}

long long init_ddr(void)
{
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);

	zeromem(&info, sizeof(struct ddr_info));
	info.num_ctlrs = NUM_OF_DDRC;
	info.dimm_on_ctlr = DDRC_NUM_DIMM;
	info.clk = get_ddr_freq(&sys, 0);
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);

	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

	return dram_size;
}
