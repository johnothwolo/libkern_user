//
//  kerntypes.h
//  testobj
//
//  Created by John Othwolo on 6/1/22.
//  Copyright © 2022 oth. All rights reserved.
//

#ifndef kerntypes_h
#define kerntypes_h

#include <sys/cdefs.h>

#define OSCONTAINER_ACCUMSIZE(s)
#define OSMETA_ACCUMSIZE(s)
#define OSIVAR_ACCUMSIZE(s)

#define kmem_alloc(map, data, size, tag)  __kmem_alloc((data),(size))
#define kmem_realloc(map, ptr, size, newptr, newsize, tag) \
                                          __kmem_realloc((void*)(ptr), (void*)(newptr), (size_t)(newsize))
#define kmem_free(map, data, size)        ::free((void*)(data))
int   __kmem_alloc(void *data, size_t size);
int   __kmem_realloc(void *old_data, void *new_data, size_t newsize);

#define kalloc_tag(size, tag)               malloc((size))
#define kalloc_tag_bt(size, tag)            malloc((size))
#define kallocp_tag_bt(sizep, tag)        __kallocp_container((size_t*)(sizep))
#define kalloc_container(size)              malloc((size))
#define kallocp_container(sizep)          __kallocp_container((size_t*)(sizep))
#define kfree(ptr, size)                  ::free((void*)(ptr));
void* __kallocp_container(size_t *size);

void  os_panic(char *fmt, ...) __printflike(1,2);

typedef uintptr_t                           vm_size_t;

#define IOLog                               printf

// from mach/vm_param.h
#include <os/overflow.h>
#include <mach/vm_param.h>

#define round_page_overflow(in, out)                                        \
(({                                                                         \
        bool __ovr = os_add_overflow(in, (__typeof__(*out))PAGE_MASK, out); \
        *out &= ~((__typeof__(*out))PAGE_MASK);                             \
        __ovr;                                                              \
}))

static inline uint64_t
round_page_64(uint64_t x)
{
    if (round_page_overflow(x, &x)) {
        os_panic("overflow detected");
    }
    return x;
}

static inline uint32_t
round_page_32(uint32_t x)
{
    if (round_page_overflow(x, &x)) {
        os_panic("overflow detected");
    }
    return x;
}

#include <mach/vm_types.h>

// device_*something*.h
#define __STRING(x)     #x
#define __DARWIN_SUF_DARWIN14               "_darwin14"
#define __DARWIN14_ALIAS(sym)               __asm("_" __STRING(sym) __DARWIN_SUF_DARWIN14)


#define IONew(type, number) \
( ((number) != 0 && ((vm_size_t) ((sizeof(type) * (number) / (number))) != sizeof(type)) /* overflow check 20847256 */ \
  ? 0 \
  : ((type*)malloc(sizeof(type) * (number)))) )

#define IODelete(ptr, type, number)         ::free((void*)(ptr))

typedef char lck_grp_t;

#endif /* kerntypes_h */
