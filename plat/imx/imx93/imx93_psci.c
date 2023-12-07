/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <drivers/arm/gicv3.h>
#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <sema42.h>
#include <trdc.h>
#include <plat_imx8.h>

#define BLK_CTRL_S_BASE                0x444F0000
#define M33_CFG_OFF            	0x60
#define CA55_CPUWAIT           0x118
#define CA55_RVBADDR0_L                0x11c
#define CA55_RVBADDR0_H                0x120
#define HW_LP_HANDHSK		0x110
#define HW_LP_HANDHSK2		0x114
#define HW_S401_RESET_REQ_MASK  0x130
#define M33_CPU_WAIT_MASK      BIT(2)

#define IMX_SRC_BASE		0x44460000
#define IMX_GPC_BASE		0x44470000

/* SRC */
#define MIX_AUTHEN_CTRL 	0x4
#define LPM_SETTING_1		0x14
#define LPM_SETTING_2		0x18
#define A55C0_MEM		0x5c00

#define MEM_CTRL		0x4
#define MEM_LP_EN		BIT(2)
#define MEM_LP_RETENTION	BIT(1)

/* GPC */
#define GPC_CMCx(i)	(IMX_GPC_BASE + 0x800 * (i))
#define CM_IMR0		U(0x100)
#define A55C0_CMC_OFFSET	0x800
#define CM_MISC		0xc
#define IRQ_MUX		BIT(5)
#define SW_WAKEUP	BIT(6)

#define IMR_NUM		U(8)

#define CM_MODE_CTRL		0x10
#define CM_IRQ_WAKEUP_MASK0	0x100
#define CM_SYS_SLEEP_CTRL	0x380

#define SS_WAIT		BIT(0)
#define SS_STOP		BIT(1)
#define SS_SUSPEND	BIT(2)

#define IMX_SRC_A55C0_OFFSET   0x2c00
#define SLICE_SW_CTRL          0x20
#define SLICE_SW_CTRL_PDN_SOFT BIT(31)

#define CM_MODE_RUN	0x0
#define CM_MODE_WAIT	0x1
#define CM_MODE_STOP	0x2
#define CM_MODE_SUSPEND	0x3

#define GPC_DOMAIN	0x10
#define GPC_DOMAIN_SEL	0x18
#define GPC_MASTER	0x1c
#define GPC_SYS_SLEEP	0x40
#define PMIC_CTRL	0x100
#define GPC_RCOSC_CTRL	0x200

#define GPC_GLOBAL_OFFSET	0x4000

#define BBNSM_BASE	U(0x44440000)
#define BBNSM_CTRL	U(0x8)
#define BBNSM_DP_EN	BIT(24)
#define BBNSM_TOSP	BIT(25)

#define LPM_DOMAINx(n)	(1 << ((n) * 4))

#define ARM_PLL		U(0x44481000)
#define SYS_PLL		U(0x44481100)
#define SYS_PLL_DFS_0	U(SYS_PLL + 0x70)
#define SYS_PLL_DFS_1	U(SYS_PLL + 0x90)
#define SYS_PLL_DFS_2	U(SYS_PLL + 0xb0)
#define OSCPLL_CHAN(x)	(0x44455000 + (x) * 0x40)
#define OSCPLL_NUM	U(12)
#define OSCPLL_LPM0	U(0x10)
#define OSCPLL_LPM_DOMAIN_MODE(x, d) ((x) << (d * 4))
#define OSCPLL_LPM_AUTH	U(0x30)
#define PLL_HW_CTRL_EN	BIT(16)
#define LPCG(x) 	(0x44458000 + (x) * 0x40)
#define LPCG_AUTH	U(0x30)
#define LPCG_CUR	U(0x1c)
#define CPU_LPM_EN	BIT(2)
#define CCM_ROOT_SLICE(x)	(0x44450000 + (x) * 0x80)
#define ROOT_MUX_MASK	GENMASK_32(9, 8)
#define ROOT_CLK_OFF	BIT(24)

#define S400_MU_RSR	(S400_MU_BASE + 0x12c)
#define S400_MU_TRx(i)	(S400_MU_BASE + 0x200 + (i) * 4)
#define S400_MU_RRx(i)	(S400_MU_BASE + 0x280 + (i) * 4)
#define ELE_POWER_DOWN_REQ	U(0x17d10306)

