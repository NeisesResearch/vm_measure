/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>
#include <camkes.h>

typedef struct ModuleMeasurement
{
    uint8_t* name;
    uint8_t* rodata;
    uint32_t rosize;
}ModuleMeasurement;

int run(void)
{
    memset(dest, '0', 4096);
    printf("Dataport reset!\n");

    strcpy(dest, "This is a crossvm dataport test string");

    // wait until the Linux kernel module is ready
    ready_wait();

    printf("\n\nMeasurement module ready!\n");


    // prepare for a measurement
    ModuleMeasurement** measurements = malloc(100 * sizeof(ModuleMeasurement*));
    int numMeasurements = 0;
    // request a measurement
    done_emit_underlying();
    while (1) {
        ready_wait();
        ModuleMeasurement* thisMeasurement = malloc(sizeof(ModuleMeasurement));

        // check for signoff
        char* thisMagicWord = malloc(9);
        for(int i=0; i<8; i++)
        {
            thisMagicWord[i] = ((char*)dest)[i];
        }
        thisMagicWord[8] = '\0';
        if(strcmp(thisMagicWord, "DEADBEEF") == 0)
        {
            printf("Got signoff\n");
            break;
        }

        printf("Receiving a module measurement\n");
        //grab the header data
        uint8_t msmtType = ((uint8_t*) dest)[0];
        uint8_t numPayloads = ((uint8_t*) dest)[1];
        uint8_t* name = malloc(256);
        for(int j=0; j<256; j++)
        {
            name[j] = ((uint8_t*) dest)[j+2];
        }
        thisMeasurement->name = name;
        thisMeasurement->rosize = 4096 * numPayloads;
        uint8_t* rodata = malloc(thisMeasurement->rosize);
        for(int j=0; j<numPayloads; j++)
        {
            done_emit_underlying();
            ready_wait();
            printf("Receiving a payload\n");
            for(int k=0; k<4096; k++)
            {
                rodata[4096*j + k] = ((uint8_t*)dest)[k];
            }
        }

        measurements[numMeasurements] = thisMeasurement;
        numMeasurements++;
        done_emit_underlying();
    }

    printf("Printing all measurements\n");
    for(int i=0; i<numMeasurements; i++)
    {
        printf("Module name: %s\n", measurements[i]->name);
        /*
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
        */
        /*
        header[30] = '\0';
        header[31] = '\0';
        printf("%s \n", header);
        */
    }

    printf("Finished\n");
    return 0;
}
