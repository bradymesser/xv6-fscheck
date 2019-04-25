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

/*
 * checks marks block in use but it is not in use
 * marks an unused block as used
 */

// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
int main(int argc, char* argv[])
{
    int fsfd;
    byte b;
    (void)argc;

    fsfd = open(argv[1], O_RDWR, 0666);


    if (lseek(fsfd, BSIZE * BSTART + 511, SEEK_SET) != BSIZE * BSTART + 511)
    {
        printf("lseek error %s\n", strerror(errno)); 
        return 1; 
    }
    read(fsfd, &b, sizeof(byte));
    b = 0b10000000;
    lseek(fsfd, BSIZE * BSTART + 511, SEEK_SET);
    write(fsfd, &b, sizeof(byte));

    close(fsfd);
    return 0;
}
