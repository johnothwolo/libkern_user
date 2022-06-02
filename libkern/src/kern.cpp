//
//  kern.cpp
//  libkern
//
//  Created by John Othwolo on 6/1/22.
//  Copyright © 2022 oth. All rights reserved.
//

#include <libkern/c++/OSLib.h>
#include <malloc/malloc.h>

int   __kmem_alloc(void *data, size_t size)
{
    data = malloc(size);
    return 0;
}

int   __kmem_realloc(void *old_data, void* new_data, size_t newsize)
{
    new_data = realloc(old_data, newsize);
    return 0;
}


void* __kallocp_container(size_t *size)
{
    if (size) return malloc(*size);
    return nullptr;
}

__attribute__((visibility("default")))
void    os_panic(char *fmt, ...)
{
    va_list ap;
    char buffer[20480] = {0};
    
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);
    
    printf("panic!!!!!!!\n");
    printf("%s\n", buffer);
    abort();
}

lck_grp_t *IOLockGroup = nullptr;
