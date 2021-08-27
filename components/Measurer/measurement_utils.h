
#ifndef MEASUREMENT_UTILS_H
#define MEASUREMENT_UTILS_H

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

#endif

