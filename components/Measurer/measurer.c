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

    printf("\n\nMeasurement module ready!\n");

    // request a measurement
    done_emit_underlying();

    // grab the entire measurement
    uint32_t** theseMeasurements = malloc(100 * sizeof(uint32_t*));
    int numMeasurements = 0;
    // this is MODULE_NAME_LEN as defined in module.h
    int moduleNameSize = 256;

    printf("Requesting measurements\n");
    while (1) {
        ready_wait();
        uint32_t* thisMeasurement = malloc(moduleNameSize);
        uint32_t* thisPtr = (uint32_t*)dest;
        for(int i=0; i<moduleNameSize/4; i++)
        {
            thisMeasurement[i] = thisPtr[i];
        }

        // check for signoff
        char* thisHeader = malloc(9);
        char* thisCharPtr = (char*)thisMeasurement;
        for(int i=0; i<8; i++)
        {
            thisHeader[i] = thisCharPtr[i];
        }
        thisHeader[8] = '\0';

        if(strcmp(thisHeader, "DEADBEEF") == 0)
        {
            printf("Got signoff\n");
            // that was the signoff
            break;
        }
        printf("Got a measurement\n");

        theseMeasurements[numMeasurements] = thisMeasurement;
        numMeasurements++;
        done_emit_underlying();
    }

    printf("Printing all measurements\n");
    for(int i=0; i<numMeasurements; i++)
    {
        char* headerPtr = (char*)theseMeasurements[i];
        char* header = malloc(32);
        printf("Measurement %d: ", i);
        for(int j=0; j<32; j++)
        {
            if(((uint8_t*)headerPtr)[j] == 0x98)
            {
                break;
            }
            printf("%c", headerPtr[j]);
            //header[j] = headerPtr[j];
        }
        printf("\n");
        /*
        header[30] = '\0';
        header[31] = '\0';
        printf("%s \n", header);
        */
    }

    printf("Finished\n");
    return 0;
}