#define MU1B_BASE	(0x44230000)
#define MU1B_GIER	(MU1B_BASE + 0x110)
#define MU1B_GSR	(MU1B_BASE + 0x118)
#define MU_GPI1		BIT(1)

#define M33_ACTIVE_FLAG		(IMX_SRC_BASE + 0x54)
#define M33_ACTIVE		U(0x5555)
#define DDR_RETENTION		(IMX_SRC_BASE + 0x58)
#define DDR_RETENTION_A55_FLAG	BIT(0)
#define DDR_RETENTION_M33_FLAG	BIT(1)
#define DDR_RETENTION_PLL_LPM	BIT(2) /* PLL can use LPM mode when DDR is in retention, CM33 has configured its PLLs PLM setting */


#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define GPIO_CTRL_REG_NUM		U(8)
#define GPIO_PIN_MAX_NUM		U(32)
#define GPIO_CTX(addr, num)	\
	{.base = (addr), .pin_num = (num), }

enum ccm_clock_root {
	M33_ROOT = 3,
	BUS_WAKUP_ROOT = 5,
	BUS_AON_ROOT = 6,
	WAKEUP_AXI_ROOT = 7,
	CAN1_ROOT = 23,
	CAN2_ROOT = 24,
	UART1_ROOT = 25,
	UART2_ROOT = 26,
	UART3_ROOT = 27,
	UART4_ROOT = 28,
	UART5_ROOT = 29,
	UART6_ROOT = 30,
	UART7_ROOT = 31,
	UART8_ROOT = 32,
	HSIO_CLK_ROOT = 61,
	NIC_CLK_ROOT = 65,
};

enum ccm_lpcg {
	MUB_LPCG = 20,
	GPIO1_LPCG = 34,
	GPIO2_LPCG = 35,
	GPIO3_LPCG = 36,
	GPIO4_LPCG = 37,
	CAN1_LPCG = 50,
	CAN2_LPCG = 51,
	UART1_LPCG = 52,
	UART2_LPCG = 53,
	UART3_LPCG = 54,
	UART4_LPCG = 55,
	UART5_LPCG = 56,
	UART6_LPCG = 57,
	UART7_LPCG = 58,
	UART8_LPCG = 59,
};

extern void dram_enter_retention(void);
extern void dram_exit_retention(void);
extern void s401_request_pwrdown(void);
extern void trdc_n_reinit(void);
extern void trdc_w_reinit(void);

struct gpio_ctx {
	/* gpio base */
	uintptr_t base;
	/* port control */
	uint32_t port_ctrl[GPIO_CTRL_REG_NUM];
	/* GPIO ICR, Max 32 */
	uint32_t pin_num;
	uint32_t gpio_icr[GPIO_PIN_MAX_NUM];
};

/* for GIC context save/restore if NIC lost power */
struct plat_gic_ctx imx_gicv3_ctx;
/* platfrom secure warm boot entry */
static uintptr_t secure_entrypoint;

static bool boot_stage = true;
static bool no_wakeup_enabled = true;

static uint32_t gpio_ctrl_offset[GPIO_CTRL_REG_NUM] = { 0xc, 0x10, 0x14, 0x18, 0x1c, 0x40, 0x54, 0x58 };
static struct gpio_ctx wakeupmix_gpio_ctx[3] = {
	GPIO_CTX(GPIO2_BASE | BIT(28), 30),
	GPIO_CTX(GPIO3_BASE | BIT(28), 32),
	GPIO_CTX(GPIO4_BASE | BIT(28), 28),
};

static uint32_t clock_root[6];

/*
 * Empty implementation of these hooks avoid setting the GICR_WAKER.Sleep bit
 * on ARM GICv3 implementations without LPI support.
 */
void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num)
{}

void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num)
{}

