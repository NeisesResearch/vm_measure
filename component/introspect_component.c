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
        printf("Okay now for that physical address you computed...\n");
        printf("That is, the modules list_head...\n");
        char bingo = ((char*)memdev)[0xFB61E0];
        for(int i=0; i<16; i++)
        {
            if(i%8==0&&i!=0){printf("\n");}
            printf("%02X", ((char*)memdev)[0xFB61E0+i]);
        }
        printf("\n");
        uint64_t* list_head_ptr = (uint64_t*)(((char*)memdev)+0xFB61E0);

        printf("%016X\n", list_head_ptr[0]);
        printf("%016X\n", list_head_ptr[1]);

        printf("\nChar was %02X\n", bingo);

        printf("Here's some bytes:\n");
        printf("%02X", ((char*)memdev)[0xC71288+0]);
        printf("%02X", ((char*)memdev)[0xC71288+1]);
        printf("%02X", ((char*)memdev)[0xC71288+2]);
        printf("%02X", ((char*)memdev)[0xC71288+3]);
        printf("%02X", ((char*)memdev)[0xC71288+4]);
        printf("%02X", ((char*)memdev)[0xC71288+5]);
        printf("%02X", ((char*)memdev)[0xC71288+6]);

        
        printf("\nHere's some bytes:\n");
        for(int i=0; i<48; i++)
        {
            if(i%48==0){printf("\n");}
            printf("%02X", ((char*)memdev)[0x4113D000-0x40000000+0x7C2+i]);
        }
        printf("\nHere's some bytes:\n");
        for(int i=0; i<48; i++)
        {
            if(i%48==0){printf("\n");}
            printf("%02X", ((char*)memdev)[0x47FFF803-0x40000000+0x188+i]);
        }

        printf("\nOkay, thanks.\n");

        for(int i=0; i<0xFFF; i+=8)
        {
            if(
                   ((char*)memdev)[0x4113D000-0x40000000+i+0] > 0
                || ((char*)memdev)[0x4113D000-0x40000000+i+1] > 0
                || ((char*)memdev)[0x4113D000-0x40000000+i+2] > 0
                || ((char*)memdev)[0x4113D000-0x40000000+i+3] > 0
                || ((char*)memdev)[0x4113D000-0x40000000+i+4] > 0
                || ((char*)memdev)[0x4113D000-0x40000000+i+5] > 0
                || ((char*)memdev)[0x4113D000-0x40000000+i+6] > 0
                || ((char*)memdev)[0x4113D000-0x40000000+i+7] > 0
                )
            {
                printf("Found possible address at offset %X\n", i);
                //printf("%p\n", (void*)((char*)memdev)[0x4113D000-0x40000000+i]);
                for(int j=0; j<8; j++)
                {
                    printf("%02X", ((char*)memdev)[0x4113D000-0x40000000+i+j]);
                }
                printf("\n");
            }
        }

        for(int i=0; i<0xFFF; i+=8)
        {
            if(
                   ((char*)memdev)[0x47fff803-0x40000000+i+0] > 0
                || ((char*)memdev)[0x47fff803-0x40000000+i+1] > 0
                || ((char*)memdev)[0x47fff803-0x40000000+i+2] > 0
                || ((char*)memdev)[0x47fff803-0x40000000+i+3] > 0
                || ((char*)memdev)[0x47fff803-0x40000000+i+4] > 0
                || ((char*)memdev)[0x47fff803-0x40000000+i+5] > 0
                || ((char*)memdev)[0x47fff803-0x40000000+i+6] > 0
                || ((char*)memdev)[0x47fff803-0x40000000+i+7] > 0
                )
            {
                printf("Found possible address at offset %X\n", i);
                for(int j=0; j<8; j++)
                {
                    printf("%02X", ((char*)memdev)[0x47FFF803-0x40000000+i+j]);
                }
                printf("\n");
            }
        }


        for(int i=0; i<0x5FFFFFF; i++)
        {
            if(
                   ((char*)memdev)[0x4113D000-0x40000000+i+0] == 0x08
                && ((char*)memdev)[0x4113D000-0x40000000+i+1] == 0xA0
                && ((char*)memdev)[0x4113D000-0x40000000+i+2] == 0x7C
                )
            {
                // subtract 8 because we match the second of two addresses
                printf("Found a match at PA %X\n", 0x4113D000+i-8);
                for(int j=0; j<24; j++)
                {
                    if(j%8==0&&j>0){printf("\n");}
                    printf("%02X", ((char*)memdev)[0x4113D000-0x40000000-8+i+j]);
                }
                printf("\n");
            }
        }

        printf("search for physical page number in table\n");
        /*
        ** This search turns up nothing because (I think) the table is in 5-hex
        ** segments, meaning that to iterate over it correctly would be to jump
        ** 2.5 bytes every time, which obviously won't do.
        ** We really need to search for the 3hex string 455, which turns up
        ** over and over in our experiments.
        **
        **
        */
        for(int i=0; i<0xFFFFF; i+=1)
        {
            if(
                      ((char*)memdev)[0x4113D000-0x40000000+i+0] == 0x04
                   && ((char*)memdev)[0x4113D000-0x40000000+i+1] == 0x55
                )
            {
                printf("Found possible match at offset %X\n", i);
                //printf("%p\n", (void*)((char*)memdev)[0x4113D000-0x40000000+i]);
                for(int j=0; j<4; j++)
                {
                    printf("%02X", ((char*)memdev)[0x4113D000-0x40000000+i+j]);
                }
                printf("\n");
            }
        }
        printf("next\n");

        //char* TTable = ((char*)memdev)+0x4113D000-0x4000000;
        char* TTable = ((char*)memdev)+0x0113D000;
        uint64_t* TranslationTable = (uint64_t*)TTable;
        printf("What about these bytes? %X\n", TranslationTable[0x7C2]);

        printf("good bytes: %02X %02x %02X %02X\n", ((char*)memdev)[0x4113D000-0x40000000],((char*)memdev)[0x4113D000-0x40000000+1],((char*)memdev)[0x4113D000-0x40000000+2], ((char*)memdev)[0x4113D000-0x40000000+3]);
        printf("bytes: %X\n", TranslationTable[0]);
        printf("bytes: %X\n", TranslationTable[0]>>0  & 0xFFF);
        printf("bytes: %X\n", TranslationTable[0]>>4  & 0xFFF);
        printf("bytes: %X\n", TranslationTable[0]>>8  & 0xFFF);
        printf("bytes: %X\n", TranslationTable[0]>>12 & 0xFFF);
        printf("bytes: %X\n", TranslationTable[0]>>16 & 0xFFF);
        printf("bytes: %X\n", TranslationTable[0]>>20 & 0xFFF);

        for(int i=0; i<0xFFFFF; i++)
        {
            if(
                      (TranslationTable[i]>> 0)  & 0xFFF == 0x554
                   || (TranslationTable[i]>> 4)  & 0xFFF == 0x554
                   || (TranslationTable[i]>> 8)  & 0xFFF == 0x554
                   || (TranslationTable[i]>> 12) & 0xFFF == 0x554
                   || (TranslationTable[i]>> 16) & 0xFFF == 0x554
                   || (TranslationTable[i]>> 20) & 0xFFF == 0x554
                )
            {
                printf("Found possible match at offset %X\n", i);
                printf("bytes: %X\n", TranslationTable[i]);
                printf("\n");
            }
        }

        



        struct list_head* module_list_head = (struct list_head*)(((uint8_t*)memdev)[0xFB61E0]);

        struct list_head* iterator = module_list_head;

        /*
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

