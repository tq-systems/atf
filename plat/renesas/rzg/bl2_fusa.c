/*
 * bl2_ecc_setup.c -
 *
 * Copyright (C) 2018 Renesas Electronics Corporation.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#define FUSA_DRAM_CLEAR		1

#define	DFUSAAREACR		0xE6785000	/* DRAM FuSa Area Conf */
#define	DECCAREACR		0xE6785040	/* DRAM ECC Area Conf */
#define	NUM_DAREA		16
#define	FUSACR			0xE6784020	/* FuSa Ctrl */

#define	ADSPLCR0		0xE6784008	/* Address Split Control 0 */
#define	ADSPLCR1		0xE678400C	/* Address Split Control 1 */
#define	ADSPLCR2		0xE6784010	/* Address Split Control 2 */
#define	ADSPLCR3		0xE6784014	/* Address Split Control 3 */

/* As the saddr, specify high-memory address (> 4 GB) */
#define	FUSAAREACR(en, size, saddr)	\
	(((uint32_t)en << 31) | ((uint32_t)size << 24) | (uint32_t)(((uintptr_t)saddr) >> 12))
#define	ECCAREACR(ecc, saddr) \
	(((uint32_t)ecc << 31) | (uint32_t)(((uintptr_t)saddr) >> 12))

#define	EFUSASEL(x)	((uint32_t)x & 0xff) << 24	/*Setting for Extra Split mode*/
#define	DFUSASEL(x)	((uint32_t)x & 0xff) << 16	/*Setting for DRAM*/
#define	SFUSASEL(x)	((uint32_t)x & 0x3f)		/*Setting for SRAM*/

#define	SPLITSEL(x)	((uint32_t)x & 0xff) << 16	/*Setting for Split mode*/
#define	AREA(x)		((uint32_t)x & 0x1f) << 8	/*address bit devides DBSC into 8 areas*/
#define	SWP(x)		((uint32_t)x & 0x1f)		/*address bit to interleave DBSCs in split mode*/
#define	ADRMODE(x)	((uint32_t)x & 0x1) << 31	/*Select address mapping mode*/

#ifndef ARRAY_SIZE
#define	ARRAY_SIZE(a)		(sizeof(a) / sizeof(a[0]))
#endif

#define	EK874_FUSAAREA			0x408000000
#define	EK874_FUSAAREA_TOTAL		((128+256+256+256+64)*1024*1024)
#define	EK874_ECCAREA			0x444000000
#define	EK874_ECCAREA_TOTAL		((128+256+256+256+64)*1024*1024)
#define	HHOPE_G2N_FUSAAREA		0x408000000
#define	HHOPE_G2N_FUSAAREA_TOTAL	((128+256+512+512+512+64)*1024*1024)
#define	HHOPE_G2N_ECCAREA		0x484000000
#define	HHOPE_G2N_ECCAREA_TOTAL		((128+256+512+512+512+64)*1024*1024)
#define	HHOPE_G2M_FUSAAREA		0x408000000
#define	HHOPE_G2M_FUSAAREA_TOTAL	((1920)*1024*1024)
#define	HHOPE_G2M_ECCAREA		0x608000000
#define	HHOPE_G2M_ECCAREA_TOTAL		((1920)*1024*1024)

#if (RZG_DRAM_ECC_FULL == 1)
#define	HHOPE_G2H_FUSAAREA1		0x408000000
#define	HHOPE_G2H_FUSAAREA1_TOTAL	((1920)*1024*1024)
#define	HHOPE_G2H_ECCAREA1		0x608000000
#define	HHOPE_G2H_ECCAREA1_TOTAL	((1920)*1024*1024)
#else
#define	HHOPE_G2H_FUSAAREA1		0x408000000
#define	HHOPE_G2H_FUSAAREA1_TOTAL	((960)*1024*1024)
#define	HHOPE_G2H_ECCAREA1		0x444000000
#define	HHOPE_G2H_ECCAREA1_TOTAL	((960)*1024*1024)

