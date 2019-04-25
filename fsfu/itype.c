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

/*
 * Program to explode the type of 1 inode
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    struct dinode dinode;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * (NINODES - 1), SEEK_SET);
    read(fsfd, &dinode, sizeof(dinode));

    dinode.type = 17;
    
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * 12, SEEK_SET);
    write(fsfd, &dinode, sizeof(dinode));
    close(fsfd);
    return 0;
}