void gpc_src_init(void)
{
	unsigned int i;

	/* Use GPC assigned domain control */
	mmio_write_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_DOMAIN_SEL, 0x0);

	/*
	 * Assigned A55 cluster to domain3, m33 to domain2, CORE0 & CORE1 to domain0/1.
	 * domain0/1 only used for trigger LPM of themselves.
	 */
	mmio_write_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_DOMAIN, 0x3102);

	/* A55 CORE0/1 GPC/SRC init */
	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		/*
		 * Disable the system sleep control trigger for A55 core0/core1 & cluster by default.
		 */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * i + CM_SYS_SLEEP_CTRL, 0x0);
		/* Config A55 core & cluster's wakeup source to GIC by default */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * i + CM_MISC, IRQ_MUX);
		/* Clear the cpu sleep hold */
		mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * i + CM_MISC, BIT(1));

		/*
		 * Ignore core's LPM trigger for system sleep.
		 * For core0/1, GPC_SYS_SLEEP FORCE_COREx_DISABLE set to 1'b1.
		 */
		mmio_setbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_SYS_SLEEP, 1 << (17 + i));
	}

	/* SRC MIX & MEM slice config for cores */
	/* MEM LPM */
	mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 0 + 0x4, MEM_LP_EN);
	/* LPM config to only ON in run mode, LPM control only by core itself; domain0/1 */
	mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 0 + 0x14, 0x1 << (0 * 4));
	/* Set CNT_MODE =0 to reduce unnecessary latency */
	mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 0 + 0x80, 0x00a000a0);
	/* config SRC to enable LPM control(HW flow) */
	mmio_clrsetbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 0 + 0x4, 0xffff0000, (1 << (0 + 16)) | BIT(2));

	/* enable the HW LP handshake between S401 & A55 cluster */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(5));

	/* A55 cluster GPC and SRC slice init */
	/* disable A55 cluster's system sleep trigger by default */
	mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_SYS_SLEEP_CTRL, 0x0);
	/* config A55 cluster's wakeup source to GIC by default */
	mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MISC, IRQ_MUX);
	/* Clear the A55 cluster's sleep hold */
	mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MISC, BIT(1));

	/* SCU Snoop MEM must be power down, can NOT be put into retention */
	mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 2 + 0x4, MEM_LP_EN);
	/* L3 MEM */
	mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 3 + 0x4, MEM_LP_EN);
	/*
	 * cluster power domain can only be controlled by cluster itself, set LPM before whitelist config
	 * to make sure the setting can be write successfully
	 */
	mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 2 + 0x14, BIT(12));
	/* Set CNT_MODE =0 to reduce unnecessary latency */
	mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 2 + 0x80, 0x00a000a0);
	/* config the SRC for cluster slice LPM control */
	mmio_clrsetbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 2 + 0x4, 0xffff0000, BIT(19) | BIT(2));

	/* enable S401 clock gating LP handshake */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(24) | BIT(23));
	mmio_setbits_32(LPCG(3) + 0x10, BIT(13) | BIT (12));
	mmio_setbits_32(LPCG(3) + 0x30, BIT(2));
}

static struct qchannel_hsk_config {
	const uint32_t lpcg_idx;
	const uint32_t root_idx;
	const unsigned int wakeup_irq;
	uint32_t root_ctrl;
	uint32_t lpcg_cur;
	uint32_t lpcg_auth;
	bool active_wakeup;
} hsk_config[] = {
	{ CAN1_LPCG, CAN1_ROOT, 8 },
	{ CAN2_LPCG, CAN2_ROOT, 51 },

	{ UART1_LPCG, UART1_ROOT, 19 },
	{ UART2_LPCG, UART2_ROOT, 20 },
	{ UART3_LPCG, UART3_ROOT, 68 },
	{ UART4_LPCG, UART4_ROOT, 69 },
	{ UART5_LPCG, UART5_ROOT, 70 },
	{ UART6_LPCG, UART6_ROOT, 71 },
	{ UART7_LPCG, UART7_ROOT, 210 },
	{ UART8_LPCG, UART8_ROOT, 211 },

	{ GPIO1_LPCG, },
	{ GPIO2_LPCG, },
	{ GPIO3_LPCG, },
	{ GPIO4_LPCG, },
};

/*
 * M33 side need to raise this flag it DDR is used when
 * A55 side enter low power mode.
 */
static inline bool is_m33_active(void)
{
	return mmio_read_32(M33_ACTIVE_FLAG) == M33_ACTIVE;
}

static inline bool is_wakeup_source(unsigned int irq)
{
	uint32_t val;

	val = mmio_read_32(GPC_CMCx(3) + CM_IMR0 + 0x4 * (irq / 32));
	return val & (1 << (irq % 32)) ? false : true;
}
/*
 * For peripherals like CANs, GPIOs & UARTs that need to support async wakeup
 * when clock is gated, LPCGs of these IPs need to be changed to CPU LPM
 * controlled, and for CANs &UARTs, we also need to make sure its ROOT clock
 * slice is enabled.
 */
