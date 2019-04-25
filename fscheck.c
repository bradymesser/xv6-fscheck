#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "stat.h"
#include "fs.h"
int ninodes = 0;

//returns 1 if the .. directory refers to the parent, and that parent refers to the directory .. is in
//0 if the child node or parent node do not point to the correct locations
int parentChildAlign(struct dinode * inodeTable, FILE * fs, int inode, int childNode) {
  unsigned char blockBuffer[BSIZE];
  struct dirent * dir;
  struct dirent * temp;

  // printf("inode: %d\nchildNode: %d\n",inode,childNode);
  for (int k = 0; k < NDIRECT + 1; k++) {
    if (inodeTable[inode * IPB].addrs[k] != 0) {
      //printf("DIRECTORY #%d\n", j);
      fseek(fs, BSIZE * inodeTable[inode * IPB].addrs[k], SEEK_SET);
      fread(blockBuffer, 1, BSIZE, fs);
      for (dir = (struct dirent*)blockBuffer; dir < (struct dirent*) blockBuffer + BSIZE; dir++) {
        temp = dir + 1;
        if (dir->inum == childNode) {
          return 1;
        }
        if (temp->inum > ninodes ) {
          printf("FAIL\n");
          return 0;
        }
      }
    }
  }
  return 0;
}

//found online, I didn't know how to convert the bitmap to 1's and 0's
const unsigned char masks[8] = {0b00000001, 0b00000010, 0b00000100, 0b00001000,
    0b00010000, 0b00100000, 0b01000000, 0b10000000};

unsigned char bitshift(unsigned char* bitmap, uint index) {
    uint location = index >> 3;
    uint offset = index & 7;
    return (bitmap[location] & masks[offset]) >> offset;
}

// struct superblock {
//   uint size;         // Size of file system image (blocks)
//   uint nblocks;      // Number of data blocks
//   uint ninodes;      // Number of inodes.
//   uint nlog;         // Number of log blocks
//   uint logstart;     // Block number of first log block
//   uint inodestart;   // Block number of first inode block
//   uint bmapstart;    // Block number of first free map block
// };

