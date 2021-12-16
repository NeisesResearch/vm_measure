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

        void GetModuleLayoutFromListHead(int physAddr)
        {
            int index = physAddr;
            index += 16; // skip list_head
            index += 256; // skip name
            index += 96; //skip mkobj
            index += 8; // skio modinfo_attrs
            index += 8; // skip version
            index += 8; // skip srcversion
            index += 8; // skip holders_dir
            index += 8; // skip syms
            index += 8; // skip crcs
            index += 4; // skip num_syms
            index += 40; // skip struct mutex
            index += 8; // skip kp
            index += 4; // num_kp
            index += 4; // num_gpl_syms
            index += 8; // gpl_syms
            index += 8; // gpl_crcs
            index += 1; //async_probe_requested
            index += 8; // gpl_future_syms
            index += 8; // gpl_future_crcs
            index += 4; // num_gpl_future_syms
            index += 4; // num_exentries
            index += 8; // extable
            index += 8; // (*init*(void)

            // okay now we're at core_layout, or at least should be
            // let's print out som elines, to check
            
            // a correction ?
            index += 3;

            // by guess, based on source code
            //printerate(index, 3);

            //by inspection
            printerate(physAddr + 47 * 8, 3);
        }

        void PrintEverythingUpToModuleLayoutFromListHead(int physAddr)
        {
            int size = 0;
            size += 16; // skip list_head
            size += 256; // skip name
            size += 96; //skip mkobj
            size += 8; // skio modinfo_attrs
            size += 8; // skip version
            size += 8; // skip srcversion
            size += 8; // skip holders_dir
            size += 8; // skip syms
            size += 8; // skip crcs
            size += 4; // skip num_syms
            size += 40; // skip struct mutex
            size += 8; // skip kp
            size += 4; // num_kp
            size += 4; // num_gpl_syms
            size += 8; // gpl_syms
            size += 8; // gpl_crcs
            size += 1; //async_probe_requested
            size += 8; // gpl_future_syms
            size += 8; // gpl_future_crcs
            size += 4; // num_gpl_future_syms
            size += 4; // num_exentries
            size += 8; // extable
            size += 8; // (*init*(void)

            // corection
            size += 3;
            // sizeof module_layout
            size += 6;

            for(int i=0; i < (size / 64) + 1; i++)
            {
                printerate(physAddr + i*64, 8);
            }

        }



        printf("Module 1 Layout: ");
        GetModuleLayoutFromListHead(module1index);
        printf("Module 2 Layout: ");
        GetModuleLayoutFromListHead(module2index);
        printf("Module 3 Layout: ");
        GetModuleLayoutFromListHead(module3index);

        /*
        printf("Module 1 'Full' Data:\n");
        PrintEverythingUpToModuleLayoutFromListHead(module1index);
        */
       
        


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

