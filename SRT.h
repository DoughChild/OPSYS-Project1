#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <queue>

#include "Process.h"
#include "tau_calc.h"

struct CompareProcess {
    // had Process const &p1... before
    bool operator()(Process *p1, Process *p2) {
        /* this will compare the int cpu burst time of each process
         * it will be true if p1 has less time remaining
         * may change bool return to int to give more options for tie-breaking
         */
        cout << p1->name << endl;
        cout << p2->name << endl;
        if (p1->CPUBursts.front() != p2->CPUBursts.front())
            return p1->CPUBursts.front() < p2->CPUBursts.front();
        else
            return p1->name < p2->name;
    }
};


struct CompareName {
    // compares process names. This is for ties within ready queue, for example
    bool operator()(Process *p1, Process *p2) {
        cout << p1->name << endl;
        cout << p2->name << endl;
        return p1->name < p2->name;
    }
};


/* If current CPU Burst length is 0, and it is by default, 
 * we will update it with the actual current burst.
 * Else, it is re-entering the CPU from preemption and the
 * value should not be updated
 */
void update_cur_CPUBurst(Process *process)
{
    if (process->cur_CPUBurst == 0)
        process->cur_CPUBurst = process->CPUBursts.front();
}

/* Removes a process from the list of processes using their name
 * This is to keep a track of processes that have yet to terminate
 */
void remove_process(deque<Process*> processes, char name)
{
    for (unsigned long i = 0; i < processes.size(); i++) {
        if (processes[i]->name == name)
            processes.erase(processes.begin()+i);
    }
}



void SRT(deque<Process*> processes, double tau, int t_cs, double alpha)
{
    /* before popping values from processes in the list, measure
     * CPU utilization from unaltered list. This will prevent us
     * from having to use additional variables etc */



    /* CPU process, current running process */
    Process * cur_process = NULL;
    /* Temp process to hold I/O processes */
    Process * waiting_process = NULL;
    /* Temp process to hold arriving processes */
    Process * arriving_process = NULL;
    /* Temp process to hold processes who were preempted, and are mid context switch */
    Process * switching_process = NULL;
    /* list of I/O Waiting processes */
    deque<Process*> IO_q;
    /* list of processes switching out of CPU into the ready queue after a preemption */
    deque<Process*> rq_q;
    /* priority queue of processes in ready queue */
    priority_queue<Process, vector<Process*>, CompareProcess> ready_q;

    int cs_time = 0, cs_count = 0, time_cur = 0;

    bool simulating = true;
    while (simulating)
    {
        /* CPU Operations */
        // if we are context switching
        if (cs_time != 0)
        {
            // decrement context switch count before we can use CPU again
            cs_time--;
            // adds time to current process
            cur_process->turnaround_time++;
            // checks if process exiting CPU to see if / when it will exit
            for (unsigned long i = 0; i < rq_q.size(); i++)
            {
                switching_process = rq_q[i];
                if (switching_process->cs_time_left == 0) {
                    ready_q.push(switching_process);
                } else {
                    switching_process->cs_time_left--;
                    switching_process->turnaround_time++;
                }
            }
        } else {
            // check if this is necessary. Do we need to cover this within CPU? The ready queue should do this...
            if (cur_process == NULL)
            {
                if (!ready_q.empty())
                {
                    cs_time = t_cs / 2;
                    cur_process = ready_q.top();
                    ready_q.pop();
                    update_cur_CPUBurst(cur_process);
                    cs_time--;
                    cur_process->turnaround_time++;
                    cs_count++;
                } else {
                    // what is the behavior here
                    continue;
                }
            } else {
                // If the current process is ending its burst, begin its exit from the CPU
                if (cur_process->CPUBursts.front() == 0)
                {
                    cs_time = t_cs / 2;
                    cur_process->CPUBursts.pop_front();
                    cs_count++;
                    cur_process->tau = calc_tau(cur_process->tau, cur_process->cur_CPUBurst, alpha);
                    cur_process->tau_remaining = cur_process->tau;
                    if (cur_process->IOBursts.size() != 0)
                    {
                        IO_q.push_back(cur_process);
                        cur_process->IOBursts[0] += (t_cs / 2);
                    } else {
                        remove_process(processes, cur_process->name);
                        if (processes.size() == 0)
                            simulating = false;
                        else {
                            // some behavior?
                            continue;
                        }
                        cs_time--;
                    }
                    cur_process->cur_CPUBurst = 0;
                    cur_process = NULL;
                } else {
                    cur_process->CPUBursts[0]--;
                    cur_process->tau_remaining--;
                    cur_process->turnaround_time++;
                }
            }

        /* Waiting on I/O Operations */
        for (unsigned long i = 0; i < IO_q.size(); i++)
        {
            // may cause trouble when there are no objects in waiting queue
            priority_queue<Process, vector<Process*>, CompareName> CPU_q;
            bool move_to_cpu = false;
            if (IO_q[i]->IOBursts[0] == 0)
            {
                waiting_process = IO_q[i];
                IO_q.pop_front();
                // this should be tie-broken, may result it wrong process being inserted
                // maybe make secondary priority queue...
                if (cur_process == NULL)
                {
                    cur_process = waiting_process;
                    cs_time = t_cs / 2;
                    cs_time--;
                    cs_count++;
                } else {
                    ready_q.push(waiting_process);
                }
            }
            // for all processes staying in I/O
            waiting_process->IOBursts[0]--;
            waiting_process->turnaround_time++;
        }

        /* Ready Queue Operations */
        for (unsigned long i = 0; i < processes.size(); i++)
        {
            // checks every process to see if its arrived at this tick
            priority_queue<Process, vector<Process*>, CompareName> CPU_q;
            bool move_to_cpu = false;
            if (processes[i]->t_arrival == time_cur)
            {
                arriving_process = processes[i];
                // this should be tie-broken, may result it wrong process being inserted
                // maybe make secondary priority queue...
                if (cur_process == NULL)
                {
                    CPU_q.push(arriving_process);
                    move_to_cpu = true;
                    // cur_process = arriving_process;
                    // cs_time = t_cs / 2;
                    // cs_time--;
                    // cs_count++;
                } else {
                    ready_q.push(arriving_process);
                }
            }
            if (!CPU_q.empty() && (move_to_cpu == true))
            {
                cur_process = CPU_q.top();
                CPU_q.pop();
                cs_time = t_cs / 2;
                cs_time--;
                cs_count++;
                move_to_cpu = false;
            }

            /* check if first process can preempt by comparing remaining tau for each process
             * preemption should also only occur when there is no context switch occuring
             * If the estimate (tau) of the current process was too short, we won't ever preempt */
            if ( (ready_q.top()->tau_remaining < cur_process->tau_remaining) && cs_time == 0)
            {
                /* moves current process out to wait for its context
                 * switch to end before going back to the ready queue */
                rq_q.push_back(cur_process);
                // moves top process of the ready queue into the CPU
                cur_process = ready_q.top();
                ready_q.pop();
            }

            // for all processes staying in the Ready Queue
            arriving_process->wait_time++;
            arriving_process->turnaround_time++;
        }

            time_cur++;
        }

    }

}
