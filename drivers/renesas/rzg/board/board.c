/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/utils_def.h>

#include <iic_dvfs.h>
#include <lib/mmio.h>
#include "rcar_def.h"

#include "board.h"

#ifndef BOARD_DEFAULT
#if (RCAR_LSI == RCAR_E3)
#define BOARD_DEFAULT		(BOARD_EK874 << BOARD_CODE_SHIFT)
#elif (RCAR_LSI == RCAR_M3N)
#if (RZG_TQMARZG2N_B)
#define BOARD_DEFAULT		(BOARD_TQMARZG2N_B << BOARD_CODE_SHIFT)
#else
#define BOARD_DEFAULT		(BOARD_HIHOPE_RZG2N << BOARD_CODE_SHIFT)
#endif
#elif (RCAR_LSI == RCAR_H3N)
#if (RZG_TQMARZG2H_C)
#define BOARD_DEFAULT		(BOARD_TQMARZG2H_C << BOARD_CODE_SHIFT)
#else
#define BOARD_DEFAULT		(BOARD_HIHOPE_RZG2H << BOARD_CODE_SHIFT)
#endif
#else
#if (RZG_TQMARZG2M_E)
#define BOARD_DEFAULT		(BOARD_TQMARZG2M_E << BOARD_CODE_SHIFT)
#elif (RZG_TQMARZG2M_AA)
#define BOARD_DEFAULT		(BOARD_TQMARZG2M_AA << BOARD_CODE_SHIFT)
#else
#define BOARD_DEFAULT		(BOARD_HIHOPE_RZG2M << BOARD_CODE_SHIFT)
#endif
#endif
#endif

#define BOARD_CODE_MASK		(0xF8)
#define BOARD_REV_MASK		(0x07)
#define BOARD_CODE_SHIFT	(0x03)
#define BOARD_ID_UNKNOWN	(0xFF)

#define		GPIO_INDT5	0xE605500C
#define		GP5_19_BIT	(0x01 << 19)
#define		GP5_21_BIT	(0x01 << 21)
#define		GP5_25_BIT	(0x01 << 25)

#define SXS_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define SX_ID	{ 0x10U, 0x11U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define SKP_ID	{ 0x10U, 0x10U, 0x20U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define SK_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define EB4_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define EB_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define DR_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define EA_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define KK_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define HM_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define HN_ID	{ 0x20U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define HH_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define EK_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define NB_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define HC_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define ME_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }
#define MAA_ID	{ 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU }

const char *g_board_tbl[] = {
	[BOARD_STARTER_KIT_PRE] = "Starter Kit Premier",
	[BOARD_STARTER_KIT] = "Starter Kit",
	[BOARD_SALVATOR_XS] = "Salvator-XS",
	[BOARD_SALVATOR_X] = "Salvator-X",
	[BOARD_EBISU_4D] = "Ebisu-4D",
	[BOARD_KRIEK] = "Kriek",
	[BOARD_EBISU] = "Ebisu",
	[BOARD_DRAAK] = "Draak",
	[BOARD_EAGLE] = "Eagle",
	[BOARD_HIHOPE_RZG2M]	= "HiHope RZ/G2M",
	[BOARD_HIHOPE_RZG2N]	= "HiHope RZ/G2N",
	[BOARD_HIHOPE_RZG2H]	= "HiHope RZ/G2H",
	[BOARD_EK874]			= "EK874 RZ/G2E",
	[BOARD_TQMARZG2N_B]		= "TQMaRZG2N (2GB)",
	[BOARD_TQMARZG2M_E]		= "TQMaRZG2M (8GB)",
	[BOARD_TQMARZG2M_AA]		= "TQMaRZG2M (2GB)",
	[BOARD_TQMARZG2H_C]		= "TQMaRZG2H (4GB)",
	[BOARD_UNKNOWN] = "unknown"
};

