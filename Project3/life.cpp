#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#include "life.h"

int** evenGen;
int** oddGen;
int** cachedGen;
int totalGenerations;
int rows;
int columns;
msg** cachedMessages;

void* threadInit(void*);

void printGrid(int iteration) {
    int** grid = iteration % 2 == 0 ? evenGen : oddGen;
    cout << "Generation " << iteration << endl;
    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < columns; c++) {
            cout << grid[r][c] << " ";
        }
        cout << endl;
    }
}

void deepCopy(int** src, int** dest)
{
    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < columns; c++) {
            dest[r][c] = src[r][c];
        }
    }
}

void initSemaphores(int threadNumber)
{
    pSems = (sem_t**) malloc((threadNumber + 1) * sizeof(sem_t*));
    cSems = (sem_t**) malloc((threadNumber + 1) * sizeof(sem_t*));
    for(int i = 0; i <= threadNumber; i++) {
        pSems[i] = (sem_t*) malloc(sizeof(sem_t));
        cSems[i] = (sem_t*) malloc(sizeof(sem_t));
        sem_init(pSems[i], 0, 1);
        sem_init(cSems[i], 0, 0);
    }
}

void initThreads(int threadNumber)
{
    threads = (pthread_t**) malloc((threadNumber) * sizeof(pthread_t*));
    for(int i = 0; i < threadNumber; i++) {
        int* index = (int *) malloc(sizeof(*index));
        *index = i + 1;
        threads[i] = (pthread_t*) malloc(sizeof(pthread_t));
        pthread_create(threads[i], NULL, &threadInit, index);
    }
}

void initMailboxes(int threadNumber)
{
    mailboxes = (msg**) malloc((threadNumber + 1) * sizeof(msg*));
    cachedMessages = (msg**) malloc(threadNumber * sizeof(msg*));
}

void initBoards(string fileName)
{
    string currentLine;
    ifstream inFile;
    inFile.open(fileName);
    int computedRows = 0;
    int computedColumns = 0;
    // Get row and column count
    while(getline(inFile, currentLine)) {
        computedRows++;
        if(computedColumns == 0) {
            for(char c : currentLine) {
                if(c != ' ') {
                    computedColumns++;
                }
            }   
        }
    }
    inFile.close();

    if(computedRows > MAXGRID || computedColumns > MAXGRID || computedRows == 0 || computedColumns == 0) {
        cerr << "Invalid grid of size " << computedRows << "x" << computedColumns << endl;
        exit(1);
    }

    rows = computedRows;
    columns = computedColumns;
    evenGen = (int**) malloc(computedRows * sizeof(int*));
    oddGen = (int**) malloc(computedRows * sizeof(int*));
    cachedGen = (int**) malloc(computedRows * sizeof(int*));
    for(int i = 0; i < computedRows; i++) {
        evenGen[i] = (int*) malloc(computedColumns * sizeof(int));
        oddGen[i] = (int*) malloc(computedColumns * sizeof(int));
        cachedGen[i] = (int*) malloc(computedColumns * sizeof(int));
    }
    // Fill the grids with data
    inFile.open(fileName);
    int currentRow = 0;
    int currentColumn = 0;
    while(getline(inFile, currentLine)) {
        for(char c : currentLine) {
            if(c != ' ') {
                evenGen[currentRow][currentColumn] = c - '0'; // Calculation for char to int
                oddGen[currentRow][currentColumn] = c - '0'; // Calculation for char to int
                currentColumn++;
            }
        }
        currentRow++;
        currentColumn = 0;
    }
    inFile.close();
}

int getNeighbors(int** grid, int r, int c)
{
    int total = 0;
    // r - 1, c - 1
    if((r != 0 && c != 0) && grid[r - 1][c - 1] == 1) total++;
    // r - 1, c
    if((r != 0) && grid[r - 1][c] == 1) total++;
    // r - 1, c + 1
    if((r != 0 && c != columns - 1) && grid[r - 1][c + 1] == 1) total++;
    // r, c - 1
    if((c != 0) && grid[r][c - 1] == 1) total++;
    // r, c + 1
    if((c != columns - 1) && grid[r][c + 1] == 1) total++;
    // r + 1, c + 1
    if((r != rows - 1 && c != columns - 1) && grid[r + 1][c + 1] == 1) total++;
    // r + 1, c
    if((r != rows - 1) && grid[r + 1][c] == 1) total++;
    // r + 1, c - 1
    if((r != rows - 1 && c != 0) && grid[r + 1][c - 1] == 1) total++;

    return total;
}

