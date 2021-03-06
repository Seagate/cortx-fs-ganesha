/*
 *
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright © 2012 CohortFS, LLC.
 * Author: Adam C. Emerson <aemerson@linuxbox.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/* TODO:PORTING: pNFS support is disabled */
/**
 * @file   ds.c
 *
 * @brief pNFS DS operations for KVSFS
 *
 * This file implements the read, write, commit, and dispose
 * operations for KVSFS data-server handles.
 *
 * Also, creating a data server handle -- now called via the DS itself.
 */

#include "config.h"

#include <assert.h>
#include <libgen.h>		/* used for 'dirname' */
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <mntent.h>
#include "gsh_list.h"
#include "fsal.h"
#include "fsal_internal.h"
#include "fsal_convert.h"
#include "fsal_api.h"		/* Later, Include fsal_private.h instead */
#include "FSAL/fsal_config.h"
#include "FSAL/fsal_commonlib.h"
#include "kvsfs_methods.h"
#include "nfs_exports.h"
#include "nfs_creds.h"
#include "pnfs_utils.h"
#include <stdbool.h>

extern struct fsal_pnfs_ds_ops def_pnfs_ds_ops;
extern struct fsal_dsh_ops def_dsh_ops;

/**
 * @brief Release a DS handle
 *
 * @param[in] ds_pub The object to release
 */
static void
kvsfs_release(struct fsal_ds_handle *const ds_pub)
{
	/* The private 'full' DS handle */
	struct kvsfs_ds *ds = container_of(ds_pub,
					   struct kvsfs_ds,
					   ds);

	fsal_ds_handle_fini(&ds->ds);
	gsh_free(ds);
}

/**
 * @brief Read from a data-server handle.
 *
 * NFSv4.1 data server handles are disjount from normal
 * filehandles (in Ganesha, there is a ds_flag in the filehandle_v4_t
 * structure) and do not get loaded into cache_inode or processed the
 * normal way.
 *
 * @param[in]  ds_pub           FSAL DS handle
 * @param[in]  req_ctx          Credentials
 * @param[in]  stateid          The stateid supplied with the READ operation,
 *                              for validation
 * @param[in]  offset           The offset at which to read
 * @param[in]  requested_length Length of read requested (and size of buffer)
 * @param[out] buffer           The buffer to which to store read data
 * @param[out] supplied_length  Length of data read
 * @param[out] eof              True on end of file
 *
 * @return An NFSv4.1 status code.
 */
static nfsstat4
kvsfs_ds_read(struct fsal_ds_handle *const ds_pub,
		struct req_op_context *const req_ctx,
		const stateid4 *stateid,
		const offset4 offset,
		const count4 requested_length,
		void *const buffer,
		count4 *const supplied_length,
		bool *const end_of_file)
{
	/* The private 'full' DS handle */
	struct kvsfs_ds *ds = container_of(ds_pub, struct kvsfs_ds, ds);
	struct kvsfs_file_handle *kvsfs_fh = &ds->wire;
	/* The amount actually read */
	ssize_t amount_read = 0;
	cfs_cred_t cred;
	struct kvsfs_fsal_obj_handle;
	struct kvsfs_file_state fd = {0};
	struct kvsfs_fsal_export *kvsfs_fsal_export = 
		container_of(ds_pub->pds->mds_fsal_export,
			     struct kvsfs_fsal_export, export);
	
	LogDebug(COMPONENT_PNFS," >> ENTER kvsfs_ds_read \n");

	fd.cfs_fd.ino = kvsfs_fh->kvsfs_handle;

	cortxfs_cred_from_op_ctx(&cred);

	/* read the data */
	amount_read = cfs_read(kvsfs_fsal_export->cfs_fs, &cred, &fd.cfs_fd,
				buffer,requested_length, offset);
	if (amount_read < 0) {
		LogCrit(COMPONENT_FSAL,
			"Error in cfs_read\n");
		/* ignore any potential error on close if read failed? */

		return posix2nfs4_error(-amount_read);
	}


	*supplied_length = amount_read;
	*end_of_file = amount_read == 0 ? true : false;

	LogDebug(COMPONENT_PNFS," >> EXIT kvsfs_ds_read\n");
	return NFS4_OK;
}

/**
 *
 * @brief Write to a data-server handle.
 *
 * This performs a DS write not going through the data server unless
 * FILE_SYNC4 is specified, in which case it connects the filehandle
 * and performs an MDS write.
 *
 * @param[in]  ds_pub           FSAL DS handle
 * @param[in]  req_ctx          Credentials
 * @param[in]  stateid          The stateid supplied with the READ operation,
 *                              for validation
 * @param[in]  offset           The offset at which to read
 * @param[in]  write_length     Length of write requested (and size of buffer)
 * @param[out] buffer           The buffer to which to store read data
 * @param[in]  stability wanted Stability of write
 * @param[out] written_length   Length of data written
 * @param[out] writeverf        Write verifier
 * @param[out] stability_got    Stability used for write (must be as
 *                              or more stable than request)
 *
 * @return An NFSv4.1 status code.
 */
