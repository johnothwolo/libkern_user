//
//  main.m
//  testobj
//
//  Created by John Othwolo on 12/6/19.
//  Copyright © 2019 oth. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <dlfcn.h>
#import <time.h>
#import <os/lock.h>
#import <vector>


typedef struct os_log_pack_s {
    uint64_t olp_continuous_time;
    struct timespec olp_wall_time;
    const void* olp_mh;
    const void* olp_pc;
    const char* olp_format;
    uint64_t olp_priv_0;
    uint64_t olp_priv_1;
    uint16_t olp_errno;
    uint16_t olp_size;
} *os_log_pack_t;

typedef void (*os_crash_callback_t) (const char *);

typedef void     (*os_crash_fmt_func)(os_log_pack_t pack, size_t pack_size);
typedef size_t   (*os_log_pack_size_func)(size_t os_log_format_buffer_size);
typedef uint8_t  (*os_log_pack_fill_func)(os_log_pack_t pack, size_t size,
                                  uint16_t saved_errno,
                                  const void* dso, const char* format);

static void __os_crash_fmt(char *fmt, ...){
    va_list ap;
    char *output = NULL;
    int ret;
    // get string...
    va_start (ap, fmt);
    ret = vasprintf (&output, fmt, ap);
    va_end (ap);
    if (ret<0) abort();

    auto test_sz = strlen(output);

    // get functions
    auto _os_log_pack_fill = (os_log_pack_fill_func) dlsym(RTLD_DEFAULT, "_os_log_pack_fill");
    auto _os_log_pack_size = (os_log_pack_size_func) dlsym(RTLD_DEFAULT, "_os_log_pack_size");
    auto _os_crash_fmt = (os_crash_fmt_func) dlsym(RTLD_DEFAULT, "_os_crash_fmt");

    const size_t size = _os_log_pack_size(strlen(output));
    uint8_t buf[size] __attribute__((aligned(alignof(struct os_log_pack_s))));
    os_log_pack_t pack = (os_log_pack_t)&buf;
    _os_log_pack_fill(pack, size, errno, nullptr, output);
    _os_crash_fmt(pack, size);
    abort();
    __builtin_unreachable();
//    free(output);
}

#define __os_crash_N(msg) __os_crash_simple(msg)
#define __os_crash_Y(...) __os_crash_fmt(__VA_ARGS__)

#define __API_UNAVAILABLE_PLATFORM_bridgeos bridgeos,unavailable

__API_UNAVAILABLE(bridgeos) void os_crash_function(const char *message) {
    printf("DEBUGGING ---- Got message %s", message);
}

typedef struct vproc_s {
    int32_t refcount;
    mach_port_t j_port;
} *vproc_t;

typedef void * vproc_err_t;
typedef void * vproc_transaction_t;
typedef void * vproc_standby_t;
typedef vproc_transaction_t (*vproc_transaction_begin_t)(vproc_t);
typedef void (*vproc_standby_end_t)(vproc_t, vproc_standby_t);

typedef long os_once_t;
typedef os_once_t os_alloc_token_t;
struct _os_alloc_once_s {
    os_alloc_token_t once;
    void *ptr;
};

struct _launch {
    void    *sendbuf;
    int    *sendfds;
    void    *recvbuf;
    int    *recvfds;
    size_t    sendlen;
    size_t    sendfdcnt;
    size_t    recvlen;
    size_t    recvfdcnt;
    int which;
    int cifd;
    int    fd;
};
typedef struct _launch *launch_t;

typedef void (*_vproc_transaction_callout)(void *);
struct launch_globals_s {
    // liblaunch.c
    pthread_once_t lc_once;
    pthread_mutex_t lc_mtx;
    launch_t l;
    launch_data_t async_resp;
    
    launch_t in_flight_msg_recv_client;
    
    int64_t s_am_embedded_god;
    
    // libvproc.c
    dispatch_queue_t _vproc_gone2zero_queue;
    _vproc_transaction_callout _vproc_gone2zero_callout;
    void *_vproc_gone2zero_ctx;
    
