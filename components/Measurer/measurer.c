/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>
#include <camkes.h>

int run(void)
{
    memset(dest, '0', 4096);
    printf("Dataport reset!\n");

    strcpy(dest, "This is a crossvm dataport test string");

    // wait until the Linux kernel module is ready
    ready_wait();

    printf("\n\nMeasurement module ready!\n\n");

    // request a measurement
    done_emit_underlying();

    /*
    while (1) {
        printf("Got an event\n");
        done_emit_underlying();
    }
    */

    return 0;
}
