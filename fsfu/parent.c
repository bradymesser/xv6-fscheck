#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include "fs.h"

#define NINODES 200
#define ISTART 32
#define IEND (ISTART + 26)

/*
 * Breaks some dir's .. 
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    struct dinode dinode;
    uint dblock;
    struct dirent d;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * 25, SEEK_SET);
    read(fsfd, &dinode, sizeof(struct dinode));

    dblock = dinode.addrs[0];
    
    lseek(fsfd, BSIZE * dblock + sizeof(struct dirent), SEEK_SET);
    //lseek(fsfd, BSIZE * dblock, SEEK_SET);
    read(fsfd, &d, sizeof(struct dirent));
    d.inum = 1;
    lseek(fsfd, BSIZE * dblock + sizeof(struct dirent), SEEK_SET);
    write(fsfd, &d, sizeof(struct dirent));

    close(fsfd);
    return 0;
}
