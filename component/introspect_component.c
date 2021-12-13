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
        char bingo = ((char*)memdev)[0xFB61E0];
        for(int i=0; i<32; i++)
        {
            printf("%02X", ((char*)memdev)[0xFB61E0+i]);
        }
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
        for(int i=0; i<32; i++)
        {
            //printf("%02X", ((char*)memdev)[0x38C71288-0x40000000+i]);
        }
        printf("Okay, thanks.\n");

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

