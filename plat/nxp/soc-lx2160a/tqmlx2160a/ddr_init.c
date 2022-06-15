/*
 * Copyright 2018 NXP
 * Copyright 2020-2022 TQ-Systems GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Author York Sun <york.sun@nxp.com>
 * Author Gregor Herburger <gregor.herburger@tq-group.com>
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>
#include <load_img.h>

#include "plat_common.h"
#include <platform_def.h>

struct dimm_params ddr_raw_timing = {
	.n_ranks = 2,/* Number of ranks/ chip selects of DDR */
	.rank_density = 0x200000000u,/* this is size in one rank, here 8GB */
	.capacity = 0x400000000u,/* this is the total size, here 16GB */
	.primary_sdram_width = 64,/* this is the data bus width */
	.ec_sdram_width = 8,/* this is the ECC data width */
	.device_width = 8,/* 4->x4DRAM, 8->x8DRAM, 16->x16DRAM */
	.die_density = 5,/* this is each DRAM die density, here 16Gbit die density. 4->4Gbit, 5->8Gbit, 6->16Gbit*/
	.rdimm = 0,/* if register chip is used similar to an RDIMM = 1, otherwise = 0 */
	.mirrored_dimm = 1,/* =1 if C/A bus mirroring is used, all UDIMMs with two ranks are mirrored */
	.n_row_addr = 16,/* number of rows from dram datasheet */
	.n_col_addr = 10,/* number of columns from dram datasheet */
	.bank_addr_bits = 0,/* for DDR4 this is always = 0 defining two bits bank address in DRAM */
	.bank_group_bits = 2,/* for x16 dram = 1, 1-bit BG, for x8 dram = 2, 2-bits for BG */
	.edc_config = 2,/* 0->no ECC, 2-> ECC*/
	.burst_lengths_bitmask = 0x0c,/* leave as is, this is needed for uboot masking, does not change */
	.tckmin_x_ps = 625,/* tck min = 625ps from DRAM datasheet */
	.tckmax_ps = 1500,/* tck max = 1500ps from DRAM datasheet */
	.caslat_x = 0x00FFFC00,/* leave as is, this is needed for uboot masking, does not change */
	.taa_ps = 13750,/* tAA from DRAM datasheet (ps)*/
	.trcd_ps = 13750,/* tRCD from DRAM datasheet (ps) */
	.trp_ps = 13750,/* tRP from DRAM datasheet (ps)*/
	.tras_ps = 32000,/* tRAS from DRAM datasheet (ps) */
	.trc_ps = 457500,/* tRC = tRP+tRCD or from DRAM datasheet (ps)*/
	.twr_ps = 15000,/* tWR,write recovery, from DRAM datasheet (ps) */
	.trfc1_ps = 350000,/* tRFC1 from DRAM datasheet (ps)*/
	.trfc2_ps = 260000,/* tRFC2 from DRAM datasheet (ps)*/
	.trfc4_ps = 160000,/* tRFC4 from DRAM datasheet (ps)*/
	.tfaw_ps = 21000,/* tFAW from DRAM datasheet (ps)*/
	.trrds_ps = 3300,/* tRRD_S from DRAM datasheet (ps)*/
	.trrdl_ps = 4900,/* tRRD_L from DRAM datasheet (ps)*/
	.tccdl_ps = 5000,/* tCCD_L from DRAM datasheet (ps)*/
	.refresh_rate_ps = 3900000,/* tREFI from DRAM datasheet (ps)*/

	.dq_mapping[0] = 0x01,
	.dq_mapping[1] = 0x21,
	.dq_mapping[2] = 0x01,
	.dq_mapping[3] = 0x21,
	.dq_mapping[4] = 0x01,
	.dq_mapping[5] = 0x21,
	.dq_mapping[6] = 0x01,
	.dq_mapping[7] = 0x21,
	.dq_mapping[8] = 0x01,
	.dq_mapping[9] = 0x21,
	.dq_mapping[10] = 0x01,
	.dq_mapping[11] = 0x21,
	.dq_mapping[12] = 0x01,
	.dq_mapping[13] = 0x21,
	.dq_mapping[14] = 0x01,
	.dq_mapping[15] = 0x21,
	.dq_mapping[16] = 0x01,
	.dq_mapping[17] = 0x21,
	.dq_mapping_ors = 1,
};

int ddr_get_ddr_params(struct dimm_params *pdimm,
			    struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;
	memcpy(pdimm, &ddr_raw_timing, sizeof(ddr_raw_timing));
	strlcpy(pdimm->mpart, dimm_model, sizeof(pdimm->mpart));

	/* valid DIMM mask, change accordingly, together with dimm_on_ctlr. */
	return 0x5;
}

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;

	popts->vref_dimm = 0x24;		/* range 1, 83.4% */
	popts->rtt_override = 0;
	popts->rtt_park = 60;
	popts->otf_burst_chop_en = 0;
	popts->burst_length = DDR_BL8;
	popts->trwt_override = 1;
	popts->bstopre = 0;			/* auto precharge */
	popts->addr_hash = 1;

	popts->trwt = 0x3;
	popts->twrt = 0x3;
	popts->trrt = 0x3;
	popts->twwt = 0x3;
	popts->vref_phy = 0x60;	/* 75% */
	popts->odt = 60;
	popts->phy_tx_impedance = 48;
	popts->output_driver_impedance = 1;
	debug("TQMLX2160a board ddr4 settings: RTT_park: %u - ODT: %u - output driver impedance: %d\n",
	      popts->rtt_park, popts->odt, popts->output_driver_impedance);

	return 0;
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
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(info));

	/* Set two DDRC. Unused DDRC will be removed automatically. */
	info.num_ctlrs = 2;
	info.ddr[0] = (void *)NXP_DDR_ADDR;
	info.ddr[1] = (void *)NXP_DDR2_ADDR;
	info.phy[0] = (void *)NXP_DDR_PHY1_ADDR;
	info.phy[1] = (void *)NXP_DDR_PHY2_ADDR;
	info.clk = get_ddr_freq(&sys, 0);
	info.img_loadr = load_img;
	info.phy_gen2_fw_img_buf = PHY_GEN2_FW_IMAGE_BUFFER;
	if (!info.clk)
		info.clk = get_ddr_freq(&sys, 1);
	info.dimm_on_ctlr = 2;

	dram_size = dram_init(&info
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
		    , NXP_CCN_HN_F_0_ADDR
#endif
	);

	if (dram_size < 0)
		ERROR("DDR init failed.\n");

	return dram_size;
}
