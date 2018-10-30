/*
 *   File name: imaxdiv.c
 *
 *  Created on: 2017年7月14日, 下午5:25:09
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <inttypes.h>

imaxdiv_t imaxdiv (intmax_t numer, intmax_t denomer)
{
    imaxdiv_t retval;

    retval.quot = numer / denomer;
    retval.rem = numer % denomer;
    if (numer >= 0 && retval.rem < 0) {
            retval.quot++;
            retval.rem -= denomer;
    }

    return (retval);
}
