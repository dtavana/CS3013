#include <iostream>
using namespace std;
#include <unordered_map>
#include <string.h>
#include <unistd.h>

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

void loadMapSeq(unordered_map<string, int> &map, unordered_map<string, unordered_map<string, int>> &seqMap, int &totalCalls)
{
    string previousCall = "";
    string currentLine;
    while (getline(cin, currentLine))
    {
        int argumentStart = currentLine.find("(");
        if (argumentStart == -1)
        {
            continue;
        }
        string systemCallName = currentLine.substr(0, argumentStart);

        // Load into the original map
        unordered_map<string, int>::iterator element = map.find(systemCallName);
        if (element == map.end())
        {
            map.insert(pair<string, int>(systemCallName, 1));
        }
        else
        {
            element->second = element->second + 1;
        }

        // Load into sequence map
        if (previousCall != "")
        {
            unordered_map<string, unordered_map<string, int>>::iterator element = seqMap.find(previousCall);
            if (element == seqMap.end())
            {
                // Need to create the inner map
                unordered_map<string, int> innerMap;
                innerMap.insert(pair<string, int>(systemCallName, 1));
                seqMap.insert(pair<string, unordered_map<string, int>>(previousCall, innerMap));
            }
            else
            {
                // At this point, element->second is the inner map, we now need to see if the current systemCall exists
                unordered_map<string, int>::iterator innerElement = element->second.find(systemCallName);
                if (innerElement == element->second.end())
                {
                    element->second.insert(pair<string, int>(systemCallName, 1));
                }
                else
                {
                    innerElement->second = innerElement->second + 1;
                }
            }
        }
        previousCall = systemCallName;
        totalCalls++;
    }
}

void printStatsSeq(unordered_map<string, int> &map, unordered_map<string, unordered_map<string, int>> &seqMap, int &totalCalls)
{
    int uniqueCalls = map.size();
    cout << "AAA: " << totalCalls << " invoked system call instances from " << uniqueCalls << " unique system calls" << endl;
    unordered_map<string, int>::iterator it;
    for (it = map.begin(); it != map.end(); it++)
    {
        cout << it->first << " " << it->second << endl;
        unordered_map<string, unordered_map<string, int>>::iterator element = seqMap.find(it->first);
        if (element != seqMap.end())
        {
            unordered_map<string, int>::iterator innerIt;
            for (innerIt = element->second.begin(); innerIt != element->second.end(); innerIt++)
            {
                cout << "\t" << it->first << ":" << innerIt->first << " " << innerIt->second << endl;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    unordered_map<string, int> map;
    int totalCalls = 0;
    if (argc > 1 && strcmp(argv[1], "seq") == 0)
    {
        // Go into sequence mode
        unordered_map<string, unordered_map<string, int>> seqMap;
        loadMapSeq(map, seqMap, totalCalls);
        printStatsSeq(map, seqMap, totalCalls);
    }
    else
    {
        loadMap(map, totalCalls);
        printStats(map, totalCalls);
    }
}