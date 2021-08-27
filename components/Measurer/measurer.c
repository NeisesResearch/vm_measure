/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "measurement_utils.h"
#include "payload_utils.h"

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
