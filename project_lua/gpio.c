
#include "vmtimer.h"

#include "lua.h"
#include "lauxlib.h"

#define LUA_TIMER   "timer"

typedef struct {
    VM_TIMER_ID_PRECISE timer_id;
    int cb_ref;
    lua_State *L;
} timer_info_t;

static void __timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
    timer_info_t *p = (timer_info_t *)user_data;
    lua_State *L = p->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, p->cb_ref);
    lua_call(L, 0, 0);
}

int timer_create(lua_State *L)
{
    timer_info_t *p;
    int ref;
    unsigned interval = luaL_checkinteger(L, 1);

    lua_pushvalue(L, 2);

    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    p = (timer_info_t *)lua_newuserdata(L, sizeof(timer_info_t));

    luaL_getmetatable(L, LUA_TIMER);
    lua_setmetatable(L, -2);

    p->L = L;
    p->cb_ref = ref;
    p->timer_id = vm_timer_create_precise(interval, __timer_callback, p);

    return 1;
}

int timer_delete(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    vm_timer_delete_precise(p->timer_id);

    return 0;
}

int timer_gc(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));

    vm_timer_delete_precise(p->timer_id);
    return 0;
}

int timer_tostring(lua_State *L)
{
    timer_info_t *p = ((timer_info_t *)luaL_checkudata(L, -1, LUA_TIMER));
    lua_pushfstring(L, "timer (%p)", p->timer_id);
    return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE timer_map[] =
{
    {LSTRKEY("create"), LFUNCVAL(timer_create)},
    {LSTRKEY("delete"), LFUNCVAL(timer_delete)},
    {LNILKEY, LNILVAL}
};

const LUA_REG_TYPE timer_table[] = {
  {LSTRKEY("delete"), LFUNCVAL(timer_delete)},
  {LSTRKEY("__gc"), LFUNCVAL(timer_gc)},
  {LSTRKEY("__tostring"), LFUNCVAL(timer_tostring)},
  {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_timer(lua_State *L)
{
    luaL_newmetatable(L, LUA_TIMER);  /* create metatable for file handles */
    lua_pushvalue(L, -1);  /* push metatable */
    lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
    luaL_register(L, NULL, timer_table);  /* file methods */

    luaL_register(L, "timer", timer_map);
    return 1;
}
