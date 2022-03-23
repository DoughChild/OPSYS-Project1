//
// Created by Iffat Nafisa on 3/20/22.
//

#ifndef CPUSCHEDULING_FUNCTIONS_H
#define CPUSCHEDULING_FUNCTIONS_H
#include <iostream>

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

void updateOnCPUEntry(Process *p, int t, int t_cs) {
    double waittime = t - p->arrived_readyQ - double(t_cs / 2.0);
    p->wait_time += waittime;
    p->cur_CPUBurst = p->CPUBursts[0];
    p->cs_time_left = p->cur_CPUBurst;
    p->CPUBursts.erase(p->CPUBursts.begin());
    p->time_for_next_interesting_event = t + p->cur_CPUBurst;
    // cout << "UPDATE on CPU BURST " << p->time_for_next_interesting_event << endl;
}

void updateOnSwitchOutOfCPU(Process *p, int t, int t_cs) {
    p->time_for_next_interesting_event = t + (t_cs / 2) + p->IOBursts[0];
    p->IOBursts.erase(p->IOBursts.begin());
}

int calc_tau(double prev_tau, int actual, double alpha)
{
    /* recalculate tau based on previous tau, alpha, and actual time */

    double new_tau = ceil((alpha * actual) + ((1.0 - alpha) * prev_tau));
    return (int)new_tau;
}

double calculateAverageCPUBurst(Process * p) {
    double total = 0.0;
    for(int i = 0; i < p->CPUBursts.size(); i++) {
        total += double(p->CPUBursts[i]);
    }
    //cout << total/p->num_bursts << endl;
    return total;
}
#endif //CPUSCHEDULING_FUNCTIONS_H
