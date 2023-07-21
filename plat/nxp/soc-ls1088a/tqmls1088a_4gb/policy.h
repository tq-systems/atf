// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2018-2021 NXP
 * Copyright (c) 2023 TQ-Systems GmbH <oss@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 *
 */

#ifndef POLICY_H
#define POLICY_H

/* Set this to 0x0 to leave the default SMMU page size in sACR
 * Set this to 0x1 to change the SMMU page size to 64K
 */
#define POLICY_SMMU_PAGESZ_64K 0x1

#endif /* POLICY_H */
