/*
 * Copyright 2018 NXP
 * Copyright 2020-2022 TQ-Systems GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>
#include <common/debug.h>
#include <utils.h>

#include <ddr.h>
#include <platform_def.h>

const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = 0x80010312,
	.cs[0].bnds = 0x3F,
	.timing_cfg[0] = 0x90550018,
	.timing_cfg[1] = 0xBCB48C52,
	.timing_cfg[2] = 0x0048C11C,
	.timing_cfg[3] = 0x010C1000,
	.timing_cfg[4] = 0x00000002,
	.timing_cfg[7] = 0x13300000,
	.timing_cfg[8] = 0x00006600,
	.sdram_cfg[0] = 0xE50C0004,
	.sdram_cfg[1] = 0x00401110,
	.sdram_cfg[2] = 0x49000000,
	.sdram_mode[0] = 0x01010210,
	.sdram_mode[8] = 0x00000500,
	.sdram_mode[9] = 0x04000000,
	.interval = 0x0C300618,
	.data_init = 0xDEADBEEF,
	.clk_cntl = 0x02000000,
	.zq_cntl = 0x8A090705,
	.wrlvl_cntl[0] = 0x8675E605,
	.wrlvl_cntl[1] = 0x05060600,
	.wrlvl_cntl[2] = 0x00000006,
	.dq_map[0] = 0x5B65B630,
	.dq_map[1] = 0xB16D8000,
	.dq_map[3] = 0x00100000,
	.cdr[0] = 0x80040000,
	.cdr[1] = 0x00000081,
};

long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_1600, sizeof(static_1600));

	return 0x40000000ULL;
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
	info.num_ctlrs = 1;
	info.dimm_on_ctlr = 1;
	info.clk = get_ddr_freq(&sys, 0);
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);

	if (dram_size < 0)
		ERROR("DDR init failed.\n");

	return dram_size;
}