#define	HHOPE_G2H_FUSAAREA2		0x600000000
#define	HHOPE_G2H_FUSAAREA2_TOTAL	((1024)*1024*1024)
#define	HHOPE_G2H_ECCAREA2		0x640000000
#define	HHOPE_G2H_ECCAREA2_TOTAL	((1024)*1024*1024)
#endif

struct rzg2_ecc_conf {
	uint32_t fusaareacr;
	uint32_t eccareacr;
};

#if (RZG_DRAM_ECC == 1 && RZG_EK874 == 1)
static const struct rzg2_ecc_conf rzg2_ek874_conf[] = {
#if (RZG_DRAM_ECC_FULL == 2) // ECC Full mode single channel
	{ FUSAAREACR(1, 7, 0x408000000),    ECCAREACR(0, 0x448000000)   }, /* 128+128 MB */
	{ FUSAAREACR(1, 8, 0x410000000),    ECCAREACR(0, 0x450000000)   }, /* 256+256 MB */
	{ FUSAAREACR(1, 8, 0x420000000),    ECCAREACR(0, 0x460000000)   }, /* 256+256 MB */
	{ FUSAAREACR(1, 8, 0x430000000),    ECCAREACR(0, 0x470000000)   }, /* 256+256 MB */
	{ FUSAAREACR(1, 6, 0x440000000),    ECCAREACR(0, 0x444000000)   }, /* 64+64 MB */
#else //(RZG_DRAM_ECC_FULL == 2)
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
#endif //(RZG_DRAM_ECC_FULL == 2)
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
	{ FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
};
#endif

#if (RZG_DRAM_ECC == 1 && RZG_HIHOPE_RZG2N == 1)
static const struct rzg2_ecc_conf rzg2_hihope_rzg2n_conf[] = {
#if (RZG_DRAM_ECC_FULL == 2) // ECC Full mode single channel
        { FUSAAREACR(1, 7, 0x408000000),    ECCAREACR(0, 0x488000000)   }, /* 128+128 MB */
        { FUSAAREACR(1, 8, 0x410000000),    ECCAREACR(0, 0x490000000)   }, /* 256+256 MB */
        { FUSAAREACR(1, 9, 0x420000000),    ECCAREACR(0, 0x4A0000000)   }, /* 512+512 MB */
        { FUSAAREACR(1, 9, 0x440000000),    ECCAREACR(0, 0x4C0000000)   }, /* 512+512 MB */
        { FUSAAREACR(1, 9, 0x460000000),    ECCAREACR(0, 0x4E0000000)   }, /* 512+512 MB */
        { FUSAAREACR(1, 6, 0x480000000),    ECCAREACR(0, 0x484000000)   }, /* 64+64 MB */
#else //(RZG_DRAM_ECC_FULL == 2)
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
#endif //(RZG_DRAM_ECC_FULL == 2)
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
        { FUSAAREACR(0, 0, 0),    ECCAREACR(0, 0)   },
};
#endif

#if (RZG_DRAM_ECC == 1 && RZG_HIHOPE_RZG2H == 1)
static const struct rzg2_ecc_conf rzg2_hihope_rzg2h_conf[] = {
#if (RZG_DRAM_ECC_FULL == 2)   // ECC Full mode single channel
	{ FUSAAREACR(1, 6, 0x408000000), ECCAREACR(0, 0x444000000) }, /* 64+64 MB */
	{ FUSAAREACR(1, 7, 0x40C000000), ECCAREACR(0, 0x448000000) }, /* 128+128 MB */
	{ FUSAAREACR(1, 8, 0x414000000), ECCAREACR(0, 0x450000000) }, /* 256+256 MB */
	{ FUSAAREACR(1, 9, 0x424000000), ECCAREACR(0, 0x460000000) }, /* 512+512 MB */
	{ FUSAAREACR(1, 9, 0x600000000), ECCAREACR(0, 0x640000000) }, /* 512+512 MB */
	{ FUSAAREACR(1, 9, 0x620000000), ECCAREACR(0, 0x660000000) }, /* 512+512 MB */
#else
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
#endif //(RZG_DRAM_ECC_FULL == 2)
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
	{ FUSAAREACR(0, 0, 0), ECCAREACR(0, 0) },
};
#endif

#if ((RZG_HIHOPE_RZG2M == 1 || RZG_HIHOPE_RZG2H == 1) && RZG_DRAM_ECC == 1)
#if (RZG_DRAM_ECC_FULL == 1) // ECC Full mode dual channel
static const uint32_t fusacr = EFUSASEL(0xF0) | DFUSASEL(0xFE)| SFUSASEL(0);
static const uint32_t adsplcr0 = ADRMODE(0) | SPLITSEL(1) | AREA(0x1C) | SWP(0);
static const uint32_t adsplcr1 = SPLITSEL(1) | AREA(0x1C) | SWP(0);
static const uint32_t adsplcr2 = 0;
static const uint32_t adsplcr3 = SPLITSEL(0) | AREA(0x19) | SWP(0);
#else //(RZG_DRAM_ECC_FULL == 1)
static const uint32_t fusacr = 0;
static const uint32_t adsplcr0 = 0;
static const uint32_t adsplcr1 = 0;
static const uint32_t adsplcr2 = 0;
static const uint32_t adsplcr3 = 0;
#endif //(RZG_DRAM_ECC_FULL == 1)
#endif //((RZG_HIHOPE_RZG2M == 1 || RZG_HIHOPE_RZG2H == 1) && RZG_DRAM_ECC == 1)

#if ((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL != 0))
/* Write zero-valued octa-byte words */
static void bzero64(uintptr_t start, uint64_t size)
{
	/* start should be aligned on word boundary, size should be multiple of 8 */
	volatile uint64_t *ptr = (volatile uint64_t *)start;
	volatile uint64_t *end = ptr + (size / sizeof(uint64_t));

	while (ptr < end)
		*ptr++ = 0;
}
#endif //((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL != 0))

/* Setup DRAM ECC configuration registers */
#if (RZG_DRAM_ECC == 1 && RZG_EK874 == 1)
static void bl2_enable_ecc_ek874(void)
{
	const struct rzg2_ecc_conf *conf;
	int n;
	int nb_of_conf;

	conf = rzg2_ek874_conf;
	nb_of_conf = ARRAY_SIZE(rzg2_ek874_conf);

#if ((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 2))
	/* Clear DRAM ECC Area (for check) */
	NOTICE("BL2: Clearing ECC area from %lx\n", EK874_ECCAREA);
	bzero64(EK874_ECCAREA, (uint64_t)EK874_ECCAREA_TOTAL);
#endif //((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 1))

	for (n = 0; n < nb_of_conf; n++, conf++) {
		mmio_write_32((uintptr_t)((uint32_t *)DFUSAAREACR + n), conf->fusaareacr);
		mmio_write_32((uintptr_t)((uint32_t *)DECCAREACR + n), conf->eccareacr);
		if(conf->fusaareacr & ((uint32_t) 1 << 31))
			NOTICE("BL2: DRAM ECC area=%d, FuSa=0x%x ECC=0x%x, size=%d MB\n",
					n, conf->fusaareacr, conf->eccareacr,
					1 << ((conf->fusaareacr >> 24) & 0xf));

	}

#if ((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 2))
	/* Clear DRAM data area to initialize ECC area */
	NOTICE("BL2: Clearing DATA area from %lx\n", EK874_FUSAAREA);
	bzero64(EK874_FUSAAREA, (uint64_t)EK874_FUSAAREA_TOTAL);
#endif //((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 1))

}
#endif

#if (RZG_DRAM_ECC == 1 && RZG_HIHOPE_RZG2N == 1)
static void bl2_enable_ecc_hihope_rzg2n(void)
{
	const struct rzg2_ecc_conf *conf;
	int n;
	int nb_of_conf;

	conf = rzg2_hihope_rzg2n_conf;
	nb_of_conf = ARRAY_SIZE(rzg2_hihope_rzg2n_conf);

#if ((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 2))
	/* Clear DRAM ECC Area (for check) */
	NOTICE("BL2: Clearing ECC area from %lx\n", HHOPE_G2N_ECCAREA);
	bzero64(HHOPE_G2N_ECCAREA, (uint64_t)HHOPE_G2N_ECCAREA_TOTAL);
#endif //((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 1))

	for (n = 0; n < nb_of_conf; n++, conf++) {
		mmio_write_32((uintptr_t)((uint32_t *)DFUSAAREACR + n), conf->fusaareacr);
		mmio_write_32((uintptr_t)((uint32_t *)DECCAREACR + n), conf->eccareacr);
		if(conf->fusaareacr & ((uint32_t) 1 << 31))
			NOTICE("BL2: DRAM ECC area=%d, FuSa=0x%x ECC=0x%x, size=%d MB\n",
					n, conf->fusaareacr, conf->eccareacr,
					1 << ((conf->fusaareacr >> 24) & 0xf));
	}

#if ((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 2))
	/* Clear DRAM ECC Area (for check) */
	NOTICE("BL2: Clearing DATA area from %lx\n", HHOPE_G2N_FUSAAREA);
	bzero64(HHOPE_G2N_FUSAAREA, (uint64_t)HHOPE_G2N_FUSAAREA_TOTAL);
#endif //((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 2))

}
#endif

#if (RZG_DRAM_ECC == 1 && RZG_HIHOPE_RZG2M == 1)
static void bl2_enable_ecc_hihope_rzg2m(void)
{
#if ((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 1))
	/* Clear DRAM ECC Area (for check) */
	NOTICE("BL2: Clearing ECC area from %lx\n", HHOPE_G2M_ECCAREA);
	bzero64(HHOPE_G2M_ECCAREA, (uint64_t)HHOPE_G2M_ECCAREA_TOTAL);
#endif //((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 1))

	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR0), adsplcr0);
	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR1), adsplcr1);
	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR2), adsplcr2);
	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR3), adsplcr3);
	mmio_write_32((uintptr_t)((uint32_t *)FUSACR),	fusacr);

	NOTICE("BL2: DRAM ECC Configured:\n");
	NOTICE("     ADSPLCR0=0x%x\n", adsplcr0);
	NOTICE("     ADSPLCR1=0x%x\n", adsplcr1);
	NOTICE("     ADSPLCR2=0x%x\n", adsplcr2);
	NOTICE("     ADSPLCR3=0x%x\n", adsplcr3);
	NOTICE("     FUSACR=0x%x\n", fusacr);

