/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2018 Arm Limited
 *
 */
#ifndef __PAL_SDEI_INTERFACE_H
#define __PAL_SDEI_INTERFACE_H

#ifndef ACPI_SIG_SDEI
#define ACPI_SIG_SDEI "SDEI"
#endif

/* SDEI return values */
#define SDEI_SUCCESS        0
#define SDEI_NOT_SUPPORTED  -1
#define SDEI_INVALID_PARAMETERS -2
#define SDEI_DENIED     -3
#define SDEI_PENDING        -5
#define SDEI_OUT_OF_RESOURCE    -10

#ifndef ACPI_HEST_NOTIFY_SDEI
#define ACPI_HEST_NOTIFY_SDEI   0xB
#endif

enum sdei_conduit_types {
    CONDUIT_INVALID = 0,
    CONDUIT_SMC,
    CONDUIT_HVC,
};


#endif /* __PAL_SDEI_INTERFACE_H */
