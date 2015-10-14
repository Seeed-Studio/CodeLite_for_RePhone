
#ifndef __SHELL_H__
#define __SHELL_H__

#include "vmthread.h"
#include "lua.h"

#define SHELL_MESSAGE_ID        326

VMINT32 shell_thread(VM_THREAD_HANDLE thread_handle, void* user_data);

void shell_docall(lua_State *L);

#endif // __SHELL_H__
