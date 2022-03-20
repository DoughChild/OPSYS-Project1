//
// Created by Iffat Nafisa on 3/15/22.
//
//#include "process.h"
#include <deque>
#include <iostream>
#include <map>
#include <set>
#include "functions.h"
using namespace std;

void FCFS(deque<Process*> processes, double tau, int t_cs, double alpha) {
    // error check
    if (processes.size() == 0) {
        return;
    }
    if (t_cs < 0 || alpha < 0) {
        return;
    }

    int clock = 0;

    vector<Process*> readyQ;
    map<int, vector<Process*> > waitingMap;
    // key is the IO bust end time i-e time for next interesting event
    vector<Process*> processesVector;
    for (int i = 0; i < processes.size(); i++) {
        Process *p = new Process();
        p->copy(processes[i]);
        processesVector.push_back(p);
    }

    sort(processesVector.begin(), processesVector.end(), shortestArrival());

    // count for processes completed
    int numProcesses = processes.size();
    int completedProcess = 0;

    // count for which process is using the CPU
    Process* usingCPU = NULL;
    // if CPU is in use
    bool CPUActive = false;
    // clock time when a CPU is released and starts context switching for next process
    int releaseCPU = 0;

    // context switch counts
    bool contextSwitch = false; // if any process is currently context switching
    Process* contextSwitchingIn = NULL; // process which is currently context switching in the CPU
    Process* contextSwitchingOut = NULL; // process which is currently context switching out the CPU
    int contextSwitchUntil = -99999999; // until which time a process is context switching in or out
    int contextSwitchCount = 0; // total number of context switches

    // start of simulation
    printf("time %dms: Simulator started for FCFS [Q empty]\n", clock);

    while (true) {
        // check if any process can start using the CPU
        if (!CPUActive && contextSwitchUntil <= clock && contextSwitchingIn != NULL) {
            // process contextSwitchingIn should enter the CPU
            contextSwitchingIn->status = "CPU";
            CPUActive = true;
            contextSwitchCount += 1;
            usingCPU = contextSwitchingIn;
            printf("time %dms: Process %c started using the CPU for %dms burst ", clock,
                   usingCPU->name, usingCPU->CPUBursts[0]);
            printQueue(readyQ);
            updateOnCPUEntry(usingCPU, clock, t_cs);
            // release the CPU after contextSwitch
            releaseCPU = clock + usingCPU->time_for_next_interesting_event;
            contextSwitch = false;
            contextSwitchingIn = NULL;
        }

        // CPU Burst completion
        if (CPUActive) {
            if(usingCPU->time_for_next_interesting_event == clock) {
                // check if any CPU Burst left
                if (usingCPU->CPUBursts.size() > 0) {
                    if (usingCPU->CPUBursts.size() == 1) {
                        printf("time %dms: Process %c completed a CPU burst; %lu burst to go ", clock,
                               usingCPU->name, usingCPU->CPUBursts.size());
                    } else {
                        printf("time %dms: Process %c completed a CPU burst; %lu bursts to go ", clock,
                               usingCPU->name, usingCPU->CPUBursts.size());
                    }
                    printQueue(readyQ);

                    // CONTEXT SWITCH OUT OF CPU
                    contextSwitch = true;
                    contextSwitchingOut = usingCPU;
                    contextSwitchingOut->status = "Waiting";
                    updateOnSwitchOutOfCPU(contextSwitchingOut, clock, t_cs);
                    printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", clock, usingCPU->name,
                           usingCPU->time_for_next_interesting_event);
                    printQueue(readyQ);
                    contextSwitchUntil = clock + (t_cs / 2);

                    // add to waiting map
                    waitingMap[contextSwitchingOut->time_for_next_interesting_event].push_back(contextSwitchingOut);
                    usingCPU = NULL;

                    // Release CPU
                    CPUActive = false;
                    releaseCPU = clock;
                }
                else {
                    // the process terminates
                    printf("time %dms: Process %c terminated ", clock, usingCPU->name);
                    printQueue(readyQ);
                    completedProcess += 1;

                    // CONTEXT SWITCH
                    contextSwitch = true;
                    contextSwitchingOut = usingCPU;
                    contextSwitchingOut->status = "Terminated";
                    contextSwitchUntil = clock + (t_cs / 2);

                    // Release CPU
                    CPUActive = false;
                    releaseCPU = clock;
                }
            }
        }

        // I/O Burst completion
        map<int, vector<Process *> >::iterator it;
        it = waitingMap.find(clock);
        if(it != waitingMap.end()) {
            // finish IO for the processes in the list and erase the processes from waitingMap
            for (int i = 0; i < it->second.size(); i++)  {
                // finished IO
                // re-enter the readyQ
                Process * temp2 = it->second[i];
                temp2->arrived_readyQ = clock;
                temp2->in_rq = true;
                temp2->status = "Ready";
                // Push to readyQ and sort
                readyQ.push_back(temp2);
                printf("time %dms: Process %c completed I/O; added to ready queue ", clock, temp2->name);
                printQueue(readyQ);
            }
            // delete from waiting map
            waitingMap.erase(it);
        }

        // New Process Arrivals
        vector<Process*> arrivedProcesses;
        arrivedProcesses = processArrived(processesVector, clock);
        if (arrivedProcesses.size() > 0) {
            for (int i = 0; i < arrivedProcesses.size(); i++) {
                arrivedProcesses[i]->arrived_readyQ = clock;
                arrivedProcesses[i]->in_rq = true;
                readyQ.push_back(arrivedProcesses[i]);
                printf("time %dms: Process %c arrived; added to ready queue ", clock,
                       arrivedProcesses[i]->name);
                printQueue(readyQ);
            }
        }

        if (numProcesses == completedProcess) {
            clock += (t_cs / 2);
            printf("time %dms: Simulator ended for FCFS ", clock);
            printQueue(readyQ);
            break;
        }

        // CONTEXT SWITCH IN
        // if readyQ is not empty, CPU is not active, and if no process is contextSwitching
        if (readyQ.size() > 0  && !CPUActive && contextSwitchUntil <= clock) {
            contextSwitchingIn = readyQ.at(0);
            contextSwitch = true;
            contextSwitchUntil = clock + (t_cs/2);
            // remove from readyQ
            readyQ.erase(readyQ.begin());
        }

        clock++;
    }
//    cout << "TOTAL CONTEXT SWITCH " << contextSwitchCount << endl;
}

