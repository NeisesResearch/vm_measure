/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>
#include <camkes.h>
#include "registered_digests.h"

typedef struct HashedModuleMeasurement
{
    char* name;
    uint8_t* rodataDigest;
}HashedModuleMeasurement;

enum MeasurementHeader
{
    ModulePayload,
    MeasurementModulePayload,
    Signoff
};

enum MeasurementHeader checkHeader(uint8_t* msmt, int headerNum)
{
    enum MeasurementHeader result;
    if(strcmp(msmt + 88*headerNum, "DEADBEEF") == 0)
    {
        result = Signoff;
    }
    else if(strcmp(msmt + 88*headerNum, "measurement") == 0)
    {
        result = ModulePayload;
        //result = MeasurementModulePayload;
    }
    else
    {
        result = ModulePayload;
    }
    return result;
}

void printMeasurement(HashedModuleMeasurement* msmt)
{
    printf("Module Name: %s\n", msmt->name);
    printf("Module Rodata Digest: ");
    for(int i=0; i<32; i++)
    {
        printf("%02hhx", msmt->rodataDigest[i]);
    }
    printf("\n");
}

void checkDigest(HashedModuleMeasurement* msmt)
{
    printf("Module Name: %s\n", msmt->name);
    printf("Module Rodata Digest: ");
    for(int i=0; i<32; i++)
    {
        printf("%02hhx", msmt->rodataDigest[i]);
    }
    printf("\n");
    if(strcmp(msmt->name, "measurement")==0)
    {
        printf("This is a digest over an empty measurement.\n");
    }
    else
    {
        // try to match against all registered digests
        bool isDigestRegistered = false;
        for(int i=0; i<digestRegistry.numEntries; i++)
        {
            uint8_t numMatchingBytes = 0;
            for(int j=0; j<32; j++)
            {
                if(msmt->rodataDigest[j] == digestRegistry.registry[i][j])
                {
                    numMatchingBytes++;
                }
            }
            if(numMatchingBytes == 32)
            {
                printf("This rodata digest was previously registered as %s.\n", digestRegistry.names[i]);
                isDigestRegistered = true;
                break;
            }
        }
        if(!isDigestRegistered)
        {
            printf("This rodata digest is not recognized.\n");
        }
    }
}

void printPayload(uint8_t* payload)
{
    for(int i=0; i<4096; i++)
    {
        if(i%88==0)
        {
            printf("\n");
        }
        uint8_t thisByte = payload[i];
        if( (i%88) < 56 )
        {
            if( 0x20 < thisByte && thisByte < 0x7f)
            {
                printf("%c", thisByte);
            }
            else
            {
                printf("-");
            }
        }
        else
        {
            //printf("-");
            printf("%02hhx", thisByte);
        }
    }
    printf("\n");
}

int run(void)
{
    memset(dest, '0', 4096);
    printf("Dataport reset!\n");
    //strcpy(dest, "This is a crossvm dataport test string");

    // register any digests
    registerDigest(goodDigest1, "Good Digest #1");

    // wait until the Linux kernel module is ready
    ready_wait();

    printf("Ready Signal Received\n");


    // prepare for a full payload of measurements
    HashedModuleMeasurement** measurements = malloc(46 * sizeof(HashedModuleMeasurement*));
    int numMeasurements = 0;
    // request a measurement
    printf("Request Measurement\n");
    done_emit_underlying();
    while (1) {
        ready_wait();
        printf("Grab payload\n");
        uint8_t* payload = malloc(4096);
        memcpy((void*)payload, dest, 4096);

        //printPayload(payload);


        // Grab the first 8 measurements from the payload
        HashedModuleMeasurement* thisMsmt = NULL;
        char* namePtr = NULL;
        uint8_t* rodataDigestPtr = NULL;
        enum MeasurementHeader currentHeader;
        bool gotSignOff = false;
        for(int i=0; i<46; i++)
        {
            currentHeader = checkHeader(payload, i);
            switch(currentHeader)
            {
                case Signoff:
                    fprintf(stderr, "Got Sign-Off. Breaking...\n");
                    gotSignOff = true;
                    break;
                case MeasurementModulePayload:
                    fprintf(stderr, "Got Measurement-Module Measurement\n");
                    numMeasurements++;
                    break;
                case ModulePayload:
                    fprintf(stderr, "Got Module Measurement\n", i);
                    thisMsmt = malloc(1 * sizeof(HashedModuleMeasurement));
                    namePtr = malloc(56 * sizeof(char));
                    rodataDigestPtr = malloc(32 * sizeof(uint8_t));
                    memcpy(namePtr,         payload + 88*i,      56);
                    memcpy(rodataDigestPtr, payload + 88*i + 56, 56);
                    thisMsmt->name = namePtr;
                    thisMsmt->rodataDigest = rodataDigestPtr;
                    measurements[numMeasurements] = thisMsmt;
                    numMeasurements++;
                    break;
                default:
                    fprintf(stderr, "Went to default. Shouldn't be here...\n");
                    break;
            }
            if(gotSignOff)
            {
                break;
            }
        }

        free(payload);
        payload = NULL;

        if(gotSignOff)
        {
            break;
        }
        else
        {
            // signal that we're ready for a new payload
            done_emit_underlying();
        }
    }

    /*
    printf("Printing all measurements\n");
    fprintf(stderr,"============================================================\n");
    for(int i=0; i<numMeasurements; i++)
    {
        printMeasurement(measurements[i]);
    }
    fprintf(stderr,"============================================================\n");
    */

    printf("Verify hash digests\n");
    fprintf(stderr,"============================================================\n");
    for(int i=0; i<numMeasurements; i++)
    {
        checkDigest(measurements[i]);
        printf("\n");
    }
    fprintf(stderr,"============================================================\n");



    fprintf(stderr,"Finished\n");
    return 0;
}
