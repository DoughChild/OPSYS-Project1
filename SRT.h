#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <queue>

#include "Process.h"
#include "tau_calc.h"

struct CompareProcess
{
    // had Process const &p1... before
    bool operator()(Process *p1, Process *p2)
    {
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
void remove_process(deque<Process *> processes, char name)
{
    for (unsigned long i = 0; i < processes.size(); i++)
    {
        if (processes[i]->name == name)
            processes.erase(processes.begin() + i);
    }
}

void SRT(deque<Process *> processes, double tau, int t_cs, double alpha)
{
    /* before popping values from processes in the list, measure
     * CPU utilization from unaltered list. This will prevent us
     * from having to use additional variables etc */

    /* CPU process, current running process */
    Process *cur_process = NULL;
    /* list of I/O Waiting processes */
    deque<Process *> IO_q;
    /* list of processes switching out of CPU into the ready queue after a preemption */
    deque<Process *> rq_q;
    /* priority queue of processes in ready queue */
    priority_queue<Process, vector<Process *>, CompareProcess> ready_q;

    int cs_time = 0, cs_count = 0, time_cur = 0;

    bool simulating = true;
    cout << "about to begin simulation\n";
    while (simulating)
    {
        /* CPU Operations */
        // if we are context switching
        if (cs_time != 0)
        {
            cout << "context switching!\n";
            // decrement context switch count before we can use CPU again
            cs_time--;
            // adds time to current process
            cur_process->turnaround_time++;
            // checks if process exiting CPU to see if / when it will exit
            for (unsigned long i = 0; i < rq_q.size(); i++)
            {
                Process * switching_process = rq_q[i];
                if (switching_process->cs_time_left == 0)
                {
                    ready_q.push(switching_process);
                    switching_process->in_rq = true;
                } else {
                    switching_process->cs_time_left--;
                    switching_process->turnaround_time++;
                }
            }
        }
        else if (processes.size() == 0)
        {
            cout << "simulation is about to end\n";
            simulating = false;
        }
        // ensures the CPU is not empty
        else if (cur_process != NULL)
        {
            // If the current process is ending its burst, begin its exit from the CPU
            if (cur_process->CPUBursts.front() == 0)
            {
                cout << "process is switching out of CPU naturally\n";
                cs_time = t_cs / 2;
                // remove burst time of 0, this makes next CPU burst the first in the queue
                cur_process->CPUBursts.pop_front();
                cs_count++;
                // recalculate and reset tau
                cur_process->tau = calc_tau(cur_process->tau, cur_process->cur_CPUBurst, alpha);
                cur_process->tau_remaining = cur_process->tau;
                // check if the process is heading to I/O, otherwise process is ending
                if (cur_process->IOBursts.size() > 0)
                {
                    IO_q.push_back(cur_process);
                    cur_process->IOBursts[0] += (t_cs / 2);
                } else {
                    remove_process(processes, cur_process->name);   
                }
                cs_time--;
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
            Process * waiting_process = IO_q[i];
            if (waiting_process->IOBursts[0] == 0)
            {
                ready_q.push(waiting_process);
                waiting_process->in_rq = true;
                // we only want to remove that i'th element
                IO_q.erase(IO_q.begin() + i);
            } else {
                // for all processes staying in I/O
                waiting_process->IOBursts[0]--;
                waiting_process->turnaround_time++;
            }
        }


        /* Ready Queue Operations */
        for (unsigned long i = 0; i < processes.size(); i++)
        {
            // checks every process to see if its arrived at this tick
            if (processes[i]->t_arrival == time_cur)
            {
                Process * arriving_process = processes[i];
                arriving_process->in_rq = true;
                ready_q.push(arriving_process);
                cout << "new process has arrived!\n";
            }
        }
        if (!ready_q.empty())
        {
            if (cur_process == NULL)
            {
                cur_process = ready_q.top();
                ready_q.pop();
                cur_process->in_rq = false;
                cs_time = t_cs / 2;
                cs_time--;
                cs_count++;
            }
            /* check if first process can preempt by comparing remaining tau for each process
            * preemption should also only occur when there is no context switch occuring
            * If the estimate (tau) of the current process was too short, we won't ever preempt */
            else if ((ready_q.top()->tau_remaining < cur_process->tau_remaining) && cs_time == 0)
            {
                /* moves current process out to wait for its context
                * switch to end before going back to the ready queue */
                rq_q.push_back(cur_process);
                // moves top process of the ready queue into the CPU
                cur_process = ready_q.top();
                ready_q.pop();
                cur_process->in_rq = false;
            }
            // add else for clarity?
        }

        // for all processes staying in the Ready Queue, use bool in_rq to check
        for (unsigned long i = 0; i < processes.size(); i++)
        {
            Process * ready_process = processes[i];
            if (ready_process->in_rq)
            {
                ready_process->wait_time++;
                ready_process->turnaround_time++;
            }
        }

        // the most important single line of code
        time_cur++;
    }
}


// notes: if there is only 1 process it seg faults every time