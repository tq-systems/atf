/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "rcar_def.h"
#if RCAR_LSI == RCAR_AUTO
#include "G2M/pfc_init_m3.h"
#include "G2N/pfc_init_m3n.h"
#include "G2H/pfc_init_h3_v2.h"
#endif
#if RCAR_LSI == RCAR_M3		/* G2M */
#include "G2M/pfc_init_m3.h"
#endif
#if RCAR_LSI == RCAR_M3N	/* G2N */
#include "G2N/pfc_init_m3n.h"
#endif
#if RCAR_LSI == RCAR_E3		/* G2E */
#include "G2E/pfc_init_e3.h"
#endif
#if RCAR_LSI == RCAR_H3N	/* G2H */
#include "G2H/pfc_init_h3_v2.h"
#endif

#define PRR_PRODUCT_ERR(reg)				\
	do {						\
		ERROR("LSI Product ID(PRR=0x%x) PFC initialize not supported.\n", \
			reg);				\
		panic();				\
	} while (0)

#define PRR_CUT_ERR(reg)				\
	do {						\
		ERROR("LSI Cut ID(PRR=0x%x) PFC initialize not supported.\n", \
			reg);				\
		panic();\
	} while (0)

void rcar_pfc_init(void)
{
	uint32_t reg;

	reg = mmio_read_32(RCAR_PRR);
#if RCAR_LSI == RCAR_AUTO
	switch (reg & PRR_PRODUCT_MASK) {
        case PRR_PRODUCT_H3:
		pfc_init_h3_v2();
		break;
	case PRR_PRODUCT_M3:
		pfc_init_m3();
		break;
	case PRR_PRODUCT_M3N:
		pfc_init_m3n();
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}

#elif RCAR_LSI_CUT_COMPAT
	switch (reg & PRR_PRODUCT_MASK) {
	case PRR_PRODUCT_H3:
#if (RCAR_LSI != RCAR_H3N)
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_h3_v2();
#endif
                break;
	case PRR_PRODUCT_M3:
#if RCAR_LSI != RCAR_M3
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_m3();
#endif
		break;
	case PRR_PRODUCT_M3N:
#if RCAR_LSI != RCAR_M3N
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_m3n();
#endif
		break;
	case PRR_PRODUCT_E3:
#if RCAR_LSI != RCAR_E3
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_e3();
#endif
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}

#else
#if (RCAR_LSI == RCAR_H3N)	/* H3N */
	/* H3 Ver.2.0 or later */
	if (PRR_PRODUCT_H3 != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_h3_v2();
#endif
#if RCAR_LSI == RCAR_M3	/* M3 */
	if ((PRR_PRODUCT_M3) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_m3();
#elif RCAR_LSI == RCAR_M3N	/* M3N */
	if ((PRR_PRODUCT_M3N) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_m3n();
#elif RCAR_LSI == RCAR_E3	/* E3 */
	if ((PRR_PRODUCT_E3) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_e3();
#else
#error "Don't have PFC initialize routine(unknown)."
#endif
#endif
}
