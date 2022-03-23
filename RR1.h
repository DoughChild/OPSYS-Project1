#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <algorithm>
//#include "functions.h"

//#include "Process.h"



struct CompareProcess
{
    bool operator()(Process *p1, Process *p2)
    {
        /* this will compare the int cpu burst time of each process
         * it will be true if p1 has less time remaining
         */
        //cout << "calling processes: " << p1->name << " and " << p2->name << endl;
        if (p1->t_arrival != p2->t_arrival)
            return p1->t_arrival > p2->t_arrival;
        else
            return p1->name > p2->name;
    }
};


void printQu (deque<Process*> rq) {
    cout << "[Q ";
    if (rq.size() == 0) {
        cout << "empty]" << endl;
        return;
    }
    for (unsigned long i = 0; i < rq.size(); i++) {
        cout << rq[i]->name;
    }
    cout << "]" << endl;
}


void RR1(deque<Process *> processes, int t_cs, int t_slice)
{
    // deque<Process *> processes;
    // for (unsigned long i = 0; i < processesList.size(); i++) {
    //     Process *p = new Process();
    //     p->copy(processesList[i]);
    //     processes.push_back(p);
    // }

    // need these values to calculate averages
    //int num_processes = (int)processes.size();
    int num_CPU_bursts = 0, preempt_time = -1;
    int sum_wait = 0, sum_turnaround = 0, sum_burst_time = 0;
    double avg_wait, avg_turnaround, avg_burst, CPU_util;

    /* CPU process, current running process */
    Process *cur_process = NULL;
    /* Process that is mid context switch */
    Process *cswitch_process = NULL;
    /* list of I/O Waiting processes */
    deque<Process *> IO_q;
    /* priority queue of processes in ready queue */
    //priority_queue<Process, vector<Process *>, CompareProcess> ready_q;
    deque<Process*> ready_q;

    int cs_count = 0, preempt_count = 0, time_cur = 0;

    bool simulating = true, context_switching = false, printing = true;
    if (printing) {printf("time %dms: Simulator started for RR with time slice %dms [Q empty]\n", time_cur, t_slice);}
    while (simulating)
    {
        /* CPU Operations */
        if (context_switching)
        {
            // either finishing context switch IN or OUT of CPU
            // destination could be CPU, Ready queue, or I/O
            if (cswitch_process->cs_time_left == 0)
            {
                // switching process finished switch, and is heading to CPU
                if (cswitch_process->dest == 'c') {
                    if (printing) {
                        if (cswitch_process->preempted && (cswitch_process->CPUBursts.front() != cswitch_process->cur_CPUBurst)) {
                            printf("time %dms: Process %c started using the CPU for remaining %dms of %dms burst ", time_cur, cswitch_process->name, cswitch_process->CPUBursts.front(), cswitch_process->cur_CPUBurst);
                            cswitch_process->turnaround_time += t_cs;
                        } else {
                            printf("time %dms: Process %c started using the CPU for %dms burst ", time_cur, cswitch_process->name, cswitch_process->CPUBursts.front());
                            // increment turnaround time by cur_CPU_burst
                            cswitch_process->turnaround_time += cswitch_process->cur_CPUBurst;
                            cswitch_process->turnaround_time += t_cs;
                        }
                        printQu(ready_q);
                    }
                    preempt_time = time_cur + t_slice;
                    //printf("preempt time is %d\n\n", preempt_time);
                    cswitch_process->CPUBursts.front()--;
                    cswitch_process->preempted = false;
                    cur_process = cswitch_process;
                    cswitch_process = NULL;
                    context_switching = false;
                }
                // switching process has finished its CPU burst and is going to I/O
                else if (cswitch_process->dest == 'i') {
                    if (!cswitch_process->IOBursts.empty())
                    {
                        //printf("%d: Process %c is being sent to I/O with a burst time of %d\n", time_cur, cswitch_process->name, cswitch_process->IOBursts[0]);
                        IO_q.push_back(cswitch_process);
                        // skips ready queue, so no need to update its value
                    } else {
                        for (unsigned long i = 0; i < processes.size(); i++)
                        {
                            if (processes[i]->name == cswitch_process->name)
                            processes.erase(processes.begin() + i);
                        }
                    }
                    // after CPU burst finishes, add up to later calculate avgs
                    num_CPU_bursts++;
                    //printf("turnaround time was: time %dms\n", cswitch_process->turnaround_time);
                    sum_turnaround += cswitch_process->turnaround_time;
                    cswitch_process->turnaround_time = 0;
                    sum_wait += cswitch_process->wait_time;
                    cswitch_process->wait_time = 0;
                    // remove pointers
                    cur_process = NULL;
                    cswitch_process = NULL;
                    context_switching = false;
                }
                // switching process has finished its CPU burst and is going to the ready queue
                else if (cswitch_process->dest == 'r') {
                    // remove the CPU process that is switching out by adding it to the ready queue
                    cswitch_process->in_rq = true;

                    ready_q.push_back(cswitch_process);
                    //sort(ready_q.begin(), ready_q.end(), CompareProcess());
                    
                    // now change the process that preempted the CPU process to be switching
                    // and remove it from the ready queue

                    cswitch_process = ready_q.front();
                    ready_q.pop_front();
                    cswitch_process->in_rq = false;

                    cswitch_process->dest = 'c';
                    context_switching = true;
                    cswitch_process->cs_time_left = t_cs / 2;
                    cswitch_process->cs_time_left--;
                }
            } else {
                //printf("time %dms: waiting for context switch\n", time_cur);
                cswitch_process->cs_time_left--;
            }
        }
        // ensures the CPU is not empty
        else if (cur_process != NULL)
        {
            // If the current process is ending its burst, begin its exit from the CPU
            if (cur_process->CPUBursts.front() == 0)
            {
                // sum all CPU bursts
                sum_burst_time += cur_process->cur_CPUBurst;
                // remove burst time of 0, this makes next CPU burst the first in the queue
                cur_process->CPUBursts.pop_front();
                if (!cur_process->IOBursts.empty())
                {
                    if (printing) {
                        if (cur_process->CPUBursts.size() == 1) {
                            printf("time %dms: Process %c completed a CPU burst; %lu burst to go ", time_cur, cur_process->name, cur_process->CPUBursts.size());
                        } else {
                            printf("time %dms: Process %c completed a CPU burst; %lu bursts to go ", time_cur, cur_process->name, cur_process->CPUBursts.size());
                        }
                        printQu(ready_q);
                    }
                    cur_process->cur_CPUBurst = cur_process->CPUBursts.front();
                }
                context_switching = true;
                cur_process->cs_time_left = t_cs / 2;
                cur_process->cs_time_left--;
                //printf("cs time (should be 1 every time): %d\n", cur_process->cs_time_left);
                cur_process->dest = 'i';
                int block_til = cur_process->IOBursts.front() + time_cur + (t_cs / 2);
                if (!cur_process->IOBursts.empty()) {
                    if (printing) {
                        printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", time_cur, cur_process->name, block_til);
                        printQu(ready_q);
                    }
                } else {
                    printf("time %dms: Process %c terminated ", time_cur, cur_process->name);
                    printQu(ready_q);
                }
                cswitch_process = cur_process;
                cur_process = NULL;
                preempt_time = -1;
            } else {
                cur_process->CPUBursts.front()--;
            }
        }



        /* Waiting on I/O Operations */
        for (unsigned long i = 0; i < IO_q.size(); i++)
        {
            Process * waiting_process = IO_q[i];
            if (waiting_process->IOBursts.front() == 0)
            {
                //printf("I/O queue has size %lu\n", IO_q.size());
                if (printing) {
                    if (cur_process != NULL) {
                        if (time_cur == preempt_time) {
                            printf("time %dms: Process %c completed I/O; preempting %c ", time_cur, waiting_process->name, cur_process->name);
                            waiting_process->will_preempt = false;
                        } else {
                            printf("time %dms: Process %c completed I/O; added to ready queue ", time_cur, waiting_process->name);
                        }
                    } else if (cswitch_process != NULL) {
                        if (time_cur == preempt_time) {
                            waiting_process->will_preempt = true;
                        }
                        printf("time %dms: Process %c completed I/O; added to ready queue ", time_cur, waiting_process->name);
                    } else {
                        printf("time %dms: Process %c completed I/O; added to ready queue ", time_cur, waiting_process->name);
                    }
                }
                waiting_process->IOBursts.pop_front();
                waiting_process->in_rq = true;

                ready_q.push_back(waiting_process);
                //sort(ready_q.begin(), ready_q.end(), CompareProcess());
                if (printing) {
                    printQu(ready_q);   
                }

                // we only want to remove that i'th element
                IO_q.erase(IO_q.begin() + i);
                i--;
            } else {
                // for all processes staying in I/O
                waiting_process->IOBursts.front()--;
            }
        }



        /* Ready Queue Operations */
        for (unsigned long i = 0; i < processes.size(); i++)
        {
            // checks every process to see if its arrived at this tick
            Process * arriving_process = processes[i];
            if (arriving_process->t_arrival == time_cur)
            {
                arriving_process->in_rq = true;

                ready_q.push_back(arriving_process);
                //sort(ready_q.begin(), ready_q.end(), CompareProcess());

                if (printing) {
                    printf("time %dms: Process %c arrived; added to ready queue ", time_cur, arriving_process->name);
                    printQu(ready_q);
                }
            }
        }
        if ((!ready_q.empty()) && !context_switching)
        {
            if (cur_process == NULL)
            {
                cswitch_process = ready_q.front();
                ready_q.pop_front();

                cswitch_process->in_rq = false;
                context_switching = true;
                cswitch_process->cs_time_left = t_cs / 2;
                cswitch_process->cs_time_left--;
                cs_count++;
                cswitch_process->dest = 'c';
            }
            /** PREEMPT BLURB **/
            else if (time_cur == preempt_time)
            {
                Process * p = ready_q.front();
                if (p->will_preempt && printing) {
                    printf("time %dms: Process %c will preempt %c ", time_cur, p->name, cur_process->name);
                    printQu(ready_q);
                    p->will_preempt = false;
                }
                //sets the switching process to be headed for the ready queue.
                cur_process->preempted = true;
                cur_process->dest = 'r';
                cur_process->CPUBursts.front()++;
                if (printing) {               
                    printf("time %dms: Time slice expired; process %c preempted with %dms to go ", time_cur, cur_process->name, cur_process->CPUBursts.front());
                    printQu(ready_q);
                }
                cswitch_process = cur_process;
                cur_process = NULL;
                // we will generate a second context switch when this one is finished
                context_switching = true;
                cswitch_process->cs_time_left = t_cs / 2;
                cswitch_process->cs_time_left--;
                cs_count++;
                preempt_count++;
            }
        } else if (!context_switching) {
            if (cur_process != NULL && time_cur == preempt_time) {
                if (printing) {               
                    printf("time %dms: Time slice expired; no preemption because ready queue is empty ", time_cur);
                    printQu(ready_q);
                }
                preempt_time = time_cur + t_slice;
            }
        }

        // for all processes staying in the Ready Queue, use bool in_rq to check
        for (unsigned long i = 0; i < processes.size(); i++)
        {
            Process * ready_process = processes[i];
            if (ready_process->in_rq)
            {
                //printf("time %dms: Process %c is \"in rq\"\n", time_cur, ready_process->name);
                ready_process->wait_time++;
                ready_process->turnaround_time++;
            }
        }

        if (processes.empty())
        {
            printf("time %dms: Simulator ended for RR ", time_cur);
            printQu(ready_q);
            simulating = false;
        } else {
            // the most important single line of code
            time_cur++;
            if (time_cur > 1000) {//} && !DISPLAY_MAX_T) {
                printing = false;
            }
        }
    }
    // calculating average wait and turnaround times
    avg_burst = ceil(((double)sum_burst_time / (double)num_CPU_bursts) * 1000) / 1000;
    avg_wait = ceil(((double)sum_wait / (double)num_CPU_bursts) * 1000) / 1000;
    avg_turnaround = ceil(((double)sum_turnaround / (double)num_CPU_bursts) * 1000) / 1000;
    CPU_util = ceil(((double)sum_burst_time / (double)time_cur) * 100000) / 1000;

    FILE * file;
    file = fopen ("simout.txt", "a");
    fprintf(file, "Algorithm SRT\n");
    fprintf(file, "-- average CPU burst time: %.3f ms\n", avg_burst);
    fprintf(file, "-- average wait time: %.3f ms\n", avg_wait);
    fprintf(file, "-- average turnaround time: %.3f ms\n", avg_turnaround);
    fprintf(file, "-- total number of context switches: %d\n", cs_count);
    fprintf(file, "-- total number of preemptions: %d\n", preempt_count);
    fprintf(file, "-- CPU utilization: %.3f%%\n", CPU_util);
    fclose( file );


}