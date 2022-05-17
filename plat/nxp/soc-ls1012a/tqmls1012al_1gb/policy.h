/*
 * Copyright 2018, 2021 NXP
 * Copyright 2022 TQ-Systems GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef POLICY_H
#define	POLICY_H

/*
 * Set this to 0x0 to leave the default SMMU page size in sACR
 * Set this to 0x1 to change the SMMU page size to 64K
 */
#define POLICY_SMMU_PAGESZ_64K 0x0

#endif /* POLICY_H */