static nfsstat4
kvsfs_ds_write(struct fsal_ds_handle *const ds_pub,
		struct req_op_context *const req_ctx,
		const stateid4 *stateid,
		const offset4 offset,
		const count4 write_length,
		const void *buffer,
		const stable_how4 stability_wanted,
		count4 *written_length,
		verifier4 *writeverf,
		stable_how4 *stability_got)
{
	/* The private 'full' DS handle */
	struct kvsfs_ds *ds = container_of(ds_pub, struct kvsfs_ds, ds);
	struct kvsfs_file_handle *kvsfs_fh = &ds->wire;
	/* The amount actually read */
	ssize_t amount_written = 0;
	cfs_cred_t cred;
	struct kvsfs_fsal_obj_handle;
	struct kvsfs_file_state fd = {0};
	struct kvsfs_fsal_export *kvsfs_fsal_export = 
		container_of(ds_pub->pds->mds_fsal_export,
			     struct kvsfs_fsal_export, export);
	void * buf = (void *)buffer;
	/* @todo: To supress compilation warnings, a const pointer
	 *	  buffer has been explicitly typecasted to non-const
	 *	  pointer buf, but a more cleaner appraoch would be
	 *	  to align the entire stack to use const everywhere
	 *	  in order to maintain consistency.
	 */

	fd.cfs_fd.ino = kvsfs_fh->kvsfs_handle;

	LogDebug(COMPONENT_PNFS," >> ENTER kvsfs_ds_write\n");

	memset(writeverf, 0, NFS4_VERIFIER_SIZE);

	cortxfs_cred_from_op_ctx(&cred);

	/** @todo Add some debug code here about the fh to be used */

	/* @todo: We currently do not have any support for writeverf */

	/* write the data */
	amount_written = cfs_write(kvsfs_fsal_export->cfs_fs, &cred, &fd.cfs_fd,
				   buf,(const)write_length, offset);
	if (amount_written < 0) {
		return posix2nfs4_error(-amount_written);
	}


	*written_length = amount_written;
	
	/* CORTX FS only provides file sync stability, rest are not supported */
	*stability_got = FILE_SYNC4;

	LogDebug(COMPONENT_PNFS," >> EXIT kvsfs_ds_write\n");
	return NFS4_OK;
}

/**
 * @brief Commit a byte range to a DS handle.
 *
 * NFSv4.1 data server filehandles are disjount from normal
 * filehandles (in Ganesha, there is a ds_flag in the filehandle_v4_t
 * structure) and do not get loaded into cache_inode or processed the
 * normal way.
 *
 * @param[in]  ds_pub    FSAL DS handle
 * @param[in]  req_ctx   Credentials
 * @param[in]  offset    Start of commit window
 * @param[in]  count     Length of commit window
 * @param[out] writeverf Write verifier
 *
 * @return An NFSv4.1 status code.
 */


static nfsstat4
kvsfs_ds_commit(struct fsal_ds_handle *const ds_pub,
		struct req_op_context *const req_ctx,
		const offset4 offset,
		const count4 count,
		verifier4 *const writeverf)
{
	memset(writeverf, 0, NFS4_VERIFIER_SIZE);
	return NFS4_OK;
}

static void
dsh_ops_init(struct fsal_dsh_ops *ops)
{
	memcpy(ops, &def_dsh_ops, sizeof(struct fsal_dsh_ops));

	ops->release = kvsfs_release;
	ops->read = kvsfs_ds_read;
	ops->write = kvsfs_ds_write;
	ops->commit = kvsfs_ds_commit;
}

/**
 * @brief Try to create a FSAL data server handle
 *
 * @param[in]  pds      FSAL pNFS DS
 * @param[in]  desc     Buffer from which to create the file
 * @param[out] handle   FSAL DS handle
 *
 * @return NFSv4.1 error codes.
 */

static nfsstat4 make_ds_handle(struct fsal_pnfs_ds *const pds,
				const struct gsh_buffdesc *const desc,
				struct fsal_ds_handle **const handle,
				int flags)
{
	struct kvsfs_ds *ds;		/* Handle to be created */

	*handle = NULL;

	if (desc->len != sizeof(struct kvsfs_file_handle))
		return NFS4ERR_BADHANDLE;

	ds = gsh_calloc(1, sizeof(struct kvsfs_ds));

	*handle = &ds->ds;
	fsal_ds_handle_init(*handle, pds);

	/* Connect lazily when a FILE_SYNC4 write forces us to, not
	   here. */

	ds->connected = false;

	memcpy(&ds->wire, desc->addr, desc->len);
	return NFS4_OK;
}

static nfsstat4 pds_permissions(struct fsal_pnfs_ds *const pds,
				struct svc_req *req)
{
	/* special case: related export has been set */
	return nfs4_export_check_access(req);
}

void kvsfs_pnfs_ds_ops_init(struct fsal_pnfs_ds_ops *ops)
{
	memcpy(ops, &def_pnfs_ds_ops, sizeof(struct fsal_pnfs_ds_ops));
	ops->permissions = pds_permissions;
	ops->make_ds_handle = make_ds_handle;
	ops->fsal_dsh_ops = dsh_ops_init;
}

