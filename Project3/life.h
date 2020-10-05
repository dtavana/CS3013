#define RANGE 1
#define ALLDONE 2
#define GO 3
#define GENDONE 4
#define ALLDEAD 5
#define GENNOTCHANGED 6
#define STOP 7

#define MAXGRID 40
#define MAXTHREAD 10

#include "mailbox.h"

extern int** evenGen;
extern int** oddGen;
extern int** cachedGen;
extern int totalGenerations;
extern int rows;
extern int columns;
extern msg** cachedMessages;