    dispatch_once_t _vproc_transaction_once;
    uint64_t _vproc_transaction_enabled;
    dispatch_queue_t _vproc_transaction_queue;
    int64_t _vproc_transaction_cnt;
};

//extern struct _os_alloc_once_s *_os_alloc_once_table;

enum {
    XPC_FLAG_NULL_BOOTSTRAP = 0x2,
    XPC_FLAG_0x4 = 0x4,
    XPC_FLAG_SERVICES_UNAVAILABLE = 0x8,
    // sets AppleInternal_dir_flag
    // i think it's a flag that specifies if "/AppleInternal" exists
    XPC_FLAG_APPLE_INTERNAL_DIR = 0x20,
    XPC_FLAG_0xD = 0xd,
    XPC_FLAG_0x10 = 0x10, // track activity?
};

typedef struct xpc_pipe* xpc_pipe_t;

#define APPLE_XPC_VALID_FLAGS 0x82

void _vproc_transaction_init_once(void *arg __unused){
    printf("_vproc_transaction_init_once() called\n");
}
void _vproc_transactions_enable_internal(void *arg __unused){
    printf("_vproc_transactions_enable_internal() called\n");
}

void decode(int flags);

int main(int argc, const char * argv[]) {
    struct os_log_pack_s pack = {0};
//    __os_crash_fmt("puredarwin CRASHED with args, %s, %s, %s !!", "arg1", "arg2", "arg3");
    
//    struct vproc_s proc_s{0};
//    auto vproc_transaction_begin = (vproc_transaction_begin_t) dlsym(RTLD_DEFAULT, "vproc_transaction_begin");
//    auto transaction = vproc_transaction_begin(&proc_s);
    struct _os_alloc_once_s *ptr = (struct _os_alloc_once_s*) dlsym(RTLD_DEFAULT, "_os_alloc_once_table");
    
//    char xpc_globals[424];
    struct xpc_globals_s {
        bool _is_launchd_client; // 0
        bool _is_xpcproxy_client; // 1
        bool _is_launchctl_client; // 2
        bool xpc_is_managed; // 3
        bool _null_boostrap; // 4
        char data0[2];
        uint64_t xpc_flags; // 8
        mach_port_t bootstrap_port; // 16
        xpc_pipe_t bootstrap_pipe; // 24
        xpc_endpoint_t bootstrap_endpoint; // 32
        bool appleInternal_dir_flag_isset; // 40
        bool debug_libxpc; // 41
        bool _pre_exec_set; // 42
        bool exit_to_corpse;
        char data000[43];
        bool _is_libxpc_unit_testing_isset; // 88
        char data0000[16];
        os_unfair_lock lock_1; // 104
        char data00000[1];
        xpc_object_t array; // 112
        char data1[32];
        bool xpc_is_initialized; // 152
        char data11[67];
        uint32_t transaction_enabled; // 232
        uint32_t some_sort_of_exit_flag; // 234
        int32_t transaction_count; // 236
        os_unfair_lock lock; // 240
        bool xpc_flag_0xd_isset; // 244
        char data2[99];
        dispatch_once_t activity_once; // 336
        dispatch_queue_t transaction_queue; // 344
        char data3[70];
    } xpc_globals = {0};
    
//    printf("%d \n", EXC_MASK_CRASH | EXC_MASK_GUARD | EXC_MASK_RESOURCE);
//    printf("%d \n", EXC_MASK_RESOURCE | EXC_MASK_GUARD | EXC_MASK_CORPSE_NOTIFY);
//    printf("%d \n", sizeof(xpc_globals));
//    assert(424 == sizeof(xpc_globals));
//    printf("%d \n", offsetof(struct xpc_globals_s, transaction_queue));
//
//
//    printf("\n\n");
//    decode(7168);
//    printf("\n");
//    decode(14336);
//
//    printf("0x%x\n", 14336 & ~(EXC_MASK_RESOURCE | EXC_MASK_GUARD | EXC_MASK_CORPSE_NOTIFY) );
    
//    struct spawn_via_launchd_attr {
//        uint64_t spawn_flags;
//        const char *spawn_path;
//        const char *spawn_chdir;
//        const char * const * spawn_env;
//        const mode_t *spawn_umask;
//        mach_port_t *spawn_observer_port;
//        const cpu_type_t *spawn_binpref;
//        size_t spawn_binpref_cnt;
//        void * spawn_quarantine;
//        const char *spawn_seatbelt_profile;
//        const uint64_t *spawn_seatbelt_flags;
//    } attrs;
//
//
//    printf("%2lu : spawn_flags\n", offsetof(struct spawn_via_launchd_attr,spawn_flags));
//    printf("%2lu : spawn_path\n", offsetof(struct spawn_via_launchd_attr,spawn_path));
//    printf("%2lu : spawn_chdir\n", offsetof(struct spawn_via_launchd_attr,spawn_chdir));
//    printf("%2lu : spawn_env\n", offsetof(struct spawn_via_launchd_attr,spawn_env));
//    printf("%2lu : spawn_umask\n", offsetof(struct spawn_via_launchd_attr,spawn_umask));
//    printf("%2lu : spawn_observer_port\n", offsetof(struct spawn_via_launchd_attr,spawn_observer_port));
//    printf("%2lu : spawn_binpref\n", offsetof(struct spawn_via_launchd_attr,spawn_binpref));
//    printf("%2lu : spawn_binpref_cnt\n", offsetof(struct spawn_via_launchd_attr,spawn_binpref_cnt));
//    printf("%2lu : spawn_quarantine\n", offsetof(struct spawn_via_launchd_attr,spawn_quarantine));
//    printf("%2lu : spawn_seatbelt_profile\n", offsetof(struct spawn_via_launchd_attr,spawn_seatbelt_profile));
//    printf("%2lu : spawn_seatbelt_flags\n", offsetof(struct spawn_via_launchd_attr,spawn_seatbelt_flags));
//
//    for (int i = 0; i < 23; i++)
//        printf("%u \n", i*8);
    
//    struct xpc_spawnattr_packed {
//        uint32_t _data_0;       // 0
//        uint32_t _data_4;       // 4
//        uint64_t _data_8;       // 8
//        uint32_t _data_16;      // 16
//        uint32_t _data_20;      // 20
//        uint16_t _data_24;      // 24
//        uint8_t  _data_26;      // 26
//        uint8_t  _data_27;      // 27
//        uint32_t _data_28;      // 28
//        uint32_t _data_32;      // 32
//        uint16_t _data_36;      // 36
//        uint16_t _data_38;      // 36
//        uint64_t _data_40;      // 40
//        uint64_t _data_48;      // 48
//        uint64_t _data_56;      // 56
//        uint64_t _data_64;      // 64
//        uint32_t _data_72;      // 72
//        uint32_t _data_76;      // 76
//        uint64_t _data_80;      // 80
//        uint64_t _data_88;      // 88
//        uint32_t _data_96;      // 96
//        uint32_t _data_100;     // 100
//        uint64_t _data_104;     // 104
//        uint64_t _data_112;     // 112
//        uint64_t _data_120;     // 120
//        uint64_t _data_128;     // 128
//        uint64_t _data_136;     // 136
//        uint64_t _data_144;     // 144
//        uint64_t _data_152;     // 152
//        uint64_t _data_160;     // 160
//        uint32_t _data_168;     // 168
//        uint8_t  _data_172;     // 172
//        uint8_t  _data_173;     // 173
//        uint16_t _data_174;     // 174
//        char     _data_175;     // 175
//        char     _strings[];    // 176
//    } __attribute__((packed));

    
//    printf("%d \n", sizeof(struct xpc_spawnattr_packed));
    
//    dispatch_once_f(&xpc_globals.activity_once, NULL, _vproc_transaction_init_once);
//    dispatch_sync_f(xpc_globals.transaction_queue, NULL, _vproc_transactions_enable_internal);
    
//    for (int i = 0; i < 100; i++) {
//        printf("{ %d , %p }\n", ptr[i].once, ptr[i].ptr);
//    }
//    if ( getenv("XPC_FLAGS") )
//    {
//      *(_QWORD *)&v30 = -6148914691236517206LL;
//      init_port_set = xmmword_29430;
//      __snprintf_chk((char *)&init_port_set, 0x18uLL, 0, 0x18uLL, "0x%llx", v26);
//      setenv("XPC_FLAGS", (const char *)&init_port_set, 1);
//    }
    
//    uint64_t tn = 0x7FFFFFFF;
    
//    char* args[] = { "main", "-debug" };
//
//    typedef void (*slxfunc_t)(int,int,void*);
//    slxfunc_t slxfunc = (slxfunc_t) dlsym(RTLD_DEFAULT, "SLXServer");
//
//    slxfunc(0, 2, &args);
    
//    int mib[128] = {0};
//
//    *((uint64_t*)mib) = 283467841537;
//    int8_t data[2][32] = {
//        {
//                0x00, 0x00, 00, 00, 00, 00, 00, 00,
//        (int8_t)0xD2, 0x1B, 00, 00, 00, 00, 00, 00,
//                0x00, 0x00, 00, 00, 00, 00, 00, 00,
//                0x03, 0x00, 00, 00, 00, 00, 00, 00,
//        },
//        {
//                0x00, 0x00, 00, 00, 00, 00, 00, 00,
//        (int8_t)0xAC, 0x1C, 00, 00, 00, 00, 00, 00,
//                0x00, 0x00, 00, 00, 00, 00, 00, 00,
//                0x03, 0x00, 00, 00, 00, 00, 00, 00,
//        }
//    };
//
//    struct IOService;
//    typedef kern_return_t IOReturn;
//    typedef IOReturn (*IOMethod)(void * p1, void * p2, void * p3,
//                                 void * p4, void * p5, void * p6 );
//    struct IOExternalMethod {
//        IOService*         object;
//        IOMethod            func;
//        UInt32              flags;
//        UInt64              count0;
//        UInt64              count1;
//    };
//
//    IOExternalMethod methods[2];
//
//    memcpy(&methods[0], &data[0], sizeof(methods[0]));
//    memcpy(&methods[1], &data[1], sizeof(methods[1]));
//
//    printf("%p \n", nullptr);
    
//#include <libkern/c++/OSArray.h>
    
    return 0;
}

