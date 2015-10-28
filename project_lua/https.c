
#include <stdlib.h>
#include <string.h>

#include "vmhttps.h"
#include "vmlog.h"

#include "lua.h"
#include "lauxlib.h"

extern lua_State* L;

int g_channel_id;
int g_read_seg_num;
char g_https_url[64] = {0,};
int g_https_response_cb_ref = LUA_NOREF;

static void https_send_request_set_channel_rsp_cb(VMUINT32 req_id, VMUINT8 channel_id, VMUINT8 result)
{
    VMINT ret = -1;

    g_channel_id = channel_id;
    ret = vm_https_send_request(0,                         /* Request ID */
                                VM_HTTPS_METHOD_GET,       /* HTTP Method Constant */
                                VM_HTTPS_OPTION_NO_CACHE,  /* HTTP request options */
                                VM_HTTPS_DATA_TYPE_BUFFER, /* Reply type (wps_data_type_enum) */
                                128, /* bytes of data to be sent in reply at a time. If data is more that this, multiple
                                        response would be there */
                                g_https_url,         /* The request URL */
                                strlen(g_https_url), /* The request URL length */
                                NULL,                /* The request header */
                                0,                   /* The request header length */
                                NULL,
                                0);

    if(ret != 0) {
        vm_https_unset_channel(channel_id);
    }
}

static void https_unset_channel_rsp_cb(VMUINT8 channel_id, VMUINT8 result)
{
    vm_log_debug("https_unset_channel_rsp_cb()");
}

static void https_send_release_all_req_rsp_cb(VMUINT8 result)
{
    vm_log_debug("https_send_release_all_req_rsp_cb()");
}

static void https_send_termination_ind_cb(void)
{
    vm_log_debug("https_send_termination_ind_cb()");
}

static void https_send_read_request_rsp_cb(VMUINT16 request_id,
                                           VMUINT8 result,
                                           VMUINT16 status,
                                           VMINT32 cause,
                                           VMUINT8 protocol,
                                           VMUINT32 content_length,
                                           VMBOOL more,
                                           VMUINT8* content_type,
                                           VMUINT8 content_type_len,
                                           VMUINT8* new_url,
                                           VMUINT32 new_url_len,
                                           VMUINT8* reply_header,
                                           VMUINT32 reply_header_len,
                                           VMUINT8* reply_segment,
                                           VMUINT32 reply_segment_len)
{
    int ret = -1;

    if(result != 0) {
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
    } else {
        int i;
        luaL_Buffer b;

        lua_rawgeti(L, LUA_REGISTRYINDEX, g_https_response_cb_ref);

        luaL_buffinit(L, &b);
        for(i = 0; i < reply_header_len; i++) {
            luaL_addchar(&b, reply_header[i]);
        }

        for(i = 0; i < reply_segment_len; i++) {
            luaL_addchar(&b, reply_segment[i]);
        }

        luaL_pushresult(&b);
        lua_pushinteger(L, more);

        lua_call(L, 2, 0);

        if(more) {
            ret = vm_https_read_content(request_id, ++g_read_seg_num, 128);
            if(ret != 0) {
                vm_https_cancel(request_id);
                vm_https_unset_channel(g_channel_id);
            }
        }
    }
}
static void https_send_read_read_content_rsp_cb(VMUINT16 request_id,
                                                VMUINT8 seq_num,
                                                VMUINT8 result,
                                                VMBOOL more,
                                                VMUINT8* reply_segment,
                                                VMUINT32 reply_segment_len)
{
    int ret = -1;
    int i;
    luaL_Buffer b;

    lua_rawgeti(L, LUA_REGISTRYINDEX, g_https_response_cb_ref);

    luaL_buffinit(L, &b);
    for(i = 0; i < reply_segment_len; i++) {
        luaL_addchar(&b, reply_segment[i]);
    }

    luaL_pushresult(&b);
    lua_pushinteger(L, more);

    lua_call(L, 2, 0);

    if(more > 0) {
        ret = vm_https_read_content(request_id,       /* Request ID */
                                    ++g_read_seg_num, /* Sequence number (for debug purpose) */
                                    128); /* The suggested segment data length of replied data in the peer buffer of
                                             response. 0 means use reply_segment_len in MSG_ID_WPS_HTTP_REQ or
                                             read_segment_length in previous request. */
        if(ret != 0) {
            vm_https_cancel(request_id);
            vm_https_unset_channel(g_channel_id);
        }
    } else {
        /* don't want to send more requests, so unset channel */
        vm_https_cancel(request_id);
        vm_https_unset_channel(g_channel_id);
        g_channel_id = 0;
        g_read_seg_num = 0;
    }
}

static void https_send_cancel_rsp_cb(VMUINT16 request_id, VMUINT8 result)
{
    vm_log_debug("https_send_cancel_rsp_cb()");
}

static void https_send_status_query_rsp_cb(VMUINT8 status)
{
    vm_log_debug("https_send_status_query_rsp_cb()");
}

int https_get(lua_State* L)
{
    int ret;
    int apn = VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN;
    vm_https_callbacks_t callbacks = { https_send_request_set_channel_rsp_cb, https_unset_channel_rsp_cb,
                                       https_send_release_all_req_rsp_cb,     https_send_termination_ind_cb,
                                       https_send_read_request_rsp_cb,        https_send_read_read_content_rsp_cb,
                                       https_send_cancel_rsp_cb,              https_send_status_query_rsp_cb };
    char* url = luaL_checkstring(L, 1);

    strncpy(g_https_url, url, sizeof(g_https_url));
    lua_pushvalue(L, 2);

    g_https_response_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    ret = vm_https_register_context_and_callback(apn, &callbacks);
    if(ret != 0) {
        return luaL_error(L, "vm_https_register_context_and_callback() failed");
    }

    ret = vm_https_set_channel(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    lua_pushinteger(L, ret);

    return 1;
}

int https_post(lua_State* L)
{

    lua_pushinteger(L, 0);

    return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE https_map[] = { { LSTRKEY("get"), LFUNCVAL(https_get) },
                                 { LNILKEY, LNILVAL } };


LUALIB_API int luaopen_https(lua_State* L)
{
    luaL_register(L, "https", https_map);
    return 1;
}