#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "fs.h"

#define NINODES 200
#define ISTART 32
#define IEND (ISTART + 26)
#define BSTART 58
typedef char byte;

/*
 * breaks root 
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    struct dinode dinode;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode), SEEK_SET);
    read(fsfd, &dinode, sizeof(dinode));

    dinode.type = 0;
    
    lseek(fsfd, BSIZE * ISTART + sizeof(struct dinode), SEEK_SET);
    write(fsfd, &dinode, sizeof(dinode));

    close(fsfd);
    return 0;
}