void decode(int flags) {
    if (flags & EXC_MASK_BAD_ACCESS)
        printf("EXC_MASK_BAD_ACCESS\n");
    if (flags & EXC_MASK_BAD_INSTRUCTION)
        printf("EXC_MASK_BAD_INSTRUCTION\n");
    if (flags & EXC_MASK_ARITHMETIC)
        printf("EXC_MASK_ARITHMETIC\n");
    if (flags & EXC_MASK_EMULATION)
        printf("EXC_MASK_EMULATION\n");
    if (flags & EXC_MASK_SOFTWARE)
        printf("EXC_MASK_SOFTWARE\n");
    if (flags & EXC_MASK_BREAKPOINT)
        printf("EXC_MASK_BREAKPOINT\n");
    if (flags & EXC_MASK_SYSCALL)
        printf("EXC_MASK_SYSCALL\n");
    if (flags & EXC_MASK_MACH_SYSCALL)
        printf("EXC_MASK_MACH_SYSCALL\n");
    if (flags & EXC_MASK_RPC_ALERT)
        printf("EXC_MASK_RPC_ALERT\n");
    if (flags & EXC_MASK_CRASH)
        printf("EXC_MASK_CRASH\n");
    if (flags & EXC_MASK_RESOURCE)
        printf("EXC_MASK_RESOURCE\n");
    if (flags & EXC_MASK_GUARD)
        printf("EXC_MASK_GUARD\n");
    if (flags & EXC_MASK_MACHINE)
        printf("EXC_MASK_MACHINE\n");
    if (flags & EXC_MASK_CORPSE_NOTIFY)
        printf("EXC_MASK_CORPSE_NOTIFY\n");
}


//  0  : spawn_flags
//  8  : spawn_path
//  16 : spawn_chdir
//  24 : spawn_env
//  32 : spawn_umask
//  40 : spawn_observer_port
//  48 : spawn_binpref
//  56 : spawn_binpref_cnt
//  64 : spawn_quarantine
//  72 : spawn_seatbelt_profile
//  80 : spawn_seatbelt_flags
