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
        cout << "calling processes: " << p1->name << " and " << p2->name << endl;
        if (p1->CPUBursts.front() != p2->CPUBursts.front())
            return p1->CPUBursts.front() < p2->CPUBursts.front();
        else
            return p1->name < p2->name;
    }
};

/* Removes a process from the list of processes using their name
 * This is to keep a track of processes that have yet to terminate
 */
// void remove_process(deque<Process *> processes, char name)
// {
//     unsigned long len = processes.size();
//     for (unsigned long i = 0; i < len; i++)
//     {
//         if (processes[i]->name == name)
//             processes.erase(processes.begin() + i);
//     }
// }

void SRT(deque<Process *> processes, double tau, int t_cs, double alpha)
{
    /* before popping values from processes in the list, measure
     * CPU utilization from unaltered list. This will prevent us
     * from having to use additional variables etc */

    /* CPU process, current running process */
    Process *cur_process = NULL;
    /* Process that is mid context switch */
    Process *cswitch_process = NULL;
    /* list of I/O Waiting processes */
    deque<Process *> IO_q;
    /* priority queue of processes in ready queue */
    priority_queue<Process, vector<Process *>, CompareProcess> ready_q;

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
            cswitch_process->turnaround_time++;
            if (cswitch_process->cs_time_left == 0)
            {
                if (!cswitch_process->IOBursts.empty())
                {
                    printf("I/O Burst is: %d\n", cswitch_process->IOBursts[0]);
                    printf("%d: Process %c is switching out of the CPU and into the ready queue\n", time_cur, cswitch_process->name);
                    ready_q.push(cswitch_process);
                    cswitch_process->in_rq = true;
                    cswitch_process = NULL;
                } else {
                    printf("%d: Process %c is on its way out\n", time_cur, cur_process->name);
                    printf("context switch time is %d\n", cs_time);
                    // remove_process(processes, cur_process->name);
                    unsigned long len = processes.size();
                    for (unsigned long i = 0; i < len; i++)
                    {
                        if (processes[i]->name == cur_process->name)
                            processes.erase(processes.begin() + i);
                    }
                }
            } else {
                cswitch_process->cs_time_left--;
            }
        }
        else if (processes.empty())
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
                cs_time = t_cs / 2;
                // remove burst time of 0, this makes next CPU burst the first in the queue
                cur_process->CPUBursts.pop_front();
                cs_count++;
                // recalculate and reset tau
                printf("%d: Process %c has finished their CPU burst, and has tau of %d, tau remaining of %d\n", time_cur, cur_process->name, cur_process->tau, cur_process->tau_remaining);
                printf("Actual CPU burst time: %d, Alpha: %f", cur_process->cur_CPUBurst, alpha);
                cur_process->tau = calc_tau(cur_process->tau, cur_process->cur_CPUBurst, alpha);
                cur_process->tau_remaining = cur_process->tau;
                printf(", new tau is: %d\n", cur_process->tau);
                cur_process->cur_CPUBurst = cur_process->CPUBursts.front();
                // check if the process is heading to I/O, otherwise process is ending
                if (!cur_process->IOBursts.empty())
                {
                    printf("%d: Process %c is going to I/O\n", time_cur, cur_process->name);
                    IO_q.push_back(cur_process);
                    cur_process->IOBursts[0] += (t_cs / 2);
                } //else {
                // printf("%d: Process %c is on its way out\n", time_cur, cur_process->name);
                // printf("context switch time is %d\n", cs_time);
                // // remove_process(processes, cur_process->name);
                // unsigned long len = processes.size();
                // for (unsigned long i = 0; i < len; i++)
                // {
                //     if (processes[i]->name == cur_process->name)
                //         processes.erase(processes.begin() + i);
                // }
                //}
                cs_time--;
                cswitch_process = cur_process;
                cur_process = NULL;
            } else {
                //cout << "cpu burst decreasing\n";
                cur_process->CPUBursts[0]--;
                cur_process->tau_remaining--;
                cur_process->turnaround_time++;
            }
        }


        /* Waiting on I/O Operations */
        for (unsigned long i = 0; i < IO_q.size(); i++)
        {
            Process * waiting_process = IO_q[i];
            //printf("%d: Process %c has %d ms left in I/O\n", time_cur, waiting_process->name, waiting_process->IOBursts[0]);
            if (waiting_process->IOBursts.front() == 0)
            {
                printf("Processes I/O burst size: %lu\n", waiting_process->IOBursts.size());
                waiting_process->IOBursts.pop_front();
                printf("Processes I/O burst size after pop: %lu\n", waiting_process->IOBursts.size());
                ready_q.push(waiting_process);
                waiting_process->in_rq = true;
                printf("length of IO_q: %lu\n", IO_q.size());
                // we only want to remove that i'th element
                IO_q.erase(IO_q.begin() + i);
                printf("length of IO_q after: %lu\n", IO_q.size());
            } else {
                // for all processes staying in I/O
                waiting_process->IOBursts[0]--;
                waiting_process->turnaround_time++;
                //printf("I/O queue size is: %ld\n", IO_q.size());
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
                printf("%d: Process %c is arriving with %d bursts\n", time_cur, arriving_process->name, arriving_process->num_bursts);
            }
        }
        if (!ready_q.empty())
        {
            if (cur_process == NULL)
            {
                cur_process = ready_q.top();
                printf("%d: Process %c is being sent to the CPU with a burst time of %d\n", time_cur, cur_process->name, cur_process->CPUBursts[0]);
                ready_q.pop();
                cur_process->in_rq = false;
                cs_time = t_cs / 2;
                cs_time--;
                cs_count++;
                cswitch_process = cur_process;
            }
                /* check if first process can preempt by comparing remaining tau for each process
                * preemption should also only occur when there is no context switch occuring
                * If the estimate (tau) of the current process was too short, we won't ever preempt */
            else if ((ready_q.top()->tau_remaining < cur_process->tau_remaining) && cs_time == 0)
            {
                cout << "preempting\n";
                printf("%d: Process %c has a tau of %d, and tau remaining of %d\n", time_cur, ready_q.top()->name, ready_q.top()->tau, ready_q.top()->tau_remaining);
                printf("%d: Process %c has a tau of %d, and tau remaining of %d\n", time_cur, cur_process->name, cur_process->tau, cur_process->tau_remaining);
                // moves top process of the ready queue into the CPU
                cur_process = ready_q.top();
                ready_q.pop();
                cur_process->in_rq = false;
                cs_time = t_cs;
                cs_time--;
                // TODO: maybe change this value to += 2 to represent the two switches?
                cs_count++;
                cswitch_process = cur_process;
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

        printf("length of IO_q: %lu\n", IO_q.size());


        // the most important single line of code
        time_cur++;
    }
}



//check calculation of tau, as in check the notes
// after making sure the actual math is correct, check the code.