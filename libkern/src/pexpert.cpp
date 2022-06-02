//
//  pexpert.c
//  libkern
//
//  Created by John Othwolo on 6/1/22.
//  Copyright © 2022 oth. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define NUM     0
#define STR     1

enum {
    FALSE = false,
    TRUE = true,
};

typedef bool boolean_t;

static boolean_t
isargsep(char c)
{
    if (c == ' ' || c == '\0' || c == '\t') {
        return TRUE;
    } else {
        return FALSE;
    }
}

static boolean_t
israngesep(char c)
{
    if (isargsep(c) || c == '_' || c == ',') {
        return TRUE;
    } else {
        return FALSE;
    }
}

char *
PE_boot_args(
    void)
{
    return "";
}

static boolean_t
PE_parse_boot_argn_internal(
    const char *arg_string,
    void *      arg_ptr,
    int         max_len,
    boolean_t   force_string)
{
    return true;
}
//{
//    char *args;
//    char *cp, c;
//    uintptr_t i;
//    long long val = 0;
//    boolean_t arg_boolean;
//    boolean_t arg_found;
//
//    args = PE_boot_args();
//    if (*args == '\0') {
//        return FALSE;
//    }
//
//    if (max_len == -1) {
//        return FALSE;
//    }
//
//    arg_found = FALSE;
//
//    while (*args && isargsep(*args)) {
//        args++;
//    }
//
//    while (*args) {
//        if (*args == '-') {
//            arg_boolean = TRUE;
//        } else {
//            arg_boolean = FALSE;
//        }
//
//        cp = args;
//        while (!isargsep(*cp) && *cp != '=') {
//            cp++;
//        }
//        if (*cp != '=' && !arg_boolean) {
//            goto gotit;
//        }
//
//        c = *cp;
//
//        i = cp - args;
//        if (strncmp(args, arg_string, i) ||
//            (i != strlen(arg_string))) {
//            goto gotit;
//        }
//
//        if (arg_boolean) {
//            if (!force_string) {
//                if (max_len > 0) {
//                    argnumcpy(1, arg_ptr, max_len);/* max_len of 0 performs no copy at all*/
//                    arg_found = TRUE;
//                } else if (max_len == 0) {
//                    arg_found = TRUE;
//                }
//            }
//            break;
//        } else {
//            while (*cp && isargsep(*cp)) {
//                cp++;
//            }
//            if (*cp == '=' && c != '=') {
//                args = cp + 1;
//                goto gotit;
//            }
//            if ('_' == *arg_string) { /* Force a string copy if the argument name begins with an underscore */
//                if (max_len > 0) {
//                    int hacklen = 17 > max_len ? 17 : max_len;
//                    strncpy(++cp, (char *)arg_ptr, hacklen - 1);  /* Hack - terminate after 16 characters */
//                    arg_found = TRUE;
//                } else if (max_len == 0) {
//                    arg_found = TRUE;
//                }
//                break;
//            }
//            switch ((force_string && *cp == '=') ? STR : getval(cp, &val, isargsep, FALSE)) {
//            case NUM:
//                if (max_len > 0) {
//                    argnumcpy(val, arg_ptr, max_len);
//                    arg_found = TRUE;
//                } else if (max_len == 0) {
//                    arg_found = TRUE;
//                }
//                break;
//            case STR:
//                if (max_len > 0) {
//                    strncpy(++cp, (char *)arg_ptr, max_len - 1);        /*max_len of 0 performs no copy at all*/
//                    arg_found = TRUE;
//                } else if (max_len == 0) {
//                    arg_found = TRUE;
//                }
//#if !CONFIG_EMBEDDED
//                else if (max_len == -1) {         /* unreachable on embedded */
//                    strcpy(++cp, (char *)arg_ptr);
//                    arg_found = TRUE;
//                }
//#endif
//                break;
//            }
//            goto gotit;
//        }
//gotit:
//        /* Skip over current arg */
//        while (!isargsep(*args)) {
//            args++;
//        }
//
//        /* Skip leading white space (catch end of args) */
//        while (*args && isargsep(*args)) {
//            args++;
//        }
//    }
//
//    return arg_found;
//}

boolean_t
PE_parse_boot_argn(
    const char      *arg_string,
    void            *arg_ptr,
    int                     max_len)
{
    return PE_parse_boot_argn_internal(arg_string, arg_ptr, max_len, FALSE);
}

boolean_t
PE_parse_boot_arg_str(
    const char      *arg_string,
    char            *arg_ptr,
    int                     strlen)
{
    return PE_parse_boot_argn_internal(arg_string, arg_ptr, strlen, TRUE);
}