#if ((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 1))
	/* Clear DRAM data area to initialize ECC area */
	NOTICE("BL2: Clearing DATA area from %lx\n", HHOPE_G2M_FUSAAREA);
	bzero64(HHOPE_G2M_FUSAAREA, (uint64_t)HHOPE_G2M_FUSAAREA_TOTAL);
#endif //((FUSA_DRAM_CLEAR == 1) && (RZG_DRAM_ECC_FULL == 1))

}
#endif

#if (RZG_DRAM_ECC == 1 && RZG_HIHOPE_RZG2H == 1)
void bl2_enable_ecc_hihope_rzg2h(void)
{
	const struct rzg2_ecc_conf *conf;
	int n;
	int nb_of_conf;

	conf = rzg2_hihope_rzg2h_conf;
	nb_of_conf = ARRAY_SIZE(rzg2_hihope_rzg2h_conf);

#if (FUSA_DRAM_CLEAR == 1)
	/* Clear DRAM ECC Area (for check) */
#if (RZG_DRAM_ECC_FULL != 0)
	NOTICE("BL2: Clearing ECC area from %lx\n", HHOPE_G2H_ECCAREA1);
	bzero64(HHOPE_G2H_ECCAREA1, (uint64_t)HHOPE_G2H_ECCAREA1_TOTAL);
#endif //(RZG_DRAM_ECC_FULL != 0)
#if (RZG_DRAM_ECC_FULL == 2)
	NOTICE("BL2: Clearing ECC area from %lx\n", HHOPE_G2H_ECCAREA2);
	bzero64(HHOPE_G2H_ECCAREA2, (uint64_t)HHOPE_G2H_ECCAREA2_TOTAL);
#endif //(RZG_DRAM_ECC_FULL == 2)
#endif //((FUSA_DRAM_CLEAR == 1)
	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR0), adsplcr0);
	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR1), adsplcr1);
	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR2), adsplcr2);
	mmio_write_32((uintptr_t)((uint32_t *)ADSPLCR3), adsplcr3);
	mmio_write_32((uintptr_t)((uint32_t *)FUSACR),fusacr);

	NOTICE("BL2: DRAM ECC Configured:\n");
	NOTICE("     ADSPLCR0=0x%x\n",adsplcr0);
	NOTICE("     ADSPLCR1=0x%x\n",adsplcr1);
	NOTICE("     ADSPLCR2=0x%x\n",adsplcr2);
	NOTICE("     ADSPLCR3=0x%x\n",adsplcr3);
	NOTICE("     FUSACR=0x%x\n",fusacr);
	for (n = 0; n < nb_of_conf; n++, conf++) {
		mmio_write_32((uintptr_t)((uint32_t *)DFUSAAREACR + n),
				conf->fusaareacr);
		mmio_write_32((uintptr_t)((uint32_t *)DECCAREACR + n),
				conf->eccareacr);
		if(conf->fusaareacr & ((uint32_t)1 << 31))
			NOTICE("BL2: DRAM ECC area=%d, FuSa=0x%x ECC=0x%x,\
				size=%d MB\n",
				n, conf->fusaareacr, conf->eccareacr,
				1 << ((conf->fusaareacr >> 24) & 0xf));
       }
