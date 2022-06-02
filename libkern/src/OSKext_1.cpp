//
//  OSKext_1.cpp
//  libkern
//
//  Created by John Othwolo on 6/1/22.
//  Copyright © 2022 oth. All rights reserved.
//

#include <stdio.h>
#include <libkern/c++/OSKext.h>
#include <libkern/c++/OSLib.h>
#include <IOKit/IOLocks.h>
struct {
    pthread_mutex_t _lock;
    pthread_cond_t _cond;
} __sKextLoggingLock;

IOLock *sKextLoggingLock = (IOLock *)&__sKextLoggingLock;
struct kmod_info g_kernel_kmod_info;

extern "C" void
OSKextLog(
    OSKext         * aKext,
    OSKextLogSpec    msgLogSpec,
    const char     * format, ...)
{
    va_list argList;

    va_start(argList, format);
    OSKextVLog(aKext, msgLogSpec, format, argList);
    va_end(argList);
}

extern "C" void
OSKextVLog(
    OSKext         * aKext,
    OSKextLogSpec    msgLogSpec,
    const char     * format,
    va_list          srcArgList)
{
    extern int       disableConsoleOutput;

    bool             logForKernel       = false;
    bool             logForUser         = false;
    va_list          argList;
    char             stackBuffer[120];
    uint32_t         length            = 0;
    char           * allocBuffer       = NULL;     // must kfree
    OSNumber       * logSpecNum        = NULL;     // must release
    OSString       * logString         = NULL;     // must release
    char           * buffer            = stackBuffer;// do not free

    IOLockLock(sKextLoggingLock);

    /* Set the kext/global bit in the message spec if we have no
     * kext or if the kext requests logging.
     */
//    if (!aKext || aKext->flags.loggingEnabled) {
//        msgLogSpec = msgLogSpec | kOSKextLogKextOrGlobalMask;
//    }

//    logForKernel = logSpecMatch(msgLogSpec, sKernelLogFilter);
//    if (sUserSpaceLogSpecArray && sUserSpaceLogMessageArray) {
//        logForUser = logSpecMatch(msgLogSpec, sUserSpaceKextLogFilter);
//    }

//    if (!(logForKernel || logForUser)) {
//        goto finish;
//    }

    /* No goto from here until past va_end()!
     */
    va_copy(argList, srcArgList);
    length = vsnprintf(stackBuffer, sizeof(stackBuffer), format, argList);
    va_end(argList);

    if (length + 1 >= sizeof(stackBuffer)) {
        allocBuffer = (char *)kalloc_tag((length + 1) * sizeof(char), VM_KERN_MEMORY_OSKEXT);
        if (!allocBuffer) {
            goto finish;
        }

        /* No goto from here until past va_end()!
         */
        va_copy(argList, srcArgList);
        vsnprintf(allocBuffer, length + 1, format, argList);
        va_end(argList);

        buffer = allocBuffer;
    }

    /* If user space wants the log message, queue it up.
     */
//    if (logForUser && sUserSpaceLogSpecArray && sUserSpaceLogMessageArray) {
//        logSpecNum = OSNumber::withNumber(msgLogSpec, 8 * sizeof(msgLogSpec));
//        logString = OSString::withCString(buffer);
//        if (logSpecNum && logString) {
//            sUserSpaceLogSpecArray->setObject(logSpecNum);
//            sUserSpaceLogMessageArray->setObject(logString);
//        }
//    }

    /* Always log messages from the kernel according to the kernel's
     * log flags.
     */
//    if (logForKernel) {
//        /* If we are in console mode and have a custom log filter,
//         * colorize the log message.
//         */
//        if (!disableConsoleOutput && sBootArgLogFilterFound) {
//            const char * color = ""; // do not free
//            color = colorForFlags(msgLogSpec);
//            printf("%s%s%s\n", colorForFlags(msgLogSpec),
//                buffer, color[0] ? VTRESET : "");
//        } else {
//            printf("%s\n", buffer);
//        }
//    }

finish:
    IOLockUnlock(sKextLoggingLock);

    if (allocBuffer) {
        kfree(allocBuffer, (length + 1) * sizeof(char));
    }
    OSSafeReleaseNULL(logString);
    OSSafeReleaseNULL(logSpecNum);
    return;
}

void
OSKext::considerUnloads(Boolean rescheduleOnlyFlag)
{
}


OSKext*
OSKext::lookupKextWithIdentifier(const char * kextIdentifier)
{
    return nullptr;
}
OSKext*
OSKext::lookupKextWithIdentifier(OSString * kextIdentifier)
{
    return nullptr;
}

void
OSKext::reportOSMetaClassInstances(const char*   kextIdentifier,
                                   OSKextLogSpec msgLogSpec
){
}


//"os_panic(char*, ...)", referenced from:
//     OSCollection::haveUpdated() in liblibkern.a(OSCollection.o)
// "OSKext::lookupKextWithIdentifier(OSString*)", referenced from:
//     OSMetaClass::postModLoad(void*) in liblibkern.a(OSMetaClass.o)
// "OSKext::lookupKextWithIdentifier(char const*)", referenced from:
//     OSMetaClass::modHasInstance(char const*) in liblibkern.a(OSMetaClass.o)
// "OSKext::reportOSMetaClassInstances(char const*, unsigned int)", referenced from:
//     OSMetaClass::reportModInstances(char const*) in liblibkern.a(OSMetaClass.o)
//     _OSRuntimeFinalizeCPP in liblibkern.a(OSRuntime.o)
// "_g_kernel_kmod_info", referenced from:
//     _OSlibkernInit in liblibkern.a(OSRuntime.o)
//     _OSRuntimeInitializeCPP in liblibkern.a(OSRuntime.o)
//     _OSRuntimeFinalizeCPP in liblibkern.a(OSRuntime.o)
//     _OSRuntimeUnloadCPPForSegment in liblibkern.a(OSRuntime.o)
