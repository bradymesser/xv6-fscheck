#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "fs.h"

#define NINODES 200
#define ISTART 32
#define IEND (ISTART + 26)
#define BSTART 58
typedef char byte;

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int i;
    int j;
    int fsfd;
    struct superblock sb;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);

    if (lseek(fsfd, BSIZE, SEEK_SET) != BSIZE) 
    { 
        printf("lseek error %s\n", strerror(errno)); 
        return 1; 
    }
    read(fsfd, &sb, sizeof(struct superblock));
    printf("superblock: size=%u, nblocks=%u, ninodes=%u, nlog=%u, logstart=%u, inodestart=%u, bmapstart=%u\n", 
            sb.size, sb.nblocks, sb.ninodes, sb.nlog, sb.logstart, sb.inodestart, sb.bmapstart);
    
    if (!sb.size)
    {
        printf("something went wrong\n");
        return 1;
    }

    lseek(fsfd, BSIZE * BSTART, SEEK_SET);
    for (i = 0; i < BSIZE; i++)
    {
        byte b;
        read(fsfd, &b, sizeof(byte));
        for (j = 0; j < 8; j++)
        {
            printf("%d", (b >> j) & 0b1);
        }
        if (i % 8 == 7)
        {
            printf("\n");
        }
        else
        {
            printf(" ");
        }
    }
    close(fsfd);
    return 0;
}