void startSend(int threadNumber)
{
    
    int shared = rows / threadNumber; // Share workload equally among threads
    int current = 0;
    if(shared == 1) {
        int remainder = rows % threadNumber;
        for(int i = 0; i < threadNumber; i++) {
            cachedMessages[i] = (msg*) malloc(sizeof(msg));
            cachedMessages[i]->iSender = 0; // Sending from parent
            cachedMessages[i]->type = RANGE;
            cachedMessages[i]->value1 = current;
            if(i != threadNumber - 1) {
                if(remainder != 0) {
                    cachedMessages[i]->value2 = current + 1;
                    remainder--;
                }
                else {
                    cachedMessages[i]->value2 = current;
                }
                
            }
            else {
                cachedMessages[i]->value2 = rows - 1;
            }
            current = cachedMessages[i]->value2 + 1;
            SendMsg(i + 1, cachedMessages[i]);
        }
    }
    else {
        for(int i = 0; i < threadNumber; i++) {
            cachedMessages[i] = (msg*) malloc(sizeof(msg));
            cachedMessages[i]->iSender = 0; // Sending from parent
            cachedMessages[i]->type = RANGE;
            cachedMessages[i]->value1 = current;
            if(i != threadNumber - 1) {
                cachedMessages[i]->value2 = shared * (i + 1);
            }
            else {
                cachedMessages[i]->value2 = rows - 1;
            }
            current = shared * (i + 1) + 1;
            SendMsg(i + 1, cachedMessages[i]);
        }
    }
}

int** getGrid(int iteration) {
    //cout << "getGrid ";
    if(iteration % 2 == 0) {
        //cout << "evenGen" << endl;
        return evenGen;
    }
    //cout << "oddGen" << endl;
    return oddGen;
}

int** getOtherGrid(int iteration) {
    //cout << "getOtherGrid ";
    if(iteration % 2 != 0) {
        //cout << "evenGen" << endl;
        return evenGen;
    }
    //cout << "oddGen" << endl;
    return oddGen;
}

int startGame(int threadNumber, bool print, bool input)
{
    msg* message = (msg*) malloc(sizeof(msg));
    int currentIteration = 0;
    while(currentIteration < totalGenerations) {
        if(print) {
            printGrid(currentIteration);
            cout << endl;
        }
        if(input) {
            cout << endl << "------------------------" << endl;
            cout << "Press enter to continue" << endl;
            cout << "------------------------" << endl;
            getchar();
        }
        // Send go messages
        for(int i = 0; i < threadNumber; i++) {
            cachedMessages[i]->iSender = 0; // Sending from parent
            cachedMessages[i]->type = GO;
            SendMsg(i + 1, cachedMessages[i]);
        }
        // Get GENDONE messages back
        int deadCount = 0;
        int unchangedCount = 0;
        for(int i = 1; i <= threadNumber; i++) {
            // Check done conditions
            RecvMsg(0, message);
            if(message->type == ALLDEAD) {
                //cout << "ALLDEAD from Thread Index " << message->iSender << endl;
                deadCount++;
            }
            else if(message->type == GENNOTCHANGED) {
                //cout << "GENNOTCHANGED from Thread Index " << message->iSender << endl;
                unchangedCount++;
            }
        }
        if(deadCount == threadNumber || unchangedCount == threadNumber) {
            // Send STOP messages
            for(int i = 0; i < threadNumber; i++) {
                cachedMessages[i]->iSender = 0; // Sending from parent
                cachedMessages[i]->type = STOP;
                SendMsg(i + 1, cachedMessages[i]);
            }
            //cout << "Terminating early, conditions to terminate were met" << endl;
            break;
        }
        int** grid1 = getGrid(currentIteration);
        int** grid2 = getOtherGrid(currentIteration);
        deepCopy(grid1, cachedGen);
        deepCopy(grid2, grid1);
        currentIteration++;
    }
    cout << "The game ends after " << currentIteration << " generations with:" << endl;
    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < columns; c++) {
            cout << getGrid(currentIteration)[r][c] << " ";
        }
        cout << endl;
    }
}

bool allDead(int** grid, int startRow, int endRow)
{
    for(int r = startRow; r <= endRow; r++) {
        for(int c = 0; c < columns; c++) {
            //cout << grid[r][c] << endl;
            if(grid[r][c] == 1) {
                return false;
            }
        }
    }
    //cout << "--------------" << endl;
    return true;
}

