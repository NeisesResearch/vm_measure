/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>
#include <camkes.h>

#define RAM_BASE 0x40000000
#define N 10
uint32_t fib_buf[N];

struct list_head {
    struct list_head *next, *prev;
};

int run(void)
{
    while (1) {
        ready_wait();
        printf("introspect: Got an event\n");

        seL4_Word paddr = *(seL4_Word *)introspect_data;
        printf("paddr in component 0x%x\n", paddr);

        seL4_Word offset = paddr - RAM_BASE;

        printf("offset in component 0x%x\n", offset);

        memcpy(fib_buf, ((char *)memdev + offset), sizeof(uint32_t) * N);

        //print data from inside linux process
        for (int i = 0; i < 10; i++) {
            printf("camkes_fib[%d]@%p = %d, ", i, (fib_buf + i), fib_buf[i]);
        }
        printf("\n");

        printf("\nOkay now for that physical address you computed...\n");
        char bingo = ((char*)memdev)[0xFB61E0];
        printf("Char was %02X\n", bingo);
        printf("Here's some bytes:\n");

        void printerate(int physAddr, int numLongs)
        {
            for(int i=0; i<numLongs; i++)
            {
                for(int j=0; j<8; j++)
                {
                    printf("%02X", ((char*)memdev)[physAddr + 8*i + j]);
                }
                printf(" ");
            }
            printf("\n");
        }
        void printerateModule(int physAddr)
        {
            for(int j=0; j<8; j++)
            {
                printf("%02X", ((char*)memdev)[physAddr + 0 + j]);
            }
            for(int j=0; j<8; j++)
            {
                printf("%02X", ((char*)memdev)[physAddr + 8 + j]);
            }
            for(int i=0; i<256; i++)
            {
                printf("%C", ((char*)memdev)[physAddr + 16 + i]);
            }
            printf("\n");
        }

        void printifyMany(int physOffset)
        {
            int offset = physOffset;
            for(int i=0; offset < 0x8001000; i++)
            {
                printf("i is %d and offset is %016X\n", i, offset);
                printerateModule(offset);
                offset = physOffset + i * 0x1000000;
            }
            printf("\n");
        }

        printf("Here's the head of the linked list of modules:\n");
        printerate(0xFB61E0, 2);
        printf("Okay, thanks.\n");
        /*
        printifyMany(0xC71288);
        printerate(0xC71288, 6);
        printerate(0xC70A80, 6);
        printerate(0xD70280, 6);
        */

        bool isMatchModuleHead(int i)
        {
            if(
                ((char*)memdev)[i + 0] == 0xE0 &&
                ((char*)memdev)[i + 1] == 0x61 &&
                ((char*)memdev)[i + 2] == 0xFB &&
                ((char*)memdev)[i + 3] == 0x08 &&
                ((char*)memdev)[i + 4] == 0x00 &&
                ((char*)memdev)[i + 5] == 0x00 &&
                ((char*)memdev)[i + 6] == 0xFF &&
                ((char*)memdev)[i + 7] == 0xFF 
                )
            {
                return true;
            }
            return false;
        }
        bool isMatchModule1(int i)
        {
            if(
                ((char*)memdev)[i + 0] == 0x88 &&
                ((char*)memdev)[i + 1] == 0x21 &&
                ((char*)memdev)[i + 2] == 0x7C &&
                ((char*)memdev)[i + 3] == 0x00 &&
                ((char*)memdev)[i + 4] == 0x00 &&
                ((char*)memdev)[i + 5] == 0x00 &&
                ((char*)memdev)[i + 6] == 0xFF &&
                ((char*)memdev)[i + 7] == 0xFF 
                )
            {
                return true;
            }
            return false;
        }
        bool isMatchModule2(int i)
        {
            if(
                ((char*)memdev)[i + 0] == 0x08 &&
                ((char*)memdev)[i + 1] == 0xA0 &&
                ((char*)memdev)[i + 2] == 0x7C &&
                ((char*)memdev)[i + 3] == 0x00 &&
                ((char*)memdev)[i + 4] == 0x00 &&
                ((char*)memdev)[i + 5] == 0x00 &&
                ((char*)memdev)[i + 6] == 0xFF &&
                ((char*)memdev)[i + 7] == 0xFF 
                )
            {
                return true;
            }
            return false;
        }
        bool isMatchModule3(int i)
        {
            if(
                ((char*)memdev)[i + 0] == 0x08 &&
                ((char*)memdev)[i + 1] == 0x20 &&
                ((char*)memdev)[i + 2] == 0x7D &&
                ((char*)memdev)[i + 3] == 0x00 &&
                ((char*)memdev)[i + 4] == 0x00 &&
                ((char*)memdev)[i + 5] == 0x00 &&
                ((char*)memdev)[i + 6] == 0xFF &&
                ((char*)memdev)[i + 7] == 0xFF 
                )
            {
                return true;
            }
            return false;
        }


        int module1index = 0;
        int module2index = 0;
        int module3index = 0;
        for(int i=0; i<0x8001000; i++)
        {
            // match module 1
            if(isMatchModuleHead(i) && isMatchModule2(i+8))
            {
                printf("Found Module 1 at %X\n", i);
                printerateModule(i);
                module1index = i;
                continue;
            }
            if(isMatchModule1(i) && isMatchModule3(i+8))
            {
                printf("Found Module 2 at %X\n", i);
                printerateModule(i);
                module2index = i;
                continue;
            }
            if(isMatchModule2(i) && isMatchModuleHead(i+8))
            {
                printf("Found Module 3 at %X\n", i);
                printerateModule(i);
                module3index = i;
                continue;
            }
        }

       
        


        /*
        struct list_head* module_list_head = (struct list_head*)(((uint8_t*)memdev)[0xFB61E0]);
        struct list_head* iterator = module_list_head;
        for (int i=0; i<5; i++)
        {
            printf("\nhere's one:\n");
            // print out like 64 bytes or something
            char* bytePtr = (char*)iterator;
            for(int j=0; j<64; j++)
            {
                printf("%02X", bytePtr[j]);
            }
            iterator = iterator->next;
        }
        printf("\nthat's all\n");
        */

        done_emit();
    }

    return 0;
}

