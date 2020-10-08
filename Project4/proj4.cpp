#include <iostream>
using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#define DEFAULTCHUNKSIZE 1024
#define MAXCHUNKSIZE 8192

void startMmap(int fdIn)
{
    cout << "Starting mmap" << endl;
}

void startRead(int chunkSize, int fdIn)
{
    cout << "Starting read" << endl;
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
    if(fdIn = open(file.c_str(), O_RDONLY) < 0) {
        cerr << "Failed to open " << file << endl;
        exit(1);
    }
    if(chunkSize == -1) {
        startMmap(fdIn);
    }
    else {
        startRead(chunkSize, fdIn);
    }
}