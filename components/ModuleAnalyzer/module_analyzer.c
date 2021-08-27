/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "digest_registry.h"

int run(void)
{
    memset(data, '0', 4096);
    printf("Modules Dataport reset!\n");
    //strcpy(dest, "This is a crossvm dataport test string");

    // register any digests
    registerDigest(goodDigest1, "Good Digest #1");

    // report to msmt mngr we're ready to go
    analyzer_ready_emit_underlying();

    // prepare for a full payload of measurements
    HashedModuleMeasurement** measurements = malloc(46 * sizeof(HashedModuleMeasurement*));
    int numMeasurements = 0;
    while (1) {
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
                    memcpy(namePtr,         data + 88*i,      56);
                    memcpy(rodataDigestPtr, data + 88*i + 56, 56);
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

        // measurements collected
        // either get more or analyze them all

        if(!gotSignOff)
        {
            fprintf(stderr, "got a payload\n");
            payload_get_emit_underlying();
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
                checkDigest(measurements[i]);
                printf("\n");
            }
            fprintf(stderr,"============================================================\n");

            memset(data, '0', 4096);
            strcpy(data, "looks good");

            report_ready_emit_underlying();
            break;
        }
    }



    fprintf(stderr,"Modules Analyzer Finished\n");
    return 0;
}
