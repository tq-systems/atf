#
# Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_AUTO})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_m3.c
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2N/pfc_init_m3n.c

else ifdef RCAR_LSI_CUT_COMPAT
  ifeq (${RCAR_LSI},${RCAR_M3})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_m3.c
  endif
  ifeq (${RCAR_LSI},${RCAR_M3N})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2N/pfc_init_m3n.c
  endif
  ifeq (${RCAR_LSI},${RCAR_E3})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2E/pfc_init_e3.c
  endif
else
  ifeq (${RCAR_LSI},${RCAR_M3})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2M/pfc_init_m3.c
  endif
  ifeq (${RCAR_LSI},${RCAR_M3N})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2N/pfc_init_m3n.c
  endif
  ifeq (${RCAR_LSI},${RCAR_E3})
    BL2_SOURCES += drivers/renesas/rzg/pfc/G2E/pfc_init_e3.c
  endif
endif

BL2_SOURCES += drivers/renesas/rzg/pfc/pfc_init.c
