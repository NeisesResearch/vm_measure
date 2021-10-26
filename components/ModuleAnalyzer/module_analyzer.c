/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "digest_registry.h"

HashedModuleMeasurement** My_Global_Array_Of_Golden_Digests;

void do_the_thing(bool isPerryProvisioning)
{
    HashedModuleMeasurement** measurements = malloc(46 * sizeof(HashedModuleMeasurement*));

    int numMeasurements = 0;

    // wait for a payload signal
    payload_loaded_wait();
    printf("grab payload\n");

    HashedModuleMeasurement* thisMsmt = NULL;
    char* namePtr = NULL;
    uint8_t* rodataDigestPtr = NULL;
    enum MeasurementHeader currentHeader;
    bool gotSignOff = false;
    for(int i=0; i<46; i++)
    {
        currentHeader = checkHeader(data, i);
        switch(currentHeader)
        {
            case Signoff:
                fprintf(stderr, "Got Sign-Off. Breaking...\n");
                gotSignOff = true;
                goto good_return;
            case MeasurementModulePayload:
                fprintf(stderr, "Got Measurement-Module Measurement\n");
                numMeasurements++;
                goto good_return;
            case ModulePayload:
                fprintf(stderr, "Got Module Measurement\n", i);
                thisMsmt = malloc(1 * sizeof(HashedModuleMeasurement));
                namePtr = malloc(56 * sizeof(char));
                rodataDigestPtr = malloc(32 * sizeof(uint8_t));
                memcpy(namePtr,         data + 88*i,      56);
                memcpy(rodataDigestPtr, data + 88*i + 56, 56);
                thisMsmt->name = namePtr;
                thisMsmt->rodataDigest = rodataDigestPtr;
                measurements[numMeasurements] = thisMsmt;
                numMeasurements++;
                goto good_return;
            default:
                fprintf(stderr, "Went to default. Shouldn't be here...\n");
                goto good_return;
        }
        if(gotSignOff)
        {
            goto good_return;
        }
    }

    // measurements collected
    // either get more or analyze them all

    if(!gotSignOff)
    {
        fprintf(stderr, "got a payload\n");
        payload_get_emit_underlying();
    }
    else
    {
        if(isPerryProvisioning)
        {
            // TODO is this all we need here?
            for(int i=0; i<46; i++)
            {
                My_Global_Array_Of_Golden_Digests[i] = measurements[i];
            }
            goto good_return;
        }
        else
        {
            printf("Printing all measurements\n");
            fprintf(stderr,"============================================================\n");
            for(int i=0; i<numMeasurements; i++)
            {
                printMeasurement(measurements[i]);
            }
            fprintf(stderr,"============================================================\n");

            printf("Verify hash digests\n");
            fprintf(stderr,"============================================================\n");
            for(int i=0; i<numMeasurements; i++)
            {
                // TODO change checkDigest to reference global array of golden
                // digests
                checkDigest(measurements[i]);
                printf("\n");
            }
            fprintf(stderr,"============================================================\n");

            memset(data, '0', 4096);
            strcpy(data, "looks good");

            report_ready_emit_underlying();
        }
    }

good_return:
       // freach measurement, free measurement
       free(measurements);
       return;
}

int run(void)
{
    HashedModuleMeasurement** My_Global_Array_Of_Golden_Digests = malloc(46 * sizeof(HashedModuleMeasurement*));
    memset(data, '0', 4096);
    printf("Modules Dataport reset!\n");
    //strcpy(dest, "This is a crossvm dataport test string");

    // register any digests
    //registerDigest(goodDigest1, "Good Digest #1");

    // report to msmt mngr we're ready to go
    analyzer_ready_emit_underlying();

    // prepare for a full payload of measurements
    do_the_thing(true);
    while (1) {
        do_the_thing(false);
    }



    fprintf(stderr,"Modules Analyzer Finished\n");
    return 0;
}
