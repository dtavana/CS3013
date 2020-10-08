#include <iostream>
using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#define DEFAULTCHUNKSIZE 1024
#define MAXCHUNKSIZE 8192

int getPercentage(int num, int den)
{
    return ((float) num / den) * 100;
}

void startMmap(int fdIn)
{
    char* buffer;
    struct stat sb;

    if(fstat(fdIn, &sb) < 0) {
        cerr << "Could not retrieve file size" << endl;
        return;
    }
    if ((buffer = (char *) mmap (NULL, sb.st_size, PROT_READ, MAP_SHARED, fdIn, 0)) == (char*) -1) {
        cerr << "Could not mmap file" << endl;
        return;
    }

    int printable = 0, totalBytes = sb.st_size;
    for(int i = 0; i < sb.st_size; i++) {
        if(isprint(buffer[i]) || isspace(buffer[i])) {
            printable++;
        }
    }
    int percentage = getPercentage(printable, totalBytes);
    cout << printable << " printable characters out of " << totalBytes << " bytes, " << percentage << "% using mmap()" << endl;

    if(munmap(buffer, sb.st_size) < 0) {
        cerr << "Could not unmap memory" << endl;
        return;
    }
}

void startRead(int chunkSize, int fdIn)
{
    char buffer[chunkSize];
    int bytesRead, printable = 0, totalBytes = 0;
    while((bytesRead = read(fdIn, buffer, chunkSize)) > 0) {
        for(int i = 0; i < bytesRead; i++) {
            if(isprint(buffer[i]) || isspace(buffer[i])) {
                printable++;
            }
        }
        totalBytes += bytesRead;
    }
    int percentage = getPercentage(printable, totalBytes);
    cout << printable << " printable characters out of " << totalBytes << " bytes, " << percentage << "% using read()" << endl;
}

int main(int argc, char* argv[])
{
    int chunkSize = DEFAULTCHUNKSIZE;
    string file = argv[1];
    if(argc > 2) {
        if(strcmp(argv[2], "mmap") == 0) {
            // Indicate that we are using mmap
            chunkSize = -1;
        }
        else {
            chunkSize = atoi(argv[2]);
        }
        if(chunkSize > MAXCHUNKSIZE) {
            cout << "Chunk size exceeded the MAXCHUNKSIZE, defaulting to " << MAXCHUNKSIZE << " bytes" << endl;
            chunkSize = MAXCHUNKSIZE;
        }
    }

    int fdIn;
    if((fdIn = open(file.c_str(), O_RDONLY)) < 0) {
        cerr << "Failed to open " << file << endl;
        exit(1);
    }
    if(chunkSize == -1) {
        startMmap(fdIn);
    }
    else {
        startRead(chunkSize, fdIn);
    }
    close(fdIn);
}