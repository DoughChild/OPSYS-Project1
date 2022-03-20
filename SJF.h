//
// Created by Iffat Nafisa on 3/15/22.
//

#ifndef CPUSCHEDULING_SJF_H
#define CPUSCHEDULING_SJF_H
#include "process.h"
#include <deque>
#include <iostream>
#include <map>
#include "tau_calc.h"
#include <set>
// t_cs ---> Context Switch
// tau     - estimated CPU burst time
// alpha   - constant in the range [0.0,1.0), often 0.5 or higher
// tau     =  alpha  x  t   +  (1-alpha)  x  tau
//      i+1                i                      i


//public:
//char name;
//int num_bursts;
//int t_arrival;
//int tau; // cpu burst time
//int tau_remaining;
//int wait_time;
//int turnaround_time;
//int cur_CPUBurst;
//int cs_time_left;
//
////bool in_rq;
//
///* will pop CPU and I/O bursts that have finished */
//deque<int> CPUBursts;
//deque<int> IOBursts;
using namespace std;
// Global time counter for process stimulation

vector<int> contextSwitchTimes;

struct shortestJob {
public:
    bool operator() (Process const *p1, Process const *p2) {
        if (p1->tau < p2->tau) {
            return true;
        }

        else if (p1->tau > p2->tau) {
            return false;
        }
        else {
            if (p1->name <= p2->name) {
                return true;
            }
            else {
                return false;
            }
        }
    }
};

struct shortestArrival {
public:
    bool operator() (Process const *p1, Process const *p2) {
        if (p1->t_arrival < p2->t_arrival) {
            return true;
        }

        else if (p1->t_arrival > p2->t_arrival) {
            return false;
        }
        else {
            if (p1->name <= p2->name) {
                return true;
            }
            else {
                return false;
            }
        }
    }
};


void printProcesses(vector<Process*> processes) {
    cout << "Printing out processes vector" << endl;
    for (int i = 0; i < processes.size(); i++) {
        processes[i]->printProcess();
    }
}


vector<Process *> processArrived(vector<Process*> processes, int t) {
    vector <Process*> arrivedProcesses;
    for (int i = 0; i < processes.size(); i++) {
        if (processes[i]->t_arrival == t) {
            arrivedProcesses.push_back(processes[i]);
        }
    }
    return arrivedProcesses;
}

void printQueue (vector<Process*> processes) {
    cout << "[Q ";
    if (processes.size() == 0) {
        cout << "empty]" << endl;
        return;
    }
    for (int i = 0; i < processes.size(); i++) {
        cout << processes[i]->name;
    }
    cout << "]" << endl;
}

void updateOnCPUBurst(Process *p, int t, int t_cs) {
    double waittime = t - p->arrived_readyQ - double(t_cs / 2.0);
    p->wait_time += waittime;
    p->cur_CPUBurst = p->CPUBursts[0];
    p->CPUBursts.erase(p->CPUBursts.begin());
    p->time_for_next_interesting_event = t + p->cur_CPUBurst;
    // cout << "UPDATE on CPU BURST " << p->time_for_next_interesting_event << endl;
}

void updateOnSwitchOutOfCPU(Process *p, int t, int t_cs) {
    p->time_for_next_interesting_event = t + (t_cs / 2) + p->IOBursts[0];
    p->IOBursts.erase(p->IOBursts.begin());
}

