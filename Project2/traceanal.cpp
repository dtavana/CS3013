#include <iostream>
using namespace std;
#include <unordered_map>

void loadMap(unordered_map<string, int> &map, int &totalCalls)
{
    string currentLine;
    while (getline(cin, currentLine))
    {
        int argumentStart = currentLine.find("(");
        if (argumentStart == -1)
        {
            continue;
        }
        string systemCallName = currentLine.substr(0, argumentStart);
        unordered_map<string, int>::iterator element = map.find(systemCallName);
        if (element == map.end())
        {
            map.insert(pair<string, int>(systemCallName, 1));
        }
        else
        {
            element->second = element->second + 1;
        }
        totalCalls++;
    }
}

void printStats(unordered_map<string, int> &map, int &totalCalls)
{
    int uniqueCalls = map.size();
    cout << "AAA: " << totalCalls << " invoked system call instances from " << uniqueCalls << " unique system calls" << endl;
    unordered_map<string, int>::iterator it;
    for (it = map.begin(); it != map.end(); it++)
    {
        cout << it->first << " " << it->second << endl;
    }
}

int main(int argc, char *argv[])
{
    unordered_map<string, int> map;
    int totalCalls;
    loadMap(map, totalCalls);
    printStats(map, totalCalls);
}