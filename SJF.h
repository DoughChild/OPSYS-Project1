//
// Created by Iffat Nafisa on 3/15/22.
//
//#include "process.h"
#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <vector>

#include "functions.h"

using namespace std;

void SJF(deque<Process*> processes, double tau, int t_cs, double alpha) {
    // error check
    if (processes.size() == 0) {
        return;
    }
    if (t_cs < 0 || alpha < 0) {
        return;
    }

    // time calculations
    double average_CPUBurst_time = 0.0;
    double average_wait_time = 0.0;
    double average_turnaround_time = 0.0;
    //int total_preemptions = 0;
    //double CPU_utilization = 0.0;
    int totalCPUBurst = 0;
    int totalWait = 0;
    int totalTurnAround = 0;


    int clock = 0;

    vector<Process*> readyQ;
    map<int, vector<Process*> > waitingMap;
    // key is the IO bust end time i-e time for next interesting event
    vector<Process*> processesVector;
    for (unsigned long i = 0; i < processes.size(); i++) {
        Process *p = new Process();
        p->copy(processes[i]);
        processesVector.push_back(p);
        totalCPUBurst += processes[i]->CPUBursts.size();
        average_CPUBurst_time += calculateAverageCPUBurst(processes[i]);
    }
    average_CPUBurst_time = average_CPUBurst_time / totalCPUBurst;

    sort(processesVector.begin(), processesVector.end(), shortestArrival());

    // count for processes completed
    int numProcesses = processes.size();
    int completedProcess = 0;

    // count for which process is using the CPU
    Process* usingCPU = NULL;
    // if CPU is in use
    bool CPUActive = false;
    // clock time when a CPU is released and starts context switching for next process
    //int releaseCPU = 0;

    // context switch counts
    //bool contextSwitch = false; // if any process is currently context switching
    Process* contextSwitchingIn = NULL; // process which is currently context switching in the CPU
    Process* contextSwitchingOut = NULL; // process which is currently context switching out the CPU
    int contextSwitchUntil = -99999999; // until which time a process is context switching in or out
    int contextSwitchCount = 0; // total number of context switches

    // start of simulation
    printf("time %dms: Simulator started for SJF [Q empty]\n", clock);

    while (true) {
        // check if any process can start using the CPU
        if (!CPUActive && contextSwitchUntil <= clock && contextSwitchingIn != NULL) {
            // process contextSwitchingIn should enter the CPU
            contextSwitchingIn->status = "CPU";
            CPUActive = true;
            contextSwitchCount += 1;
            usingCPU = contextSwitchingIn;
            printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst ", clock,
                   usingCPU->name, usingCPU->tau, usingCPU->CPUBursts[0]);
            printQueue(readyQ);
            updateOnCPUEntry(usingCPU, clock, t_cs);
            // release the CPU after contextSwitch
            //releaseCPU = clock + usingCPU->time_for_next_interesting_event;
            //contextSwitch = false;
            contextSwitchingIn = NULL;
        }

        // CPU Burst completion
        if (CPUActive) {
            if(usingCPU->time_for_next_interesting_event == clock) {
                // check if any CPU Burst left
                if (usingCPU->CPUBursts.size() > 0) {
                    if (usingCPU->CPUBursts.size() == 1) {
                        printf("time %dms: Process %c (tau %dms) completed a CPU burst; %lu burst to go ", clock,
                               usingCPU->name, usingCPU->tau, usingCPU->CPUBursts.size());
                        } else {
                        printf("time %dms: Process %c (tau %dms) completed a CPU burst; %lu bursts to go ", clock,
                               usingCPU->name, usingCPU->tau, usingCPU->CPUBursts.size());
                    }
                    printQueue(readyQ);
                    // recalculate Tau
                    int newTau = calc_tau(usingCPU->tau, usingCPU->cur_CPUBurst, alpha);
                    printf("time %dms: Recalculated tau from %dms to %dms for process %c ", clock, usingCPU->tau
                            , newTau, usingCPU->name);
                    usingCPU->tau = newTau;
                    printQueue(readyQ);

                    // CONTEXT SWITCH OUT OF CPU
                    //contextSwitch = true;
                    contextSwitchingOut = usingCPU;
                    contextSwitchingOut->status = "Waiting";
                    updateOnSwitchOutOfCPU(contextSwitchingOut, clock, t_cs);
                    printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", clock, usingCPU->name,
                           usingCPU->time_for_next_interesting_event);
                    printQueue(readyQ);
                    contextSwitchUntil = clock + (t_cs / 2);

                    // add to turn around time
                    totalTurnAround += 1;
                    average_turnaround_time += (contextSwitchUntil - contextSwitchingOut->arrived_readyQ);

                    // add to waiting map
                    waitingMap[contextSwitchingOut->time_for_next_interesting_event].push_back(contextSwitchingOut);
                    usingCPU = NULL;

                    // Release CPU
                    CPUActive = false;
                    //releaseCPU = clock;
                }
                else {
                    // the process terminates
                    printf("time %dms: Process %c terminated ", clock, usingCPU->name);
                    printQueue(readyQ);
                    completedProcess += 1;

                    // CONTEXT SWITCH
                    //contextSwitch = true;
                    contextSwitchingOut = usingCPU;
                    contextSwitchingOut->status = "Terminated";
                    contextSwitchUntil = clock + (t_cs / 2);
                    // add to turn around time
                    totalTurnAround += 1;
                    average_turnaround_time += (contextSwitchUntil - contextSwitchingOut->arrived_readyQ);

                    // Release CPU
                    CPUActive = false;
                    //releaseCPU = clock;
                }
            }
        }

        // I/O Burst completion
        map<int, vector<Process *> >::iterator it;
        it = waitingMap.find(clock);
        if(it != waitingMap.end()) {
            // finish IO for the processes in the list and erase the processes from waitingMap
            for (unsigned long i = 0; i < it->second.size(); i++)  {
                // finished IO
                // re-enter the readyQ
                Process * temp2 = it->second[i];
                temp2->arrived_readyQ = clock;
                temp2->in_rq = true;
                temp2->status = "Ready";
                // Push to readyQ and sort
                readyQ.push_back(temp2);
                sort(readyQ.begin(), readyQ.end(), shortestJob());
                printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ", clock, temp2->name ,
                       temp2->tau);
                printQueue(readyQ);
            }
            // delete from waiting map
            waitingMap.erase(it);
        }

        // New Process Arrivals
        vector<Process*> arrivedProcesses;
        arrivedProcesses = processArrived(processesVector, clock);
        if (arrivedProcesses.size() > 0) {
            for (unsigned long i = 0; i < arrivedProcesses.size(); i++) {
                arrivedProcesses[i]->arrived_readyQ = clock;
                arrivedProcesses[i]->in_rq = true;
                readyQ.push_back(arrivedProcesses[i]);
                sort(readyQ.begin(), readyQ.end(), shortestJob());
                printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ", clock,
                       arrivedProcesses[i]->name, arrivedProcesses[i]->tau);
                printQueue(readyQ);
            }
        }

        if (numProcesses == completedProcess) {
            clock += (t_cs / 2);
            printf("time %dms: Simulator ended for SJF ", clock);
            printQueue(readyQ);
            break;
        }

        // CONTEXT SWITCH IN
        // if readyQ is not empty, CPU is not active, and if no process is contextSwitching
        if (readyQ.size() > 0  && !CPUActive && contextSwitchUntil <= clock) {
            contextSwitchingIn = readyQ.at(0);
            //contextSwitch = true;
            contextSwitchUntil = clock + (t_cs/2);
            // remove from readyQ
            readyQ.erase(readyQ.begin());
            // Calculate wait time
            // wait time = clock - in_rq
            average_wait_time += double(clock - contextSwitchingIn->arrived_readyQ);
            totalWait += 1;
        }

        clock++;
    }

    // printf("Average CPU Burst Time %.3f\n", average_CPUBurst_time);
    // printf("Average Wait Time %.4f\n", average_wait_time/double(totalWait));
    // printf("Average Turn Around Time %.4f\n", average_turnaround_time/double(totalTurnAround));
    // cout << "TOTAL CONTEXT SWITCH " << contextSwitchCount << endl;
    // printf("Total preemptions %d\n", total_preemptions);

    ofstream file;
    file.open("simout.txt", std::fstream::app | std::fstream::out);
    file << "Algorithm SJF\n";
    // file << "-- average CPU burst time: " << avg_burst << " ms\n";
    // file << "-- average wait time: " << avg_wait << " ms\n";
    // file << "-- average turnaround time: " << avg_turnaround << " ms\n";
    // file << "-- total number of context switches: " << cs_count << "\n";
    // file << "-- total number preemptions: " << preempt_count << "\n";
    // file << "-- CPU utilization: " << CPU_util << "%\n";
    file.close();
}

