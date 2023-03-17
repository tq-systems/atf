// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2018-2021 NXP
 * Copyright (c) 2023 TQ-Systems GmbH <oss@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 *
 */

#ifndef _POLICY_H
#define	_POLICY_H

/* Following defines affect the PLATFORM SECURITY POLICY */

/* set this to 0x0 if the platform is not using/responding to ECC errors
 * set this to 0x1 if ECC is being used (we have to do some init)
 */
#define  POLICY_USING_ECC 0x0

/* Set this to 0x0 to leave the default SMMU page size in sACR
 * Set this to 0x1 to change the SMMU page size to 64K
 */
#define POLICY_SMMU_PAGESZ_64K 0x1


/*
 * set this to '1' if the debug clocks need to remain enabled during
 * system entry to low-power (LPM20) - this should only be necessary
 * for testing and NEVER set for normal production
 */
#define POLICY_DEBUG_ENABLE 0


#endif /* _POLICY_H */
