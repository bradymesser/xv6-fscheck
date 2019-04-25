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
 * ref marked in inode must be referred to in directory
 * inode marked use but not found in directory
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    struct dinode dinode;
    struct dirent dir;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * 23, SEEK_SET);
    read(fsfd, &dinode, sizeof(struct dinode));
    
    lseek(fsfd, BSIZE * dinode.addrs[0] + sizeof(struct dirent) * 2, SEEK_SET);
    read(fsfd, &dir, sizeof(struct dirent));
    dir.inum = 0;
    
    lseek(fsfd, BSIZE * dinode.addrs[0] + sizeof(struct dirent) * 2, SEEK_SET);
    write(fsfd, &dir, sizeof(struct dirent));
    close(fsfd);
    return 0;
}