void peripheral_qchannel_hsk(bool en)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(hsk_config); i++) {
		if (en) {
			/* Only enable the qchannel handshake for active wakeup used by A55 */
			if (!hsk_config[i].wakeup_irq || is_wakeup_source(hsk_config[i].wakeup_irq)) {
				hsk_config[i].active_wakeup = true;
				if (hsk_config[i].root_idx) {
					hsk_config[i].root_ctrl = mmio_read_32(CCM_ROOT_SLICE(hsk_config[i].root_idx));
					mmio_clrbits_32(CCM_ROOT_SLICE(hsk_config[i].root_idx), ROOT_CLK_OFF);
				}

				hsk_config[i].lpcg_auth = mmio_read_32(LPCG(hsk_config[i].lpcg_idx) + LPCG_AUTH);
				hsk_config[i].lpcg_cur = mmio_read_32(LPCG(hsk_config[i].lpcg_idx) + LPCG_CUR);
				mmio_setbits_32(LPCG(hsk_config[i].lpcg_idx) + LPCG_AUTH, CPU_LPM_EN);
				mmio_write_32(LPCG(hsk_config[i].lpcg_idx) + LPCG_CUR, 0x2);
			} else {
				hsk_config[i].active_wakeup = false;
			}
		} else if (hsk_config[i].active_wakeup) {
			/* restore the initial config */
			mmio_write_32(LPCG(hsk_config[i].lpcg_idx) + LPCG_CUR, hsk_config[i].lpcg_cur);
			mmio_write_32(LPCG(hsk_config[i].lpcg_idx) + LPCG_AUTH, hsk_config[i].lpcg_auth);
			if (hsk_config[i].root_idx) {
				mmio_write_32(CCM_ROOT_SLICE(hsk_config[i].root_idx), hsk_config[i].root_ctrl);
			}
		}
	}
}

void pll_pwr_down(bool enter)
{
	if(enter) {
		/* Switch the ARM/SYS PLLs to hw_ctrl(bit 16) in PLL CTRL reg */
		mmio_setbits_32(ARM_PLL, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_0, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_1, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_2, PLL_HW_CTRL_EN);

		/* LPM setting for PLL */
		for (unsigned int i = 1; i <= OSCPLL_NUM; i++) {
			mmio_setbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM0, OSCPLL_LPM_DOMAIN_MODE(0x1, 0x3));
			mmio_setbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM_AUTH, BIT(2));
		}
	} else {
		mmio_clrbits_32(ARM_PLL, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_0, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_1, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_2, PLL_HW_CTRL_EN);

		for (unsigned int i = 1; i <= OSCPLL_NUM; i++) {
			mmio_clrbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM_AUTH, BIT(2));
		}
	}
}

void s401_request_pwrdown(void)
{
	uint32_t msg, resp;

	mmio_write_32(S400_MU_TRx(0), ELE_POWER_DOWN_REQ);
	mmio_write_32(S400_MU_TRx(1), 0x4000);
	mmio_write_32(S400_MU_TRx(2), BL31_BASE);

	do {
		resp = mmio_read_32(S400_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(S400_MU_RRx(0));
	resp = mmio_read_32(S400_MU_RRx(1));

	VERBOSE("resp %x; %x", msg, resp);
}

void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	unsigned int i;
	uint32_t irq_mask;
	uintptr_t gicd_base = PLAT_GICD_BASE;

	if (pdn) {
		/*
		 * If NICMIX power down, need to switch the primary core & cluster wakeup
		 * source to GPC as GIC will be power down.
		 */
		mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MISC, IRQ_MUX);
		mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * last_core + CM_MISC, IRQ_MUX);

		/* make sure MUB side clock is enabled */
		mmio_write_32(LPCG(MUB_LPCG), 0x1);
		/* enable the MU1B general interrupt 1 for M33 SW to wakeup A55 by assert an interrupt */
		mmio_setbits_32(MU1B_GIER, MU_GPI1);
	} else {
		/* switch to GIC wakeup source for last_core and cluster */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MISC, IRQ_MUX);
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * last_core + CM_MISC, IRQ_MUX);

		/* make sure MUB side clock is enabled */
		mmio_write_32(LPCG(MUB_LPCG), 0x1);
		/* clear pending General interrupt 1 and disable the it */
		mmio_clrbits_32(MU1B_GIER, MU_GPI1);
		mmio_setbits_32(MU1B_GSR, MU_GPI1);
	}

	/* Set the GPC IMRs based on GIC IRQ mask setting */
	for (i = 0; i < IMR_NUM; i++) {
		if (pdn) {
			/* set the wakeup irq base GIC */
			irq_mask = ~gicd_read_isenabler(gicd_base, 32 * (i + 1));
		} else {
			irq_mask = 0xFFFFFFFF;
		}

		/* set the mask into core & cluster GPC IMR */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + 0x100 + 0x4 * i, irq_mask);
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * last_core + 0x100 + 0x4 * i, irq_mask);
	}
}

