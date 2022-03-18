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

// struct CompareProcess
// {
//     bool operator()(Process const p1, Process const p2)
//     {
//         /* this will compare the int cpu burst time of each process
//          * it will be true if p1 has less time remaining
//          * may change bool return to int to give more options for tie-breaking
//          */
//         cout << "calling processes: " << p1->name << " and " << p2->name << endl;
//         if (p1->CPUBursts.front() != p2->CPUBursts.front())
//             return p1->CPUBursts.front() < p2->CPUBursts.front();
//         else
//             return p1->name < p2->name;
//     }
// };

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
            // either finishing context switch IN or OUT of CPU
            // destination could be CPU, Ready queue, or even I/O
            if (cswitch_process->cs_time_left == 0)
            {
                // switching process finished switch, and is heading to CPU
                if (cswitch_process->dest == 'c') {
                    // need to account for turnaround time when switching
                    cswitch_process->turnaround_time++;
                    cur_process = cswitch_process;
                    // can only have come from ready queue
                    cswitch_process->in_rq = false;
                    cswitch_process = NULL;
                }
                // switching process has finished its CPU burst and is going to I/O
                else if (cswitch_process->dest == 'i') {
                    if (!cswitch_process->IOBursts.empty())
                    {
                        printf("%d: Process %c is being sent to I/O with a burst time of %d\n", time_cur, cur_process->name, cur_process->IOBursts[0]);
                        IO_q.push_back(cswitch_process);
                        // skips ready queue, so no need to update its value
                    } else {
                        printf("%d: Process %c is on its way out\n", time_cur, cswitch_process->name);
                        for (unsigned long i = 0; i < processes.size(); i++)
                        {
                            if (processes[i]->name == cswitch_process->name)
                            processes.erase(processes.begin() + i);
                        }
                    }
                    cur_process = NULL;
                    cswitch_process = NULL;
                }
                // switching process has finished its CPU burst and is going to the ready queue
                else if (cswitch_process->dest == 'r') {
                    printf("cswitch process %c is leaving CPU\n", cswitch_process->name);
                    printf("top of ready queue is %c\n", ready_q.top()->name);
                    cswitch_process->in_rq = true;

                    printf("%d: (CPU) size of ready queue: %lu\n", time_cur, ready_q.size());
                    ready_q.push(cswitch_process);
                    printf("size of ready queue after push: %lu\n", ready_q.size());


                    printf("top of ready queue is %c\n", ready_q.top()->name);
                    cswitch_process = ready_q.top();
                    printf("read queue size %lu\n", ready_q.size());
                    ready_q.pop();
                    printf("ready queue empty? %d\n", ready_q.empty());
                    printf("top of ready queue is %c\n", ready_q.top()->name);
                    printf("%c has %d time left on the current CPU burst\n", ready_q.top()->name, ready_q.top()->CPUBursts.front());
                    ready_q.pop();
                    printf("read queue size %lu\n", ready_q.size());


                    cs_time = t_cs / 2;
                    cs_time--;
                    cs_count++;
                } else {
                    printf("Major Error occurred\n\n\n\n\n\n\n\n\n");
                }
            } else {
                cswitch_process->cs_time_left--;
            }

            // decrement context switch count before we can use CPU again
            cs_time--;
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
                if (!cur_process->IOBursts.empty())
                {
                    // recalculate and reset tau
                    printf("%d: Process %c finished CPU Burst. Actual time: %d, Alpha: %f", time_cur, cur_process->name, cur_process->cur_CPUBurst, alpha);
                    cur_process->tau = calc_tau(cur_process->tau, cur_process->cur_CPUBurst, alpha);
                    cur_process->tau_remaining = cur_process->tau;
                    printf(", new tau is: %d\n", cur_process->tau);
                    cur_process->cur_CPUBurst = cur_process->CPUBursts.front();
                }
                cs_time--;
                cur_process->dest = 'i';
                cswitch_process = cur_process;
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
            if (waiting_process->IOBursts.front() == 0)
            {
                waiting_process->IOBursts.pop_front();

                printf("%d: (I/O) size of ready queue: %lu\n", time_cur, ready_q.size());
                ready_q.push(waiting_process);
                printf("size of ready queue after push: %lu\n", ready_q.size());

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
                printf("%d: (RQ) size of ready queue: %lu\n", time_cur, ready_q.size());
                ready_q.push(arriving_process);
                printf("size of ready queue after push: %lu\n", ready_q.size());
                cout << "new process has arrived!\n";
            }
        }
        if ((!ready_q.empty()) && (cs_time == 0))
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
                cur_process->dest = 'c';
                cswitch_process = cur_process;
            }
            /* check if first process can preempt by comparing remaining tau for each process
            * preemption should also only occur when there is no context switch occuring
            * If the estimate (tau) of the current process was too short, we won't ever preempt */
            else if (ready_q.top()->tau_remaining < cur_process->tau_remaining)
            {
                printf("%d: Preempting\n", time_cur);
                printf("top of ready queue (%c) remaining tau time: %d\n", ready_q.top()->name, ready_q.top()->tau_remaining);
                printf("current process (%c) remaining tau time: %d\n", cur_process->name, cur_process->tau_remaining);

                //sets the switching process to be headed for the ready queue.
                cur_process->dest = 'r';
                cswitch_process = cur_process;
                cur_process = NULL;
                // we will generate a second context switch when this one is finished
                cs_time = t_cs / 2;
                cs_time--;
                cs_count++;
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
        // usleep(1000);
        // printf("%d: ready_q.size() is %lu\n", time_cur, ready_q.size());

        // if (time_cur % 100 == 0)
        // {
        //     printf("%d: %c in ready queue? %d\n", time_cur, processes[1]->name, processes[1]->in_rq);
        // }

        // the most important single line of code
        time_cur++;
    }
    printf("time %d\n", time_cur);
    printf("number of context switches: %d\n", cs_count);
}



//check calculation of tau, as in check the notes
// after making sure the actual math is correct, check the code.