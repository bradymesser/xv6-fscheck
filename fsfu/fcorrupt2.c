#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "fs.h"

#define BSTART 58
#define NINODES 200
#define ISTART 32
#define IEND (ISTART + 26)
typedef char byte;

#define MAX_ADDR 16777216
/*
 * no extra links
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    struct dinode dinode;
    (void)argc;
    byte buf[BSIZE];

    fsfd = open(argv[1], O_RDWR, 0666);
    
    int lastinode;
    for (int i = 1; i < NINODES; i++)
    {
        lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * i, SEEK_SET);
        read(fsfd, &dinode, sizeof(struct dinode));
        if (!dinode.type)
        {
            lastinode = i - 1;
            break;
        }
    }

    printf("corrupting block 0 of inode %d\n", lastinode);
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * lastinode, SEEK_SET);
    read(fsfd, &dinode, sizeof(struct dinode));
    int addr = dinode.addrs[NDIRECT];
    addr <<= 8;
    addr >>= 8;

    lseek(fsfd, BSIZE * addr, SEEK_SET);
    read(fsfd, &addr, sizeof(int));
    addr <<= 8;
    addr >>= 8;
    

    for (int i = 0; i < NINODES/2; i++)
    {
        buf[i] += buf[NINODES/2 - i];
    }

    lseek(fsfd, BSIZE * addr, SEEK_SET);
    write(fsfd, buf, sizeof(BSIZE));



    close(fsfd);
    return 0;
}