void nicmix_qos_init(void)
{
	mmio_write_32(0x49000010, 0x44);
	mmio_write_32(0x49000014, 0x330033);
	mmio_write_32(0x49000018, 0x330033);
	mmio_write_32(0x4900001c, 0x330033);
	mmio_write_32(0x49000020, 0x440044);
}

void wakeupmix_qos_init(void)
{
	mmio_write_32(0x42846100, 0x4);
	mmio_write_32(0x42846104, 0x4);

	mmio_write_32(0x42847100, 0x4);
	mmio_write_32(0x42847104, 0x4);

	mmio_write_32(0x42842100, 0x4);
	mmio_write_32(0x42842104, 0x4);

	mmio_write_32(0x42843100, 0x4);
	mmio_write_32(0x42843104, 0x4);

	mmio_write_32(0x42845100, 0x4);
	mmio_write_32(0x42845104, 0x4);

	mmio_write_32(0x43947100, 0x4);
	mmio_write_32(0x43947104, 0x4);

	mmio_write_32(0x43945100, 0x4);
	mmio_write_32(0x43945104, 0x4);
}

void nicmix_pwr_down(unsigned int core_id)
{
	/* enable the handshake between sentinel & NICMIX */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(11));

	/* swith wakeup axi, hsio & nic to 24M when NICMIX power down */
	clock_root[1] = mmio_read_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT));
	clock_root[2] = mmio_read_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT));
	clock_root[3] = mmio_read_32(CCM_ROOT_SLICE(NIC_CLK_ROOT));
	mmio_clrbits_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), ROOT_MUX_MASK);
	mmio_clrbits_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), ROOT_MUX_MASK);
	mmio_clrbits_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), ROOT_MUX_MASK);

	/* NICMIX */
	mmio_write_32(IMX_SRC_BASE + 0x1c00 + 0x14, BIT(12));
	mmio_clrsetbits_32(IMX_SRC_BASE + 0x1c00 + 0x4, 0xffff0000, BIT(19) | BIT(2));
	/* NICMIX OCRAM memory retention */
	mmio_setbits_32(IMX_SRC_BASE + 0x4c00 + MEM_CTRL, MEM_LP_EN);
	/* OCRAM MEM */
	mmio_setbits_32(IMX_SRC_BASE + 0x5000 + MEM_CTRL, MEM_LP_EN | MEM_LP_RETENTION);

	/* Save the gic context */
	plat_gic_save(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, true);
}

void nicmix_pwr_up(unsigned int core_id)
{
	mmio_setbits_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), clock_root[1] & ROOT_MUX_MASK);
	mmio_setbits_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), clock_root[2] & ROOT_MUX_MASK);
	mmio_setbits_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), clock_root[3] & ROOT_MUX_MASK);

	/* keep nicmix on when exit from system suspend */
	mmio_write_32(IMX_SRC_BASE + 0x1c00 + 0x14, 0x33333333);
	mmio_clrbits_32(IMX_SRC_BASE + 0x1c00 + 0x4, BIT(2));

	trdc_n_reinit();
	nicmix_qos_init();
	plat_gic_restore(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, false);
}

extern int trdc_mbc_blk_config(unsigned long trdc_reg, uint32_t mbc_x,
	 uint32_t dom_x, uint32_t mem_x, uint32_t blk_x,
	 bool sec_access, uint32_t glbac_id);

void set_gpio_secure(bool ns)
{
	/* GPIO2-4 */
	/* GPIO2 TRDC_W MBC0 MEM 1 */
	trdc_mbc_blk_config(0x42460000, 0, 3, 1, 0x0, ns, 0);
	/* GPIO3 TRDC_W MBC0 MEM 2 */
	trdc_mbc_blk_config(0x42460000, 0, 3, 2, 0x0, ns, 0);
	/* GPIO4 TRDC_W MBC1 MEM 3 */
	trdc_mbc_blk_config(0x42460000, 1, 3, 3, 0x0, ns, 0);
}

