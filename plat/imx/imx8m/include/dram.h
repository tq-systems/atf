/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRAM_H
#define DRAM_H

#include <lib/utils_def.h>
#include <arch_helpers.h>
#include <assert.h>

#include <ddrc.h>
#include <platform_def.h>

#define DDRC_LPDDR4		BIT(5)
#define DDRC_DDR4		BIT(4)
#define DDRC_DDR3L		BIT(0)
#define DDR_TYPE_MASK		U(0x3f)
#define ACTIVE_RANK_MASK	U(0x3)

/* reg & config param */
struct dram_cfg_param {
	unsigned int reg;
	unsigned int val;
};

struct dram_timing_info {
	/* umctl2 config */
	struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
	/* ddrphy config */
	struct dram_cfg_param *ddrphy_cfg;
	unsigned int ddrphy_cfg_num;
	/* ddr fsp train info */
	struct dram_fsp_msg *fsp_msg;
	unsigned int fsp_msg_num;
	/* ddr phy trained CSR */
	struct dram_cfg_param *ddrphy_trained_csr;
	unsigned int ddrphy_trained_csr_num;
	/* ddr phy PIE */
	struct dram_cfg_param *ddrphy_pie;
	unsigned int ddrphy_pie_num;
	/* initialized fsp table */
	unsigned int fsp_table[4];
};

struct dram_info {
	int dram_type;
	unsigned int num_rank;
	int current_fsp;
	int boot_fsp;
	struct dram_timing_info *timing_info;
};

extern struct dram_info dram_info;

void dram_info_init(unsigned long dram_timing_base);
void dram_umctl2_init(struct dram_timing_info *timing);
void dram_phy_init(struct dram_timing_info *timing);

/* dram retention */
void dram_enter_retention(void);
void dram_exit_retention(void);

#endif /* DRAM_H */