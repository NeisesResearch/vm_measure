
#ifndef MEASUREMENT_UTILS_H
#define MEASUREMENT_UTILS_H

#include <string.h>
#include <camkes.h>

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

#endif