void gpio_save(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	/* Enable GPIO secure access */
	set_gpio_secure(true);

	for (i = 0; i < port_num; i++) {
		/* save the port control setting */
		for (j = 0; j < GPIO_CTRL_REG_NUM; j++) {
			if (j < 4) {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
				/*
				 * clear the permission setting to read the GPIO non-secure world setting.
				*/
				mmio_write_32(ctx->base + gpio_ctrl_offset[j], 0x0);
			} else {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
			}
		}
		/* save the gpio icr setting */
		for (j = 0; j < ctx->pin_num; j++) {
			ctx->gpio_icr[j] = mmio_read_32(ctx->base + 0x80 + j * 4);

			/* check if any gpio irq is enabled as wakeup source */
			if (ctx->gpio_icr[j]) {
				no_wakeup_enabled = false;
			}
		}

		/* permission config retore back */
		for (j = 0; j < 4; j++) {
			mmio_write_32( ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		ctx++;
	}
	/* Disable GPIO secure access */
	set_gpio_secure(false);
}

void gpio_restore(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	set_gpio_secure(true);

	for (i = 0; i < port_num; i++) {
		for (j = 0; j < ctx->pin_num; j++)
			mmio_write_32(ctx->base + 0x80 + j * 4, ctx->gpio_icr[j]);

		for (j = 4; j < GPIO_CTRL_REG_NUM; j++)
			mmio_write_32( ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);

		/* permission config retore last */
		for (j = 0; j < 4; j++) {
			mmio_write_32( ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		ctx++;
	}

	set_gpio_secure(false);
}

void wakeupmix_pwr_down(void)
{
	gpio_save(wakeupmix_gpio_ctx, 3);
	if (no_wakeup_enabled) {
		/* m33 root need to switch to 24M OSC when wakeupmix power down */
		clock_root[0] = mmio_read_32(CCM_ROOT_SLICE(M33_ROOT));
		mmio_clrbits_32(CCM_ROOT_SLICE(M33_ROOT), ROOT_MUX_MASK);

		/* wakeup mix controlled by A55 cluster power down: domain3 only */
		mmio_write_32(IMX_SRC_BASE + 0xc00 + 0x14, BIT(12));
		mmio_clrsetbits_32(IMX_SRC_BASE + 0xc00 + 0x4, 0xffff0000, BIT(19) | BIT(2));
		/* wakeupmix mem off */
		mmio_setbits_32(IMX_SRC_BASE + 0x3c00 + MEM_CTRL, MEM_LP_EN);
		/* enable the handshake between sentinel & wakeupmix */
		mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(9));
	}
}

void wakeupmix_pwr_up(void)
{
	if (no_wakeup_enabled) {
		mmio_setbits_32(CCM_ROOT_SLICE(M33_ROOT), clock_root[0] & ROOT_MUX_MASK);
		/* keep wakeupmix on when exit from system suspend */
		mmio_write_32(IMX_SRC_BASE + 0xc00 + 0x14, BIT(12) | BIT(13));
		mmio_clrbits_32(IMX_SRC_BASE + 0xc00 + 0x4, BIT(2));
		trdc_w_reinit();
		wakeupmix_qos_init();
		gpio_restore(wakeupmix_gpio_ctx, 3);
	}

	/*
	 * after wakeup, revert back to ‘true‘, so next time
	 * evaluation for wakeupmix on/off can work well.
	 */
	no_wakeup_enabled = true;
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	if (pwr_type == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	if (pwr_type == PSTATE_TYPE_POWERDOWN && state_id == 0x33) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	return PSCI_E_SUCCESS;
}

void imx_set_cpu_boot_entry(unsigned int core_id, uint64_t boot_entry)
{
	/* set the cpu core reset entry: BLK_CTRL_S */
	mmio_write_32(BLK_CTRL_S_BASE + CA55_RVBADDR0_L + core_id * 8, boot_entry >> 2);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id;
	core_id = MPIDR_AFFLVL1_VAL(mpidr);

	imx_set_cpu_boot_entry(core_id, secure_entrypoint);

	if (boot_stage) {
		/* assert CPU core SW reset */
		mmio_clrbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + core_id * 0x400 + 0x24, BIT(2) | BIT(0));

		/* deassert CPU core SW reset */
		mmio_setbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + core_id * 0x400 + 0x24, BIT(2) | BIT(0));

		/* release the cpuwait to kick the cpu */
		mmio_clrbits_32(BLK_CTRL_S_BASE + CA55_CPUWAIT, BIT(core_id));
	} else {
		/* config the CMC MISC SW WAKEUP BIT to kick the cpu core */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + core_id * 0x800 + CM_MISC, SW_WAKEUP);
	}

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();

	/* below config is harmless either first time boot or hotplug */
	/* clear the CPU power mode */
	mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_RUN);
	/* clear the SW wakeup */
	mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + core_id * 0x800 + CM_MISC, SW_WAKEUP);
	/* switch to GIC wakeup source */
	mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MISC, IRQ_MUX);

	if (boot_stage) { 
		/* SRC MIX & MEM slice config for cores */
		/* MEM LPM */
		mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * core_id + 0x4, MEM_LP_EN);
		/* LPM config to only ON in run mode, LPM control only by core itself; domain0/1 */
		mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * core_id + 0x14, 0x1 << (core_id * 4));
		/* Set CNT_MODE =0 to reduce unnecessary latency */
		mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * core_id + 0x80, 0x00a000a0);
		/* config SRC to enable LPM control(HW flow) */
		mmio_clrsetbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * core_id + 0x4, 0xffff0000, (1 << (core_id + 16)) | BIT(2));

		boot_stage = false;
	}

}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	int i;

	plat_gic_cpuif_disable();

	write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);

	/* switch to GPC wakeup source */
	mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MISC, IRQ_MUX);

	/*
	 * mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core as
	 * we need to use SW_WAKEUP for hotplug purpose
	 */
	for (i = 0; i < IMR_NUM; i++)
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800  * core_id + CM_IRQ_WAKEUP_MASK0 + i * 4, 0xffffffff);

	/* config the target mode to suspend */
	mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_SUSPEND);

	/* as this cpu core is hotpluged, so config its GPC_SYS_SLEEP FORCE_COREx_DISABLE  to 1'b1 */
	mmio_setbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_SYS_SLEEP, 1 << (17 + core_id));
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		imx_set_cpu_boot_entry(core_id, secure_entrypoint);
		/* config the target mode to WAIT */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_WAIT);
	}

	/* do cluster level config */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* config the A55 cluster target mode to WAIT */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MODE_CTRL, CM_MODE_WAIT);

		/* enable L3 retention */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 3 + 0x4, MEM_LP_RETENTION);
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF | BIT(1));
		} else {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);
		}
	}

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		/*
		 * if M33 is active to use DRAM and the bus fabric, need to do
		 * special handling to reduce the system power.
		 */
		if (is_m33_active()) {
			dcsw_op_all(DCCISW);

			/* make sure sema42_1 clock enabled */
			mmio_write_32(LPCG(17), 0x1);
			/* set LPM CUR domaon control to ON in case of CPU_LPM control */
			mmio_write_32(LPCG(17) + LPCG_CUR, 0x3);

			sema42_lock(0);
			/* put ddr into retention if not used by m33 */
			if (mmio_read_32(DDR_RETENTION) & DDR_RETENTION_M33_FLAG) {
				dram_enter_retention();
			}
			/* DDR is not used by A55 now */
			mmio_setbits_32(DDR_RETENTION, DDR_RETENTION_A55_FLAG);

			/* swith wakeup axi, hsio & nic to 24M when NICMIX power down */
			clock_root[1] = mmio_read_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT));
			clock_root[2] = mmio_read_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT));
			clock_root[3] = mmio_read_32(CCM_ROOT_SLICE(NIC_CLK_ROOT));
			clock_root[4] = mmio_read_32(CCM_ROOT_SLICE(BUS_WAKUP_ROOT));
			clock_root[5] = mmio_read_32(CCM_ROOT_SLICE(BUS_AON_ROOT));

			/* slow down WAKEUP AXI to rate / DIV5 */
			mmio_clrsetbits_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), 0xff, 0x4);
			/* slow down NIC CLK to rate / DIV8 */
			mmio_clrsetbits_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), 0xff, 0x7);
			mmio_clrbits_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), ROOT_MUX_MASK);

			sema42_unlock(0);

			if (mmio_read_32(DDR_RETENTION) & DDR_RETENTION_PLL_LPM) {
				pll_pwr_down(true);
			}

		} else {
			/*
			 * for the A55 cluster, the cache disable/flushing is controlled by HW,
			 * so flush cache explictly before put DDR into retention to make sure
			 * no cache maintenance to DDR memory happens afte DDR retention.
			 */
			dcsw_op_all(DCCISW);
			dram_enter_retention();

			/*
			 * if NICMIX or WAKEUPMIX power down, the TRDC_N/W config will lost,
			 * so need to request Sentinel to set the correct permission at the early
			 * begining.
			 */
			nicmix_pwr_down(core_id);
			s401_request_pwrdown();
			wakeupmix_pwr_down();

			/* power down PLL */
			pll_pwr_down(true);
		}

		peripheral_qchannel_hsk(true);
		/* config the A55 cluster target mode to SUSPEND */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MODE_CTRL, CM_MODE_SUSPEND);

		/* Enable system suspend when A55 cluster is in SUSPEND MODE */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_SYS_SLEEP_CTRL, SS_SUSPEND);

		/* force M33 into system sleep if m33 is not enabled. */
		if (mmio_read_32(BLK_CTRL_S_BASE + M33_CFG_OFF) & M33_CPU_WAIT_MASK)
			mmio_setbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_SYS_SLEEP, BIT(16));
		/* put OSC into power down */
		mmio_setbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_RCOSC_CTRL, BIT(0));
		/* put PMIC into standby mode */
		mmio_setbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + PMIC_CTRL, BIT(0));
	}
}

