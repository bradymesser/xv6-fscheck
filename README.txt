The file system cheker is implemented entirely in fscheck.c

First the disk image is opened and the superblock is read in.  Then the information in
the superblock is used to find the locations of other sectors such as the bitmap.
Then the sectors in the image are looped through and checked for the errors listed
in the write up.

I couldn't figure out how to read in the bitmap, I found a function online that
would convert the bitmap elements to 1's and 0's, but I don't use it since I couldn't figure out
how to read in the bitmap properly.
