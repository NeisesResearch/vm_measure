
#ifndef PAYLOAD_UTILS_H
#define PAYLOAD_UTILS_H

void printPayload(uint8_t* payload)
{
    for(int i=0; i<4096; i++)
    {
        if(i%88==0)
        {
            printf("\n");
        }
        uint8_t thisByte = payload[i];
        if( (i%88) < 56 )
        {
            if( 0x20 < thisByte && thisByte < 0x7f)
            {
                printf("%c", thisByte);
            }
            else
            {
                printf("-");
            }
        }
        else
        {
            //printf("-");
            printf("%02hhx", thisByte);
        }
    }
    printf("\n");
}

#endif