bool unchanged(int** oldGrid, int** newGrid, int startRow, int endRow)
{
    for(int r = startRow; r <= endRow; r++) {
        for(int c = 0; c < columns; c++) {
            if(oldGrid[r][c] != newGrid[r][c]) {
                return false;
            }
        }
    }
    return true;
}

void cleanup(int threadNumber)
{
    for(int i = 0; i < threadNumber; i++) {
        free(mailboxes[i]);
        free(cachedMessages[i]);
        //pthread_join(*threads[i], NULL);
        sem_destroy(pSems[i]);
        sem_destroy(cSems[i]);
    }
    sem_destroy(pSems[threadNumber]);
    sem_destroy(cSems[threadNumber]);
    free(pSems);
    free(cSems);
    free(mailboxes);
    free(cachedMessages);
    free(threads);
}

void* threadInit(void* passedIndex)
{
    /*
    Receive RANGE message from thread 0 to obtain range of rows.
    for (gen = 1; gen <= cGen; gen++) {
        Receive GO message from thread 0 and play a generation of the game on the thread’s portion of rows.
        Send GENDONE message to thread 0.
    }
    Send ALLDONE message to thread 0.
    */

    int index = *((int*) passedIndex);
    free(passedIndex);
    msg* message = (msg*) malloc(sizeof(msg));
    RecvMsg(index, message); // Receive range message
    message->iSender = index;
    message->type = ALLDONE;

    int startRow = message->value1;
    int endRow = message->value2;

    for(int currentGeneration = 0; currentGeneration <= totalGenerations; currentGeneration++) { // Iterate
        RecvMsg(index, message); // Receive GO message or STOP message
        if(message->type == STOP) {
            pthread_exit(NULL);
        }
        else {
            //cout << "Thread Index: " << index << " | " << (currentGeneration % 2 == 0 ? "Even" : "Odd") << endl;
            int** grid1 = getGrid(currentGeneration);
            int** grid2 = getOtherGrid(currentGeneration);
            // Play generation
            for(int r = startRow; r <= endRow; r++) {
                for(int c = 0; c < columns; c++) {
                    int neighbors = getNeighbors(grid1, r, c);
                    //cout << r << ":" << c << "-" << neighbors << endl;
                    if(grid1[r][c] == 0) {
                        if(neighbors == 3) {
                            // Birth
                            //cout << "Thread Index " << index << " Birth at " << r << "x" << c << endl;
                            grid2[r][c] = 1;
                        }
                    }
                    else {
                        if(neighbors != 2 && neighbors != 3) {
                            // Death
                            //cout << "Thread Index " << index << " Death at " << r << "x" << c << endl;
                            grid2[r][c] = 0;
                        }
                    }
                }
            }

            if(allDead(grid2, startRow, endRow)) {
                message->type = ALLDEAD;
            }
            else if(currentGeneration != 0 && unchanged(cachedGen, grid2, startRow, endRow)) {
                message->type = GENNOTCHANGED;
            }   
            else {
                message->type = GENDONE;
            }
            message->iSender = index;

            SendMsg(0, message);
        }
    }

    message->iSender = index;
    message->type = ALLDONE;

    SendMsg(0, message);

    return (void*) 0;
}

int main(int argc, char* argv[])
{
    int threadNumber;
    string fileName, printRaw, inputRaw;
    bool print = false, input = false;
    threadNumber = atoi(argv[1]);
    fileName = argv[2];
    totalGenerations = atoi(argv[3]);
    if(argc > 4) {
        printRaw = argv[4];
        if(printRaw.compare("y") == 0) {
            print = true;
        }
    }
    if(argc > 5) {
        inputRaw = argv[5];
        if(inputRaw.compare("y") == 0) {
            input = true;
        }
    }

    if(threadNumber > MAXTHREAD) {
        cout << "Too many threads, defaulting to " << MAXTHREAD << " threads" << endl;
        threadNumber = MAXTHREAD;
    }

    initSemaphores(threadNumber);
    initMailboxes(threadNumber);
    initBoards(fileName);
    if(threadNumber > rows) {
        cout << "More threads than rows. Modifying to " << rows << " threads" << endl;
        threadNumber = rows;
    }
    startSend(threadNumber);
    initThreads(threadNumber);
    startGame(threadNumber, print, input);
    cleanup(threadNumber);
}