int32_t rcar_get_board_type(uint32_t *type, uint32_t *rev)
{
	int32_t ret = 0;
	const uint8_t board_tbl[][8] = {
		[BOARD_STARTER_KIT_PRE] = SKP_ID,
		[BOARD_SALVATOR_XS] = SXS_ID,
		[BOARD_STARTER_KIT] = SK_ID,
		[BOARD_SALVATOR_X] = SX_ID,
		[BOARD_EBISU_4D] = EB4_ID,
		[BOARD_EBISU] = EB_ID,
		[BOARD_DRAAK] = DR_ID,
		[BOARD_EAGLE] = EA_ID,
		[BOARD_KRIEK] = KK_ID,
		[BOARD_HIHOPE_RZG2M] = HM_ID,
		[BOARD_HIHOPE_RZG2N] = HN_ID,
		[BOARD_HIHOPE_RZG2H] = HH_ID,
		[BOARD_EK874] = EK_ID,
		[BOARD_TQMARZG2N_B]	= NB_ID,
		[BOARD_TQMARZG2M_E]	= ME_ID,
		[BOARD_TQMARZG2M_AA]	= MAA_ID,
		[BOARD_TQMARZG2H_C]	= HC_ID,
	};
	static uint8_t board_id = BOARD_ID_UNKNOWN;
#if (RZG_HIHOPE_RZG2H)
	uint32_t boardInfo;
#else /* RZG_HIHOPE_RZG2H */
	uint32_t read_rev;
#endif /* RZG_HIHOPE_RZG2H */
#if (RZG_HIHOPE_RZG2N) | (RZG_HIHOPE_RZG2M)
	uint32_t reg, boardInfo;
#endif /* RZG_HIHOPE_RZG2N | RZG_HIHOPE_RZG2M */

	if (board_id != BOARD_ID_UNKNOWN)
		goto get_type;

#if PMIC_ROHM_BD9571
	/* Board ID detection from EEPROM */
	ret = rcar_iic_dvfs_receive(EEPROM, BOARD_ID, &board_id);
	if (ret) {
		board_id = BOARD_ID_UNKNOWN;
		goto get_type;
	}

	if (board_id == BOARD_ID_UNKNOWN)
		board_id = BOARD_DEFAULT;
#else
	board_id = BOARD_DEFAULT;
#endif

get_type:
	*type = ((uint32_t) board_id & BOARD_CODE_MASK) >> BOARD_CODE_SHIFT;

	if (*type >= ARRAY_SIZE(board_tbl)) {
		/* no revision information, set Rev0.0. */
		*rev = 0;
		return ret;
	}

#if (RZG_HIHOPE_RZG2M)
	reg = mmio_read_32(RCAR_PRR);
	if (RCAR_M3_CUT_VER11 == (reg & PRR_CUT_MASK))
	{
		read_rev = (uint8_t)(board_id & BOARD_REV_MASK);
		*rev = board_tbl[*type][read_rev];
	}
	else
	{
		boardInfo = mmio_read_32(GPIO_INDT5) & (GP5_19_BIT |GP5_21_BIT);
		*rev = (((boardInfo & GP5_19_BIT) >> 14) | ((boardInfo & GP5_21_BIT) >> 17)) + 0x30;
	}
#elif (RZG_HIHOPE_RZG2N)
	reg = mmio_read_32(GPIO_INDT5);
	if (reg & GP5_25_BIT)
	{
		read_rev = (uint8_t)(board_id & BOARD_REV_MASK);
		*rev = board_tbl[*type][read_rev];
	}
	else
	{
		boardInfo = reg & (GP5_19_BIT |GP5_21_BIT);
		*rev = (((boardInfo & GP5_19_BIT) >> 14) | ((boardInfo & GP5_21_BIT) >> 17)) + 0x30;
	}
#elif (RZG_HIHOPE_RZG2H)
	boardInfo = mmio_read_32(GPIO_INDT5) & (GP5_19_BIT |GP5_21_BIT);
	*rev = (((boardInfo & GP5_19_BIT) >> 14) | ((boardInfo & GP5_21_BIT) >> 17)) + 0x30;
#else
	if (mmio_read_32(RCAR_PRR) & RCAR_MINOR_MASK)
		*rev = 0x30U;
	else {
		read_rev = (uint8_t)(board_id & BOARD_REV_MASK);
		*rev = board_tbl[*type][read_rev];
	}
#endif
	return ret;
}