void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* system level */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		/* Disable system suspend when A55 cluster is in SUSPEND MODE */
		mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_SYS_SLEEP_CTRL, SS_SUSPEND);
		if (mmio_read_32(BLK_CTRL_S_BASE + M33_CFG_OFF) & M33_CPU_WAIT_MASK)
			mmio_clrbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_SYS_SLEEP, BIT(16));
		/* Disable PMIC standby */
		mmio_clrbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + PMIC_CTRL, BIT(0));
		/* Disable OSC power down */
		mmio_clrbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_RCOSC_CTRL, BIT(0));
		peripheral_qchannel_hsk(false);
		if (is_m33_active()) {
			pll_pwr_down(false);

			sema42_lock(0);

			mmio_write_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), clock_root[1]);
			mmio_write_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), clock_root[2]);
			mmio_write_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), clock_root[3]);
			mmio_write_32(CCM_ROOT_SLICE(BUS_WAKUP_ROOT), clock_root[4]);
			mmio_write_32(CCM_ROOT_SLICE(BUS_AON_ROOT), clock_root[5]);

			if (mmio_read_32(DDR_RETENTION) & DDR_RETENTION_M33_FLAG) {
				dram_exit_retention();
			}
			/* DDR need to keep active as A55 will use it */
			mmio_clrbits_32(DDR_RETENTION, DDR_RETENTION_A55_FLAG);

			sema42_unlock(0);
		} else {
			/* power down PLL */
			pll_pwr_down(false);
			nicmix_pwr_up(core_id);
			wakeupmix_pwr_up();
			dram_exit_retention();
		}
	}

	/* cluster level */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* set the cluster's target mode to RUN */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MODE_CTRL, CM_MODE_RUN);
		/* clear L3 retention */
		mmio_clrbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 3 + 0x4, MEM_LP_RETENTION);
	}
	/* do core level */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* set A55 CORE's power mode to RUN */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_RUN);
		plat_gic_cpuif_enable();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;

	SYSTEM_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
}

void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1)
		wfi();
}

#define IMX_WDOG3_BASE	U(0x42490000)

void __dead2 imx_system_reset(void)
{
	mmio_write_32(IMX_WDOG3_BASE + 0x4, 0xd928c520);
	while ((mmio_read_32(IMX_WDOG3_BASE) & 0x800) == 0)
		;
	mmio_write_32(IMX_WDOG3_BASE + 0x8, 0x10);
	mmio_write_32(IMX_WDOG3_BASE, 0x21e3);

	while (true)
		;
}

void __dead2 imx_system_off(void)
{
	mmio_setbits_32(BBNSM_BASE + BBNSM_CTRL, BBNSM_DP_EN | BBNSM_TOSP);

	while (1)
		;
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_suspend = imx_pwr_domain_suspend,
	.pwr_domain_suspend_finish = imx_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.pwr_domain_pwr_down_wfi = imx_pwr_domain_pwr_down_wfi,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	/* sec_entrypoint is used for warm reset */
	secure_entrypoint = sec_entrypoint;
	imx_set_cpu_boot_entry(0, sec_entrypoint);

	gpc_src_init();
	nicmix_qos_init();
	wakeupmix_qos_init();

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
