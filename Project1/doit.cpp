#include <iostream>
using namespace std;
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <vector>
#include <string.h>
#include <unistd.h>

typedef struct {
    int pid;
    string command;
    timeval start;
} process;
vector<process> backgroundTasks;

/* Define limits on characters and arguments for shell mode */
#define MAX_CHARACTERS 128
#define MAX_ARGS 32
#define SEPERATOR "-----------------------------\n"

long int toMs(struct timeval tv) {
    long int wholeSeconds = tv.tv_sec * 1000;
    long int remainderSeconds = tv.tv_usec / 1000;
    return wholeSeconds + remainderSeconds;
}

void printStatistics(struct timeval start) {
    struct timeval end;
    struct rusage r_usage;
    long int userTime, systemTime, wallTime, involuntarily, voluntarily, majorFaults, minorFaults;

    gettimeofday(&end, NULL);
    getrusage(RUSAGE_CHILDREN, &r_usage);

    userTime = toMs(r_usage.ru_utime);
    systemTime = toMs(r_usage.ru_stime);
    wallTime = toMs(end) - toMs(start);
    involuntarily = r_usage.ru_nivcsw;
    voluntarily = r_usage.ru_nvcsw;
    minorFaults = r_usage.ru_minflt;
    majorFaults = r_usage.ru_majflt;

    cout << "Recorded stats" << endl;
    cout << "\tUser Time: " << userTime << " ms" << endl;
    cout << "\tSystem Time: " << systemTime << " ms" << endl;
    cout << "\tWall Clock Time: " << wallTime << " ms" << endl;
    cout << "\tPreempted involuntarily: " << involuntarily << endl;
    cout << "\tGave up CPU voluntarily: " << voluntarily << endl;
    cout << "\tMinor Page Faults: " << minorFaults << endl;
    cout << "\tMajor Page Faults: " << majorFaults << endl;
    cout << SEPERATOR;
}

void printBackgroundTask(int position, process task) {
    cout << "[" << position << "] | " << task.command << " | " << task.pid;
}

/* Returns -1 for exit, 1 for continue, 0 for ignore */
int checkCustomCommands(char* userInput, char* argvNew[MAX_ARGS], string* currentPrompt) {
    if(feof(stdin)) {
        return -1;
    }
    else if(strcmp(userInput, "exit") == 0) {
        return -1;
    }
    else if(strcmp(argvNew[0], "cd") == 0 && argvNew[1] != NULL) {
        int result = chdir(argvNew[1]);
        if(result < 0) {
            // Directory change error
            cerr << "There was a problem changing directories" << endl;
        }
        return 1;
    }
    else if(
        strcmp(argvNew[0], "set") == 0 &&
        strcmp(argvNew[1], "prompt") == 0 &&
        strcmp(argvNew[2], "=") == 0 &&
        argvNew[3] != NULL
        ) {
            *currentPrompt = argvNew[3];
            return 1;
        }
    else if(strcmp(argvNew[0], "jobs") == 0) {
        if(backgroundTasks.size() == 0) {
            cout << "There are currently no background tasks running" << endl;
        }
        else {
            for(int i = 0; i < backgroundTasks.size(); i++) {
                process task = backgroundTasks.at(i);
                printBackgroundTask(i + 1, task);
                cout << endl;
            }
        }
        return 1;
    }
    return 0;
}

void loadArgumentsFromInvocation(int argc, char* argv[], char* argvNew[MAX_ARGS]) {
    for(int i = 0; i < MAX_ARGS; i++) {
        argvNew[i] = argv[i + 1];
        argvNew[i + 1] = NULL;
    }
}

int loadArgumentsFromInput(char* userInput, char* argvNew[MAX_ARGS]) {
    char* delimited = strtok(userInput, " ");
    int i = 0;

    while(delimited != NULL && i <= MAX_ARGS) {
        argvNew[i] = delimited;
        argvNew[i + 1] = NULL;
        delimited = strtok(NULL, " ");
        i++;
    }
    return i;
}

bool checkBackgroundTask(int position, char* argvNew[MAX_ARGS]) {
    if(strcmp(argvNew[position - 1], "&") == 0) {
        /* Remove the '&' from the command */
        argvNew[position - 1] = NULL;
        return true;
    }
    return false;
}

void processBackgroundJobs() {
    for(int i = 0; i < backgroundTasks.size(); i++) {
        int status;
        process task = backgroundTasks.at(i);
        pid_t result = waitpid(task.pid, &status, WNOHANG);
        if(result > 0) {
            // Task finished, print information
            printBackgroundTask(i + 1, task);
            cout << " Completed" << endl;
            cout << SEPERATOR;
            printStatistics(task.start);
            // Might need to be erase
            backgroundTasks.erase(backgroundTasks.begin() + i);
        }
    }
}

void startFork(char* argvNew[MAX_ARGS], bool isBackground) {
    int pid;
    struct timeval start;

    /* Create child process */
    gettimeofday(&start, NULL);
    pid = fork();
    
    if(pid < 0) {
        /* Error forking */
        cerr << "Error forking\n";
        exit(1);
    }
    else if(pid == 0) {
        /* In child process */
        if(execvp(argvNew[0], argvNew) < 0) {
            /* Error with execvp */
            cerr << "Error with exec\n";
            exit(1);
        }
    }
    else {
        /* In parent, wait for child to finish */
        if(!isBackground) {
            /* Only wait if the task is not a background task */ 
            wait(0);
            printStatistics(start);
        }
        else {
            /* Queue a new task */ 
            process newTask = { pid, argvNew[0], start };
            backgroundTasks.push_back(newTask);
            printBackgroundTask(backgroundTasks.size(), newTask);
            cout << endl;
        }
    }
}

void sanitizeInput(char* input) {
    int len = strlen(input);
    if(len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }
}

void cleanExit() {
    if(backgroundTasks.size() > 0) {
        cout << SEPERATOR;
        cout << endl << "Waiting for background processes to complete" << endl;
    }
    for(int i = 0; i < backgroundTasks.size(); i++) {
        process task = backgroundTasks.at(i);
        int status;
        /* Instead of WNOHANG, 
        use 0 as we do want to 
        wait for the child to finish execution */
        pid_t result = waitpid(task.pid, &status, 0);
        if(result > 0) {
            /* Child finished */
            printBackgroundTask(i + 1, task);
            cout << " Completed" << endl;
            printStatistics(task.start);
        }
    }
    exit(1);
}

int main(int argc, char* argv[]) {
    char* argvNew[MAX_ARGS];
    char userInput[MAX_CHARACTERS];
    string currentPrompt = "==>";

    if(argc == 1) {
        /* Enter shell mode */
        while(1) {
            cout << currentPrompt << " ";
            cin.getline(userInput, MAX_CHARACTERS);
            processBackgroundJobs();
            sanitizeInput(userInput);
            int position = loadArgumentsFromInput(userInput, argvNew);
            bool isBackground = checkBackgroundTask(position, argvNew);
            int result = checkCustomCommands(userInput, argvNew, &currentPrompt);
            if(result == -1) {
                cleanExit();
            }
            else if(result == 1) {
                continue;
            }
            startFork(argvNew, isBackground);
        }
    }
    else {
        /* Direct command mode */
        loadArgumentsFromInvocation(argc, argv, argvNew);
        startFork(argvNew, false);
    }
}