void addContextSwitch(int t_cs, int time) {
    for ( int i = 0; i < t_cs/2; i++) {
        time += 1;
        contextSwitchTimes.push_back(time);
    }
}
void SJF(deque<Process*> processes, double tau, int t_cs, double alpha) {
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
    int numProcesses = processes.size();
    int completedProcess = 0;
    Process* cpuInUse = NULL;
    bool CPUActive = false; // cpu not in use
    int releaseCPU = 0;
    int contextSwitchCount = 0;
    int contextSwitchUntil = -99999999;
    // store each clock time when the CPU is contextswitching
    printf("time %dms: Simulator started for SJF [Q empty]\n", clock);
    while (true) {

//        if (contextSwitchTimes.size() > 0) {
//            cout << "last contextSwitchTime " << contextSwitchTimes[contextSwitchTimes.size() - 1] << endl;
//        }

        // check for CPU Burst
        if (!CPUActive) {
            // enter if statement
            bool enter = true;

            if (contextSwitchUntil > 0 && contextSwitchUntil >= clock) {
                enter = false;
            }
            if (enter && readyQ.size() > 0 && ((readyQ.at(0)->time_for_next_interesting_event == clock)  ||
                    (readyQ.at(0)->time_for_next_interesting_event < (clock - t_cs/2) && releaseCPU == clock)) ){
                // get the first element
//                cout << "11111111111" << endl;
//                cout << "Time " << clock << endl;
//                cout << "ReleaseCPU " << releaseCPU << " Entered the loop " << clock << "(readyQ.at(0)->time_for_next_interesting_event " << readyQ.at(0)->time_for_next_interesting_event << endl;
//                cout << "Entered the loop time = " << clock << endl;
                cpuInUse = readyQ.at(0);
                CPUActive = true;
                cpuInUse->in_rq = false;
                // erase from readyQ
                readyQ.erase(readyQ.begin());
//                contextSwitchUntil = clock + cpuInUse->CPUBursts[0] + t_cs;
                printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst ", clock,
                       cpuInUse->name, cpuInUse->tau, cpuInUse->CPUBursts[0]);
                // update releaseCPU
                printQueue(readyQ);
                // remove the first CPU burst from the list
                // update cur_CPUBurst and wait time
                // Update next Interest time

                updateOnCPUBurst(cpuInUse, clock, t_cs);
                releaseCPU = cpuInUse->time_for_next_interesting_event;
//                cout << "Is A using the CPU " << cpuInUse->name << "CPUActive " << CPUActive << endl;
            }
        }

        // I/O
        // if CPU Burst finished, estimate the next CPU Burst Time
        if (CPUActive) {
            // needs to perform context switch
            // a boolean may be?
            if (cpuInUse->time_for_next_interesting_event == clock) {
                // if there is more CPU Burst left
//                cout << "CPU is active " << cpuInUse->name << " is using the cpu" << endl;
                if (cpuInUse->CPUBursts.size() > 0) {
//                    cout << "2222222222222" << endl;
//                    cout << "Time " << clock << endl;
                    if (cpuInUse->CPUBursts.size() == 1) {
                        printf("time %dms: Process %c (tau %dms) completed a CPU burst; %lu burst to go ", clock,
                               cpuInUse->name, cpuInUse->tau, cpuInUse->CPUBursts.size());
                    }
                    else {
                        printf("time %dms: Process %c (tau %dms) completed a CPU burst; %lu bursts to go ", clock,
                               cpuInUse->name, cpuInUse->tau, cpuInUse->CPUBursts.size());
                    }

                    printQueue(readyQ);
                    int newTau = calc_tau(cpuInUse->tau, cpuInUse->cur_CPUBurst, alpha);
//                    cout << "cpuInUse->cur_CPUBurst " << cpuInUse->cur_CPUBurst << " tau " << cpuInUse->tau << endl;
                    printf("time %dms: Recalculated tau from %dms to %dms for process %c ", clock, cpuInUse->tau
                    , newTau, cpuInUse->name);
                    cpuInUse->tau = newTau;
                    printQueue(readyQ);
                    // switch out of CPU
                    updateOnSwitchOutOfCPU(cpuInUse, clock, t_cs);
                    printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", clock, cpuInUse->name,
                           cpuInUse->time_for_next_interesting_event);
                    printQueue(readyQ);
                    // add the process into waiting queue
                    Process * temp = cpuInUse;
                    //waitingQ.push_back(temp);
                    waitingMap[temp->time_for_next_interesting_event].push_back(temp);
                    cpuInUse = NULL;
                    CPUActive = false;
                    // Context Switch
                }
                else { // process terminates
//                    cout << "3333333333333" << endl;
//                    cout << "Time " << clock << endl;
                    printf("time %dms: Process %c terminated ", clock, cpuInUse->name);
                    printQueue(readyQ);
                    completedProcess += 1;
//                    cout << "COMPLETED PROCESSES " << completedProcess << endl;
                    cpuInUse = NULL;
                    CPUActive = false;
//                    addContextSwitch(t_cs, clock);
                    // needs to perform context switch
                }
                releaseCPU = clock + t_cs;
                contextSwitchUntil = clock + (t_cs / 2);
            }
        }

        // Finish I/O

        //vector<Process*> waitingQTemp;
        map<int, vector<Process *> >::iterator it;
        it = waitingMap.find(clock);
        if(it != waitingMap.end()) {
            // finish IO for the processes in the list and erase the processes from waitingMap
            for (int i = 0; i < it->second.size(); i++) {
//                cout << "444444444444" << endl;
//                cout << "Time " << clock << endl;
                // finished IO
                // re-enter the readyQ
                Process * temp2 = it->second[i];
                temp2->arrived_readyQ = clock;
                temp2->in_rq = true;
                // push to readyQ
                readyQ.push_back(temp2);
                sort(readyQ.begin(), readyQ.end(), shortestJob());
                printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ", clock, temp2->name ,
                       temp2->tau);
                printQueue(readyQ);
                // if CPU in use
                // or if readyQ is not empty and the first element is not process
                if(CPUActive || (readyQ.size() != 0 && temp2 != readyQ.at(0))) {
                    temp2->time_for_next_interesting_event = clock;
                } else {
                    if (!CPUActive && readyQ.size() > 0 && readyQ.at(0)->name == temp2->name) {
                        if (contextSwitchUntil > 0 && contextSwitchUntil > clock) {
                            temp2->time_for_next_interesting_event = contextSwitchUntil + (t_cs / 2.0);
                        }
                        else {
                            temp2->time_for_next_interesting_event = clock + t_cs / 2.0;
                        }
                    }
                    else {
                        temp2->time_for_next_interesting_event = clock;
                    }
                }
                // increment context switch
                contextSwitchCount += 1;
            }