#if (FUSA_DRAM_CLEAR == 1)
	/* Clear DRAM data area to initialize ECC area */
#if (RZG_DRAM_ECC_FULL != 0)
	NOTICE("BL2: Clearing DATA area from %lx\n", HHOPE_G2H_FUSAAREA1);
	bzero64(HHOPE_G2H_FUSAAREA1, (uint64_t)HHOPE_G2H_FUSAAREA1_TOTAL);
#endif //(RZG_DRAM_ECC_FULL != 0)
#if (RZG_DRAM_ECC_FULL == 2)
	NOTICE("BL2: Clearing DATA area from %lx\n", HHOPE_G2H_FUSAAREA2);
	bzero64(HHOPE_G2H_FUSAAREA2, (uint64_t)HHOPE_G2H_FUSAAREA2_TOTAL);
#endif //(RZG_DRAM_ECC_FULL == 2)
#endif //(FUSA_DRAM_CLEAR == 1)
}
#endif

void bl2_ecc_init(void)
{
#if (RZG_DRAM_ECC == 1)

#if (RZG_EK874 == 1)
	bl2_enable_ecc_ek874();
#elif (RZG_HIHOPE_RZG2M == 1)
	bl2_enable_ecc_hihope_rzg2m();
#elif (RZG_HIHOPE_RZG2N == 1)
	bl2_enable_ecc_hihope_rzg2n();
#elif (RZG_HIHOPE_RZG2H == 1)
	bl2_enable_ecc_hihope_rzg2h();
	return;
#else
#error "Don't have ECC initialize routine(unknown)."
#endif

#endif /* RZG_DRAM_ECC == 1 */
}
