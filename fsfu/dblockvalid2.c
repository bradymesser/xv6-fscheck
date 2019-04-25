#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "fs.h"

#define NINODES 200
#define ISTART 32
#define IEND (ISTART + 26)

#define MAX_ADDR 16777216
/*
 * Program to explode the address of 1 data block in inode ---indirect---
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    struct dinode dinode;
    uint ndir;
    uint badaddr;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * 3, SEEK_SET);
    read(fsfd, &dinode, sizeof(dinode));
    ndir = dinode.addrs[NDIRECT];

     badaddr = MAX_ADDR + 1;
    
    lseek(fsfd, BSIZE * ndir, SEEK_SET);
    write(fsfd, &badaddr, sizeof(uint));
    close(fsfd);
    return 0;
}
