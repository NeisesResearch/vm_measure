
#ifndef REGISTERED_DIGESTS_H
#define REGISTERED_DIGESTS_H

#include "measurement_utils.h"

typedef struct DigestRegistry
{
    // allow for 100 entries
    unsigned char registry[100][32];
    char names[100][56];
    unsigned int numEntries;
}DigestRegistry;
DigestRegistry digestRegistry = {{{0}}, {{0}}, 0};

void registerDigest(unsigned char* digest, char* name)
{
    for(int i=0; i<32; i++)
    {
        digestRegistry.registry[digestRegistry.numEntries][i] = digest[i];
    }
    for(int i=0; i<56; i++)
    {
        digestRegistry.names[digestRegistry.numEntries][i] = name[i];
    }
    digestRegistry.numEntries++;
}

// Register one digest
unsigned char goodDigest1[] = {
    0x3d, 0xf4, 0xf8, 0xea, 0x75, 0x43, 0x0e, 0x93, 
    0x9d, 0xda, 0x1b, 0xe5, 0x7c, 0x21, 0x6e, 0x1f, 
    0x0e, 0x7e, 0xbf, 0x08, 0x49, 0x9c, 0xc1, 0xf5, 
    0x2e, 0x42, 0x83, 0xef, 0xff, 0x54, 0x67, 0xdd
};

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

