//
// Created by Iffat Nafisa on 3/15/22.
//
//#include "process.h"
#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <iomanip>
#include "functions.h"
using namespace std;

void FCFS(deque<Process*> processes, int t_cs, double alpha) {
    // error check
    if (processes.size() == 0) {
        return;
    }
    if (t_cs < 0 || alpha < 0) {
        return;
    }

    int clock = 0;
    int totalPrint = 999;
    // time calculations
    double average_CPUBurst_time = 0.0;
    double average_wait_time = 0.0;
    double average_turnaround_time = 0.0;
    int total_preemptions = 0;
    double CPU_utilization = 0.0;
    int totalCPUBurst = 0;
    int totalWait = 0;
    int totalTurnAround = 0;

    vector<Process*> readyQ;
    map<int, vector<Process*> > waitingMap;
    // key is the IO bust end time i-e time for next interesting event
    vector<Process*> processesVector;
    for (int i = 0; i < processes.size(); i++) {
        processesVector.push_back(processes[i]);
        totalCPUBurst += processes[i]->CPUBursts.size();
        average_CPUBurst_time += calculateAverageCPUBurst(processes[i]);
    }


    sort(processesVector.begin(), processesVector.end(), shortestArrival());

    // count for processes completed
    int numProcesses = processes.size();
    int completedProcess = 0;

    // count for which process is using the CPU
    Process* usingCPU = NULL;
    // if CPU is in use
    bool CPUActive = false;

    // context switch counts
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
            if (clock <= totalPrint) {
                printf("time %dms: Process %c started using the CPU for %dms burst ", clock,
                       usingCPU->name, usingCPU->CPUBursts[0]);
                printQueue(readyQ);
            }


            // cpu utilization calculation
            CPU_utilization += (clock - usingCPU->CPUBursts[0]);


            updateOnCPUEntry(usingCPU, clock, t_cs);
            // release the CPU after contextSwitch
            contextSwitchingIn = NULL;

        }

        // CPU Burst completion
        if (CPUActive) {
            if(usingCPU->time_for_next_interesting_event == clock) {
                // check if any CPU Burst left
                if (usingCPU->CPUBursts.size() > 0) {
                    if (clock <= totalPrint) {
                        if (usingCPU->CPUBursts.size() == 1) {
                            printf("time %dms: Process %c completed a CPU burst; %lu burst to go ", clock,
                                   usingCPU->name, usingCPU->CPUBursts.size());
                        } else {
                            printf("time %dms: Process %c completed a CPU burst; %lu bursts to go ", clock,
                                   usingCPU->name, usingCPU->CPUBursts.size());
                        }
                        printQueue(readyQ);
                    }

                    // CONTEXT SWITCH OUT OF CPU
                    contextSwitchingOut = usingCPU;
                    contextSwitchingOut->status = "Waiting";
                    updateOnSwitchOutOfCPU(contextSwitchingOut, clock, t_cs);
                    if (clock <= totalPrint) {
                        printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", clock, usingCPU->name,
                               usingCPU->time_for_next_interesting_event);
                        printQueue(readyQ);
                    }

                    contextSwitchUntil = clock + (t_cs / 2);

                    // add to turn around time
                    totalTurnAround += 1;
                    average_turnaround_time += (contextSwitchUntil - contextSwitchingOut->arrived_readyQ);

                    // add to waiting map
                    waitingMap[contextSwitchingOut->time_for_next_interesting_event].push_back(contextSwitchingOut);
                    usingCPU = NULL;

                    // Release CPU
                    CPUActive = false;
                }
                else {
                    // the process terminates
                    printf("time %dms: Process %c terminated ", clock, usingCPU->name);
                    printQueue(readyQ);
                    completedProcess += 1;

                    // CONTEXT SWITCH
                    contextSwitchingOut = usingCPU;
                    contextSwitchingOut->status = "Terminated";
                    contextSwitchUntil = clock + (t_cs / 2);

                    // add to turn around time
                    totalTurnAround += 1;
                    average_turnaround_time += (contextSwitchUntil - contextSwitchingOut->arrived_readyQ);

                    // Release CPU
                    CPUActive = false;
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
                if (clock <= totalPrint) {
                    printf("time %dms: Process %c completed I/O; added to ready queue ", clock, temp2->name);
                    printQueue(readyQ);
                }

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
                if (clock <= totalPrint) {
                    printf("time %dms: Process %c arrived; added to ready queue ", clock,
                           arrivedProcesses[i]->name);
                    printQueue(readyQ);
                }

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
            contextSwitchUntil = clock + (t_cs/2);
            // remove from readyQ
            readyQ.erase(readyQ.begin());
            average_wait_time += double(clock - contextSwitchingIn->arrived_readyQ);
            totalWait += 1;
        }

        clock++;
    }
    FILE *fp;
    fp = fopen("simout.txt","w");
    fprintf(fp, "Algorithm FCFS\n");
    fprintf(fp, "-- average CPU burst time: %.3fms\n", ceil(average_CPUBurst_time/ double(totalCPUBurst) * 1000)/1000);
    fprintf(fp, "-- average wait time: %.3fms\n", ceil(average_wait_time/double(totalCPUBurst) * 1000) / 1000);
    fprintf(fp, "-- average turnaround time: %.3fms\n", ceil(average_turnaround_time/double(totalCPUBurst) * 1000)/ 1000);
    fprintf(fp, "-- total number of context switches: %d\n", contextSwitchCount);
    fprintf(fp, "-- total number of preemptions: %d\n", total_preemptions);
    fprintf(fp, "-- CPU utilization: %.3f%%\n", ceil(average_CPUBurst_time * 100.00 / double(clock) * 1000) / 1000);
    fclose(fp);
}

