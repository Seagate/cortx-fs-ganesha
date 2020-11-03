/*
 * Filename:	cfs_ganesha_perfc.h
 * Description:	This module defines performance counters and helpers.
 *
 * Copyright (c) 2020 Seagate Technology LLC and/or its Affiliates
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * For any questions about this software or licensing,
 * please email opensource@seagate.com or cortx-questions@seagate.com. 
 */

#ifndef __CFS_GANESHA_PERF_COUNTERS_H_
#define __CFS_GANESHA_PERF_COUNTERS_H_ 
/******************************************************************************/
#include "perf/tsdb.h" /* ACTION_ID_BASE */
#include "operation.h"
#include <pthread.h>
#include <string.h>
#include "debug.h"
#include "perf/perf-counters.h"

enum perfc_fsal_function_tags {
	PFT_FSAL_START = PFTR_RANGE_2_START,
	PFT_FSAL_READ,
	PFT_FSAL_WRITE,
	PFT_FSAL_GETATTRS,
	PFT_FSAL_SETATTRS,
	PFT_FSAL_MKDIR,
	PFT_FSAL_RMDIR,
	PFT_FSAL_READDIR,
	PFT_FSAL_LOOKUP,
	PFT_FSAL_END = PFTR_RANGE_2_END
};

enum perfc_fsal_entity_attrs {
	PEA_FSAL_START = PEAR_RANGE_2_START,
	PEA_W_OFFSET,
	PEA_W_SIZE,
	PEA_W_RES_MAJ,
	PEA_W_RES_MIN,
	PEA_R_OFFSET,
	PEA_R_IOVC,
	PEA_R_IOVL,
	PEA_R_RES_MAJ,
	PEA_R_RES_MIN,
	PEA_GETATTR_RES_MAJ,
	PEA_GETATTR_RES_MIN,
	PEA_TIME_ATTR_START_OTHER_FUNC,
	PEA_TIME_ATTR_END_OTHER_FUNC,
	PEA_SETATTR_RES_MAJ,
	PEA_SETATTR_RES_MIN,
	PEA_FSAL_END = PEAR_RANGE_2_END
};

enum perfc_fsal_entity_maps {
	PEM_FSAL_START = PEMR_RANGE_2_START,
	PEM_FSAL_END = PEMR_RANGE_2_END
};

/******************************************************************************/
#endif /* __CFS_GANESHA_PERF_COUNTERS_H_ */
