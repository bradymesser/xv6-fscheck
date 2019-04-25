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
 * test hard links
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    struct dirent dir;
    struct dinode dinode;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode) * 24, SEEK_SET);
    read(fsfd, &dinode, sizeof(struct dinode));
    
    lseek(fsfd, BSIZE * dinode.addrs[0] + sizeof(struct dirent) * 2, SEEK_SET);
    read(fsfd, &dir, sizeof(struct dirent));
    strcpy(dir.name, "spooky");
    dir.inum = 26;

    lseek(fsfd, BSIZE * dinode.addrs[0] + sizeof(struct dirent) * 2, SEEK_SET);
    write(fsfd, &dir, sizeof(struct dirent));



    close(fsfd);
    return 0;
}
