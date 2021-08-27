
#ifndef REGISTERED_DIGESTS_H
#define REGISTERED_DIGESTS_H

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

#endif

