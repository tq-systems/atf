// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2018-2021 NXP
 * Copyright (c) 2023 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 *
 */

#include <string.h>
#include <common/debug.h>
#include <lib/utils.h>

#include "ddr.h"
#include "errata.h"
#include "platform_def.h"

#ifdef CONFIG_STATIC_DDR
struct ddr_cfg_regs static_2000 = {
	.cs[0].bnds = 0x0000007F,
	.cs[0].config = 0x80010312,
	.timing_cfg[0] = 0x77660008,
	.timing_cfg[1] = 0xF1FCC265,
	.timing_cfg[2] = 0x0059415E,
	.timing_cfg[3] = 0x020F1100,
	.sdram_cfg[0] = 0x65000000,
	.sdram_cfg[1] = 0x00401150,
	.sdram_cfg[2] =  0x00000000,
	.sdram_mode[0] =  0x03010625,
	.sdram_mode[1] = 0x00100200,
	.sdram_mode[2] = 0x00010625,
	.sdram_mode[3] = 0x00100200,
	.sdram_mode[4] = 0x00010625,
	.sdram_mode[5] = 0x00100200,
	.sdram_mode[6] = 0x00010625,
	.sdram_mode[7] = 0x00100200,
	.sdram_mode[8] = 0x00000500,
	.sdram_mode[9] = 0x04400000,
	.sdram_mode[10] = 0x00000400,
	.sdram_mode[11] = 0x04400000,
	.sdram_mode[12] = 0x00000400,
	.sdram_mode[13] = 0x04400000,
	.sdram_mode[14] = 0x00000400,
	.sdram_mode[15] = 0x04400000,
	.interval = 0x0F3C0000,
	.clk_cntl = 0x02000000,
	.timing_cfg[4] = 0x00224002,
	.timing_cfg[5] = 0x04401400,
	.timing_cfg[6] = 0x00000000,
	.timing_cfg[7] = 0x25500000,
	.timing_cfg[8] = 0x03335A00,
	.timing_cfg[9] = 0x00000000,
	.zq_cntl = 0x8A090705,
	.wrlvl_cntl[0] = 0x86550609,
	.wrlvl_cntl[1] = 0x09080806,
	.wrlvl_cntl[2] = 0x06040409,
	.cdr[0] = 0x80080000,
	.cdr[1] = 0x000000C0,
	.dq_map[0] = 0x00000000,
	.dq_map[1] = 0x00000000,
	.dq_map[2] = 0x00000000,
	.dq_map[3] = 0x00000000,
	.debug[28] = 0x00700046,
};

long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_2000, sizeof(static_2000));

	return 0x80000000;
}

#endif

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
		ERROR("DDR init failed\n");

	return dram_size;
}
