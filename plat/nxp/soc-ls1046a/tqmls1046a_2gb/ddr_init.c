// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2018-2022 NXP
 *
 * Copyright (c) 2023-2025 TQ-Systems GmbH <oss@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>

#include <errata.h>
#include "platform_def.h"

#ifdef CONFIG_STATIC_DDR
static const struct ddr_cfg_regs static_2000 = {
	.cs[0].bnds = U(0x0000007F),
	.cs[0].config = U(0x80010312),
	.timing_cfg[0] = U(0xF7660008),
	.timing_cfg[1] = U(0xF1FC4178),
	.timing_cfg[2] = U(0x00590160),
	.timing_cfg[3] = U(0x020F1100),
	.sdram_cfg[0] = U(0x65000008),
	.sdram_cfg[1] = U(0x00401150),
	.sdram_cfg[2] = U(0x40000000),
	.sdram_mode[0] = U(0x01030631),
	.sdram_mode[1] = U(0x00100200),
	.sdram_mode[2] = U(0x00000000),
	.sdram_mode[3] = U(0x00000000),
	.sdram_mode[4] = U(0x00000000),
	.sdram_mode[5] = U(0x00000000),
	.sdram_mode[6] = U(0x00000000),
	.sdram_mode[7] = U(0x00000000),
	.sdram_mode[8] = U(0x00000500),
	.sdram_mode[9] = U(0x08800000),
	.sdram_mode[10] = U(0x00000000),
	.sdram_mode[11] = U(0x00000000),
	.sdram_mode[12] = U(0x00000000),
	.sdram_mode[13] = U(0x00000000),
	.sdram_mode[14] = U(0x00000000),
	.interval = U(0x0F3C079E),
	.clk_cntl = U(0x03000000),
	.timing_cfg[4] = U(0x00220002),
	.timing_cfg[5] = U(0x00000000),
	.timing_cfg[6] = U(0x00000000),
	.timing_cfg[7] = U(0x25500000),
	.timing_cfg[8] = U(0x05447A00),
	.timing_cfg[9] = U(0x00000000),
	.zq_cntl = U(0x8A090705),
	.wrlvl_cntl[0] = U(0x8605070A),
	.wrlvl_cntl[1] = U(0x0A080807),
	.wrlvl_cntl[2] = U(0x0706060A),
	.cdr[0] = U(0x80080000),
	.cdr[1] = U(0x000000C0),
	.dq_map[0] = U(0x00000000),
	.dq_map[1] = U(0x00000000),
	.dq_map[2] = U(0x00000000),
	.dq_map[3] = U(0x00000000),
	.debug[28] = U(0x00700046),
};

long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_2000, sizeof(static_2000));

#ifndef CONFIG_DDR_ECC_EN
	priv->ddr_reg.sdram_cfg[0] &= ~SDRAM_CFG_ECC_EN;
#endif

	return ULL(0x80000000);
}

#endif

long long init_ddr(void)
{
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys)) {
		ERROR("System clocks are not set\n");
		assert(0);
	}
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(struct ddr_info));
	info.num_ctlrs = NUM_OF_DDRC;
	info.dimm_on_ctlr = DDRC_NUM_DIMM;
	info.clk = get_ddr_freq(&sys, 0);
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);

	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

#ifdef ERRATA_SOC_A008850
	erratum_a008850_post();
#endif

	return dram_size;
}
