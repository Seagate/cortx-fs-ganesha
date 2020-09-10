#ifndef FSAL_PERF_H_
#define FSAL_PERF_H_

#include "operation.h"
#include <pthread.h>

/**
 * The scope for "fuser_ops" enumeration.
 * This is the module context for all the operations
 * being originated from this module, i.e. FSAL_CORTXFS
 */
extern struct modctx g_cfs_perf_mod;

/**
 * Defines a set of calls to be traced by the Operation API.
 * This is used as operation type tag
 */
enum fsuser_ops {
	FSUSER_OP_WRITE = 1,
	// TODO: Add here for operations of future CFS FSAL handlers
};

extern pthread_key_t tls_op_key;

#ifdef ENABLE_TSDB_ADDB

/** Store a TSDB operation in TLS */
#define cfs_perf_op_ini(op, mod, opcode, ...) do {	\
	int rc;						\
	opstack_begin(op, mod, opcode, __VA_ARGS__);	\
	rc = pthread_setspecific(tls_op_key, op);	\
	dassert(rc == 0);				\
} while(0)

/** Read the TLS op key and return, NULL if no key set */
#define cfs_perf_op_get pthread_getspecific(tls_op_key)\

/** Unset the TLS unconditionally */
#define cfs_perf_op_fini(op, ...) do {			\
	int rc;						\
	rc = pthread_setspecific(tls_op_key, NULL);	\
	dassert(rc == 0);				\
	opstack_end(op, __VA_ARGS__);			\
} while(0)

/** Set a TSDB action record */
#define cfs_perf_action(pfc_action_id, ...) \
	opstack_action(cfs_perf_op_get, pfc_action_id, __VA_ARGS__)
#else

#define cfs_perf_op_ini(op, mod, opcode, ...)

#define cfs_perf_op_get

#define cfs_perf_op_fini(op, ...)

#define cfs_perf_action(pfc_action_id, ...)

#define cfs_perf_action(pfc_action_id, ...)

#endif /* ENABLE_TSDB_ADDB */

#endif /** FSAL_PERF_H_ */
