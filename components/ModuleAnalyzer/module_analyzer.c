/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "digest_registry.h"

HashedModuleMeasurement** My_Global_Array_Of_Golden_Digests;

void PrintGoldenDigests(void)
{
    for(int i=0; i<46; i++)
    {
        if(strcmp(My_Global_Array_Of_Golden_Digests[i]->name, "dummy name")==0)
        {
            continue;
        }
        PrintMeasurement(My_Global_Array_Of_Golden_Digests[i]);
    }
}

bool CheckForDigestMatchInMySpecialGlobalArray(HashedModuleMeasurement* inputMsmt)
{
    for(int i=0; i<46; i++)
    {
        if(strcmp(inputMsmt->name, My_Global_Array_Of_Golden_Digests[i]->name) == 0)
        {
            bool isMatch = true;
            for(int j=0; j<32; j++)
            {
                if(inputMsmt->rodataDigest[j] != My_Global_Array_Of_Golden_Digests[i]->rodataDigest[j])
                {
                    isMatch = false;
                    break;
                }
            }
            if(isMatch)
            {
                return true;
            }
        }
        continue;
    }
    return false;
}

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
        if(isPerryProvisioning)
        {
            // TODO is this all we need here?
            InitGoldenDigests();
            printf("start perry provisioning...\n");
            for(int i=0; i<numMeasurements; i++)
            {
                printf("%d, ", i);
                HashedModuleMeasurement* temp = measurements[i];
                printf("%d, ", i);
                //My_Global_Array_Of_Golden_Digests[i] = temp;
                My_Global_Array_Of_Golden_Digests[i]->name = temp->name;
                My_Global_Array_Of_Golden_Digests[i]->rodataDigest = temp->rodataDigest;
            }
            printf("Perry Provisioning Completed. Observe the golden digests:\n");
            PrintGoldenDigests();
            goto good_return;
        }
        else
        {
            // TODO just print later during the verification stage
            /*
            printf("Printing all measurements\n");
            fprintf(stderr,"============================================================\n");
            for(int i=0; i<numMeasurements; i++)
            {
                PrintMeasurement(measurements[i]);
            }
            fprintf(stderr,"============================================================\n");
            */

            printf("Verify hash digests\n");
            fprintf(stderr,"============================================================\n");
            for(int i=0; i<numMeasurements; i++)
            {
                if(CheckForDigestMatchInMySpecialGlobalArray(measurements[i]))
                {
                    printf("We matched the following module:\n");
                }
                else
                {
                    printf("We failed to match the following module:\n");
                }

                PrintMeasurement(measurements[i]);

                // This is a check against the old digest registry. We'll
                // probably let this die soon.
                //checkDigest(measurements[i]);
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

void InitGoldenDigests(void)
{
    My_Global_Array_Of_Golden_Digests = malloc(46 * sizeof(HashedModuleMeasurement*));
    for(int i=0; i<46; i++)
    {
        HashedModuleMeasurement* thisDummyMsmt = malloc(sizeof(HashedModuleMeasurement));
        thisDummyMsmt->name = malloc(100);
        thisDummyMsmt->name = "dummy name";
        thisDummyMsmt->rodataDigest = malloc(32);
        thisDummyMsmt->rodataDigest = "00000000000000000000000000000000";
        My_Global_Array_Of_Golden_Digests[i] = thisDummyMsmt;
    }
}

int run(void)
{



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