//            // context switch
//            releaseCPU = clock + (t_cs / 2);
            // erase from map
            waitingMap.erase(it);
        }

        if (CPUActive && (releaseCPU == clock)) {
            CPUActive = false;
            cout << "6666666666666666" << endl;
            cout << "Time " << clock << endl;
            if (readyQ.size() > 0) {
                readyQ.at(0)->time_for_next_interesting_event = clock + (t_cs / 2);
                cout << "77777777777777" << endl;
                cout << "Time " << clock << endl;
                // context switch starting
                releaseCPU = clock + (t_cs / 2);
            }
        }

        // constructing ready queue
        vector<Process*> arrivedProcesses;
        arrivedProcesses = processArrived(processesVector, clock);
        // check if any process has arrived
        if (arrivedProcesses.size() > 0) {
            for (int i = 0; i < arrivedProcesses.size(); i++) {
                arrivedProcesses[i]->arrived_readyQ = clock;
                arrivedProcesses[i]->in_rq = true;
                readyQ.push_back(arrivedProcesses[i]);
                // sort by CPU burst
                sort(readyQ.begin(), readyQ.end(), shortestJob());
                printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ", clock,
                       arrivedProcesses[i]->name, arrivedProcesses[i]->tau);
                printQueue(readyQ);
                if (!CPUActive) {
//                    cout << "8888888888888888" << endl;
//                    cout << "Time " << clock << endl;
                    arrivedProcesses[i]->time_for_next_interesting_event = clock + (t_cs / 2);
//                    addContextSwitch(t_cs, clock);
                }
                else {
//                    cout << "99999999999999" << endl;
//                    cout << "Time " << clock << endl;
                    arrivedProcesses[i]->time_for_next_interesting_event = clock;
                }
                contextSwitchCount += 1;
            }
        }

        if (numProcesses == completedProcess) {
//            cout << "101010101010101010101010" << endl;
//            cout << "Time " << clock << endl;
            addContextSwitch(t_cs, clock);
            clock += (t_cs / 2);
            printf("time %dms: Simulator ended for SJF ", clock);
            printQueue(readyQ);
            break;
        }
//        cout << "CLOCKCLOCKCLOCK" << endl;
//        cout << "Time " << clock << endl;
//        cout << "ReleaseCPU " << releaseCPU << endl;
        clock++;

    }

}

//        // release CPU
//        if (CPUActive && (releaseCPU == clock)) {
//            CPUActive = false;
//            cout << "6666666666666666" << endl;
//            cout << "Time " << clock << endl;
//            if (readyQ.size() > 0) {
//                readyQ.at(0)->time_for_next_interesting_event = clock + (t_cs / 2);
//                cout << "77777777777777" << endl;
//                cout << "Time " << clock << endl;
//                // context switch starting
//                releaseCPU = clock + (t_cs / 2);
//            }
//        }

#endif //CPUSCHEDULING_SJF_H