// struct dinode {
//   short type;           // File type
//   short major;          // Major device number (T_DEV only)
//   short minor;          // Minor device number (T_DEV only)
//   short nlink;          // Number of links to inode in file system
//   uint size;            // Size of file (bytes)
//   uint addrs[NDIRECT+1];   // Data block addresses
// };
//disk layout
// [ boot block | super block | log | inode blocks | free bit map | data blocks]
int main (int argc, char* argv[]) {
  struct superblock superBlock;
  unsigned char blockBuffer[BSIZE];
  int inodes_blocks;

  if (argc != 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
    exit(1);
  }
  FILE* fs = fopen(argv[1], "r");
  if (!fs) {
    fprintf(stderr, "image not found\n");
    exit(1);
  }

  //the file system has a boot block at the beginning, skip this block
  fseek(fs, BSIZE, SEEK_CUR);
  //read the super block
  fread(blockBuffer, 1, BSIZE, fs);
  //store the super block in a superblock struct
  memcpy(&superBlock, blockBuffer, sizeof(struct superblock));
  ninodes = superBlock.ninodes;
  //calculate size of inode table
  inodes_blocks = superBlock.ninodes / IPB + 1;
  struct dinode inodeTable[inodes_blocks * IPB];
  //for debugging
  // printf("%u\n%u\n%u\n",superBlock.size,superBlock.nblocks,superBlock.ninodes);
   //printf("%u\n%u\n%u\n%u\n",superBlock.nlog,superBlock.logstart,superBlock.inodestart,superBlock.bmapstart);
  fseek(fs, BSIZE * superBlock.inodestart, SEEK_SET);  //skip to inode blocks
  for (int i = 0; i < inodes_blocks; i++) {
    fread(blockBuffer, 1, sizeof(struct dinode), fs);
    memcpy(&inodeTable[i * IPB], blockBuffer, BSIZE);
    //printf("%d: %d %d %d\n",i,inodeTable[i * IPB].type, inodeTable[i * IPB].nlink, inodeTable[i * IPB].size);

    //Check if every inode is either unallocated or a valid type (error 1)
    if (inodeTable[i * IPB].type != T_FILE && inodeTable[i * IPB].type != T_DIR && inodeTable[i * IPB].type != T_DEV && inodeTable[i * IPB].type != 0) {
      fprintf(stderr,"ERROR: bad inode.\n");
      exit(1);
    }
    if (inodeTable[i * IPB].type == 0 && inodeTable[i * IPB].size != 0) {
      fprintf(stderr,"ERROR: bad inode.\n");
      exit(1);
    }

    //check the address stored in each inode (error 2)
    if (inodeTable[i * IPB].type != 0) {
      for (int j = 0; j < NDIRECT + 1; j++) {
        //printf("%u ", inodeTable[i * IPB].addrs[j]);
        if (inodeTable[i * IPB].addrs[j] != 0) {
          //the range of addresses are bmapstart+1 to superBlock.size
          if (inodeTable[i * IPB].addrs[j] <= superBlock.bmapstart || inodeTable[i * IPB].addrs[j] > superBlock.size) {
            fprintf(stderr, "ERROR: bad address in inode.\n");
            exit(1);
          }
        }
      }
      // printf("\n");
    }
  }

  int bmapBlocks = superBlock.size / BPB + 1;
  int bmap[superBlock.nblocks];
  fseek(fs, BSIZE * superBlock.bmapstart, SEEK_SET);  //skip to bitmap block
  for (int i = 0; i < bmapBlocks; i++) {
    //read in bitmap and store it in bmap
    fread(blockBuffer, 1, BSIZE, fs);
    memcpy(bmap, blockBuffer, BSIZE);
  }

  //check that root directory exists and is node 1 (error 3)
  if (inodeTable[1 * IPB].type != T_DIR || inodeTable[1 * IPB].size == 0) {
    fprintf(stderr, "ERROR: root directory does not exist.\n");
    exit(1);
  }

  struct dirent * dir;
  struct dirent * temp;
  int childNode = 0;
  int f = 0;

  //check that each directory contains . and .. (error 4)
  for (int i = 0; i < inodes_blocks; i++) {
    if (inodeTable[i * IPB].type == T_DIR) {
      //printf("**FOUND DIRECTORY AT INODE %d**",i);
      //loop through directory addresses and check each directory
      for (int j = 0; j < NDIRECT + 1; j++) {
        if (inodeTable[i * IPB].addrs[j] != 0) {
          //printf("DIRECTORY #%d\n", j);
          childNode = i;
          fseek(fs, BSIZE * inodeTable[i * IPB].addrs[j], SEEK_SET);
          fread(blockBuffer, 1, BSIZE, fs);
          for (dir = (struct dirent*)blockBuffer; dir < (struct dirent*) blockBuffer + BSIZE; dir++) {
            if (dir->inum > superBlock.ninodes) {
              break;
            }
            if (dir->inum != 0) {
              //printf("%s: %d\n", dir->name, dir->inum);
              temp = dir+1;
              //printf("%s: %d\n", temp->name, temp->inum);
              if (!strcmp(dir->name,".") && !strcmp(temp->name,"..")) {
                f = 1;
                //check if inodeTable[temp->inum * IPB] contains a reference to temp
                //error 5
                if (!parentChildAlign(inodeTable, fs, temp->inum, childNode)) {
                  fprintf(stderr,"ERROR: parent directory mismatch.\n");
                  exit(1);
                }

                break;
              }
              //(error 4)
              if (dir == (struct dirent*) blockBuffer + BSIZE) {
                fprintf(stderr,"ERROR: directory not properly formatted.\n");
                exit(1);
              }
            }
          }
          if (!f) {
            fprintf(stderr,"ERROR: directory not properly formatted.\n");
            exit(1);
          }
          f = 0;
        }
      }
    }
  }

//todo this is segfaulting
  //error 6, bitmap marks each address in use as in use
  // for (int i = 0; i < inodes_blocks; i++) {
  //   for (int j = 0; j < NDIRECT + 1; j++) {
  //     if (inodeTable[i * IPB].addrs[j] != 0) {
  //       //printf("addr: %d %u %d\n", inodeTable[i * IPB].addrs[j], get_bit(bmap, inodeTable[i * IPB].addrs[j]), bmap[inodeTable[i * IPB].addrs[j]]);
  //       if (!bitshift(bmap, inodeTable[i * IPB].addrs[j] - (superBlock.bmapstart+ bmapBlocks))) {
  //         fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
  //       }
  //     }
  //   }
  // }

  // int k;
  // //TODO
  // //error 7 each address the bitmap marks as in use is actually in use
  // for (int i = 0; i < bmapBlocks * BSIZE; i++) {
  //   k = i + bmapBlocks + superBlock.bmapstart;
  // //  printf("%d\n",k);
  // }
  int address;
  int count = 0;
  int found = 0;
  //error 8 address only used once, error 9 if marked used must be found in directory
  for (int i = 0; i < inodes_blocks; i++) {
    if (inodeTable[i * IPB].type > 0 && inodeTable[i * IPB].type < 4) {
      for (int j = 0; j < NDIRECT + 1; j++) {
        if (inodeTable[i * IPB].addrs[j] != 0 && inodeTable[i * IPB].addrs[j] < superBlock.nblocks) {
          count = 0;
          address = inodeTable[i * IPB].addrs[j];
          for (int k = 0; k < inodes_blocks; k++) {
            if (inodeTable[k * IPB].type > 0  && inodeTable[i * IPB].type < 4) {
              for (int l = 0; l < NDIRECT + 1; l++) {
                if (inodeTable[k * IPB].addrs[l] == address) {
                  count++;
                  if (count > 1) {
                    fprintf(stderr,"ERROR: address used more than once.\n");
                    exit(1);
                  }
                }
              }
            }
          }
          //check each directory for i (error 9)
          //for each in use inode, it is referenced in at least 1 directory
          found = 0;
          for (int k = 0; k < inodes_blocks; k++) {
            if (inodeTable[k * IPB].type == T_DIR) {
              for (int l = 0; l < NDIRECT + 1; l++) {
                if (inodeTable[k * IPB].addrs[l] != 0) {
                  fseek(fs, BSIZE * inodeTable[k * IPB].addrs[l], SEEK_SET);
                  fread(blockBuffer, 1, BSIZE, fs);
                  for (dir = (struct dirent*)blockBuffer; dir < (struct dirent*) blockBuffer + BSIZE; dir++) {
                    //error 10
                    if (inodeTable[dir->inum * IPB].type == 0) {
                      fprintf(stderr,"ERROR: inode referred to in directory but marked free.\n");
                      exit(1);
                    }
                    //if the structure belongs to the inode specified by i,
                    //then denote that the inode is marked in use and found in a directory
                    if (dir->inum == i) {
                      //printf("FOUND %d\n", i);
                      found = 1;
                      break;
                    }
                  }
                }
              }
            }
          }
          //error 9
          if (!found) {
            fprintf(stderr,"ERROR: inode marked use but not found in a directory.\n");
            exit(1);
          }
        }
      }
    }
  }

  int ref;
  count = 0;
  int count2 = 0;
  for (int i = 0; i < inodes_blocks; i++) {
    if (inodeTable[i * IPB].type == T_FILE) {
      count = 0;
      //check if the files reference count matches how many times it is referenced in a directory
      ref = inodeTable[i * IPB].nlink;
      for (int j = 0; j < inodes_blocks; j++) {
        if (inodeTable[j * IPB].type == T_DIR) {
          //loop through the directory counting references to inodeTable[i * IPB]
          for (int k = 0; k < NDIRECT + 1; k++) {
            if (inodeTable[j * IPB].addrs[k] != 0) {
              fseek(fs, BSIZE * inodeTable[j * IPB].addrs[k], SEEK_SET);
              fread(blockBuffer, 1, BSIZE, fs);
              //printf("**%d**\n", (inodeTable[j*IPB].size));
              for (dir = (struct dirent*)blockBuffer; dir < (struct dirent*) blockBuffer + BSIZE; dir++) {
                temp = dir + 1;
                //printf("%s: %d\n", dir->name, dir->inum);
                //printf("searching for: %d found %d\ncount: %d", i, dir->inum, count);
                if (dir->inum > superBlock.ninodes) {
                  break;
                }
                if (dir->inum == i) {
                  count++;
                }
                if (temp->inum > superBlock.ninodes) {
                  break;
                }
                //printf("searching for: %d found: %d\nref count: %d cur count: %d\n", i, dir->inum, ref, count);
              }
            }
          }
        }
      }
      //error 10
      if (count != ref) {
        fprintf(stderr,"ERROR: bad reference count for file.\n");
        exit(1);
        //printf("DEBUG: inode %d\n", i);
      }
    }
    else if (inodeTable[i * IPB].type == T_DIR) {
      count2 = 0;
      //check if the directory reference count matches how many times it is referenced in a directory
      ref = inodeTable[i * IPB].nlink;
      for (int j = 0; j < inodes_blocks; j++) {
        if (inodeTable[j * IPB].type == T_DIR) {
          //loop through the directory counting references to inodeTable[i * IPB]
          for (int k = 0; k < NDIRECT + 1; k++) {
            if (inodeTable[j * IPB].addrs[k] != 0) {
              fseek(fs, BSIZE * inodeTable[j * IPB].addrs[k], SEEK_SET);
              fread(blockBuffer, 1, BSIZE, fs);
              //printf("**%d**\n", (inodeTable[j*IPB].size));
              for (dir = (struct dirent*)blockBuffer; dir < (struct dirent*) blockBuffer + BSIZE; dir++) {
                temp = dir + 1;
                //printf("%s: %d\n", dir->name, dir->inum);
                //printf("searching for: %d found %d\ncount: %d", i, dir->inum, count);
                if (dir->inum > superBlock.ninodes) {
                  break;
                }
                if (dir->inum == i) {
                  count2++;
                  break;
                }
                if (temp->inum > superBlock.ninodes) {
                  break;
                }
                //printf("searching for: %d found: %d\nref count: %d cur count: %d\n", i, dir->inum, ref, count);
              }
            }
          }
        }
      }
      //error 11
      if (count2 > 1) {
        fprintf(stderr,"ERROR: directory appears more than once in file system.\n");
        exit(1);
        //printf("DEBUG: inode %d\n", i);
      }
    }
  }
  fclose(fs);
  return 0;
}
