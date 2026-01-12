// User-level IPC library routines

#include "inc/env.h"
#include "inc/error.h"
#include <inc/lib.h>

// Receive a value via IPC and return it.
// If 'pg' is nonnull, then any page sent by the sender will be mapped at
//	that address.
// If 'from_env_store' is nonnull, then store the IPC sender's envid in
//	*from_env_store.
// If 'perm_store' is nonnull, then store the IPC sender's page permission
//	in *perm_store (this is nonzero iff a page was successfully
//	transferred to 'pg').
// If the system call fails, then store 0 in *fromenv and *perm (if
//	they're nonnull) and return the error.
// Otherwise, return the value sent by the sender
//
// Hint:
//   Use 'thisenv' to discover the value and who sent it.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value, since that's
//   a perfectly valid place to map a page.)
int32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
	int r;

	envid_t envid = thisenv->env_id;
	//while (sys_env_msg_state(envid) == MSG_EMPTY) {
		//cprintf("msg empty: %x\n", envid);
	//	sys_yield();
	//}
	//cprintf("msg recv env: %x\n", envid);
	for(;;) {
		if ((r = sys_ipc_recv(pg == NULL ? (void *)UTOP : pg, from_env_store, perm_store)) < 0) {
			if (r == -E_IPC_MSG_NOT_FREE) {
				sys_yield();
				continue;
			} else {
				if (from_env_store != NULL) {
					*from_env_store = 0;
				}
				if (perm_store != NULL) {
					*perm_store = 0;
				}
				return r;
			}
		} else {
			break;
		}
	}
	return thisenv->env_ipc_value;
}

// Send 'val' (and 'pg' with 'perm', if 'pg' is nonnull) to 'toenv'.
// This function keeps trying until it succeeds.
// It should panic() on any error other than -E_IPC_NOT_RECV.
//
// Hint:
//   Use sys_yield() to be CPU-friendly.
//   If 'pg' is null, pass sys_ipc_try_send a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
	int r;
	
	//envid_t envid = thisenv->env_id;
	//while (sys_env_msg_state(to_env) == MSG_FULL) {
		//cprintf("msg full: %x\n", to_env);
	//	sys_yield();
	//}
	//cprintf("ipc_send val: %u  env: %x\n", val, thisenv->env_id);
	pg = pg == NULL ? (void *) UTOP : pg;
	//for (;;) {
top:
	if ((r = sys_ipc_try_send(to_env, val, pg, perm)) < 0) {
		if (r == -E_IPC_MSG_NOT_FREE) {
			sys_yield();
			goto top;
		}
		if (r == -E_IPC_NOT_RECV) {
			cprintf("Failed to send value: %u", val);
			//continue;
		} else {
			panic("ipc_send: sys_ipc_try_send: %e\n", r);
		}
	} //else {
		//break;
	//}
	//}
	//sys_yield();
}

// Find the first environment of the given type.  We'll use this to
// find special environments.
// Returns 0 if no such environment exists.
envid_t
ipc_find_env(enum EnvType type)
{
	int i;
	for (i = 0; i < NENV; i++)
		if (envs[i].env_type == type)
			return envs[i].env_id;
	return 0;
}
