
#include <stdlib.h>

#include "vmtcp.h"

#include "lua.h"
#include "lauxlib.h"

#define LUA_TCP "tcp"

typedef struct {
    VM_TCP_HANDLE handle;
    int cb_ref;
    lua_State* L;
} tcp_info_t;

static void __tcp_callback(VM_TCP_HANDLE handle, VM_TCP_EVENT event, void* user_data)
{
    tcp_info_t* p = (tcp_info_t*)user_data;
    lua_State* L = p->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, p->cb_ref);
    lua_pushlightuserdata(L, p);
    luaL_getmetatable(L, LUA_TCP);
    lua_setmetatable(L, -2);
    lua_pushinteger(L, (int)event);
    lua_call(L, 2, 0);
}

int tcp_connect(lua_State* L)
{
    tcp_info_t* p;
    int ref;
    char* addr = luaL_checkstring(L, 1);
    unsigned port = luaL_checkinteger(L, 2);

    lua_pushvalue(L, 3);

    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    p = (tcp_info_t*)lua_newuserdata(L, sizeof(tcp_info_t));

    luaL_getmetatable(L, LUA_TCP);
    lua_setmetatable(L, -2);

    p->L = L;
    p->cb_ref = ref;
    p->handle = vm_tcp_connect(addr, port, VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN, p, __tcp_callback);

    return 1;
}

int tcp_write(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));
    int len;
    char* str = luaL_checklstring(L, 2, &len);

    lua_pushinteger(L, vm_tcp_write(p->handle, str, len));

    return 1;
}

int tcp_read(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));
    size_t size = luaL_checkinteger(L, 2);
    luaL_Buffer b;
    int nread;
    int i;
    int ret;

    char* buf = malloc(size);
    if(buf == 0) {
        return luaL_error(L, "malloc() failed");
    }

    nread = vm_tcp_read(p->handle, buf, size);
    if(nread < 0) {
        ret = luaL_error(L, "tcp failed to read");
    } else if(nread == 0) {
        // receives the FIN from the server
        ret = 0;
    } else {
        luaL_buffinit(L, &b);
        for(i = 0; i < size; i++) {
            luaL_addchar(&b, buf[i]);
        }

        luaL_pushresult(&b);
        ret = 1;
    }

    free(buf);
    return ret;
}

int tcp_close(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));

    vm_tcp_close(p->handle);

    return 0;
}

int tcp_gc(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));

    return 0;
}

int tcp_tostring(lua_State* L)
{
    tcp_info_t* p = ((tcp_info_t*)luaL_checkudata(L, 1, LUA_TCP));
    lua_pushfstring(L, "tcp (%p)", p->handle);
    return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE tcp_map[] = { { LSTRKEY("connect"), LFUNCVAL(tcp_connect) },
                                 { LSTRKEY("read"), LFUNCVAL(tcp_read) },
                                 { LSTRKEY("write"), LFUNCVAL(tcp_write) },
                                 { LSTRKEY("close"), LFUNCVAL(tcp_close) },
                                 { LNILKEY, LNILVAL } };

const LUA_REG_TYPE tcp_type_table[] = { { LSTRKEY("read"), LFUNCVAL(tcp_read) },
                                        { LSTRKEY("write"), LFUNCVAL(tcp_write) },
                                        { LSTRKEY("close"), LFUNCVAL(tcp_close) },
                                        { LSTRKEY("__gc"), LFUNCVAL(tcp_gc) },
                                        { LSTRKEY("__tostring"), LFUNCVAL(tcp_tostring) },
                                        { LNILKEY, LNILVAL } };

LUALIB_API int luaopen_tcp(lua_State* L)
{
    luaL_newmetatable(L, LUA_TCP);     /* create metatable for file handles */
    lua_pushvalue(L, -1);                /* push metatable */
    lua_setfield(L, -2, "__index");      /* metatable.__index = metatable */
    luaL_register(L, NULL, tcp_type_table); /* file methods */

    luaL_register(L, "tcp", tcp_map);
    return 1;
}