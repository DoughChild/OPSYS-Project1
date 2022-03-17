#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <queue>
#include <vector>
#include "Process.h"

bool sort_by_arrival(Process* p1, Process* p2)
{
    return (this->t_arrival < p1->t_arrival);
}

bool sort_by_name(Process* p1, Process* p2)
{
    return (this->name < p1->name);
}

void FCFS(deque<Process*> processes, int t_cs)
{
    //cpu time in ms
    int cpu_t = 0;
    Process* running == NULL;
    deque<Process*> ready_queue;
    std::vector<Process*> waiting_queue;
    std::vector<Process*> finished_IO;

    //loop through all processes and set their cur_CPUBurst and cur_IOBurst to 0
    for(unsigned int i = 0; i < processes.size(); i++){
        processes[i]->cur_CPUBurst = processes[i]->CPUBursts.front();
        processes[i]->CPUBursts.pop_front();
        processes[i]->cur_CPUBurst = processes[i]->CPUBursts.front();
        processes[i]->IOBursts.pop_front();
    }

    //sort process deque
    sort(processes.begin(), processes.end(), sort_by_arrival)

    //loop until process queue, waiting queue, running process, and ready queue are all empty
    while(!processes.empty() || !ready_queue.empty() || !waiting_queue.empty() || running != NULL;){
        //decrement running cpu burst's timer, if it is decremted to 0 replace with next burst time from the burst dequeue
        //then add proccess to waiting queue for I/O
        if(running != NULL && !running->in_cs && !running->out_cs){
            running->cur_CPUBurst--;
            if(running->cur_CPUBurst == 0){
                running->cur_CPUBurst = running->CPUBursts.front();
                running->CPUBursts.pop_front();
                running->out_cs = true;
                running->cs_time_left = t_cs;
            }
        }
        //allow running to begin cpu burst once context switch has finished for starting cpu burst
        else if(running->in_cs){
            running->cs_time_left--;
            if(running->cs_time_left == 0){
                running->in_cs = false;
            }
        }
        //set running equal to NULL once context switch has finished for process ending cpu burst
        else if(running->out_cs){
            running->cs_time_left--;
            if(running->cs_time_left == 0){
                running->out_cs = false;
                waiting_queue.push_back(running);
                running = NULL;
            }
        }

        //loop through waiting_queue and decrement all I/0 burst timers
        //check all waiting_queue I/O burst timers. If they are 0 replace with next burst time from the burst dequeue
        //remove process from waiting queue and add them to finished I/O vector
        for(unsigned int i = 0; i < waiting_queue.size(); i++){
            waiting_queue[i]->cur_IOBurst--;
            if(waiting_queue[i]->cur_IOBurst == 0){
                waiting_queue[i]->cur_IOBurst = waiting_queue[i]->IOBursts.front();
                waiting_queue[i]->IOBursts.pop_front();
                finished_IO.push_back(waiting_queue[i]);
                waiting_queue.remove(waiting_queue.begin()+i);
            }
        }

        //if finished I/O vector is more than one element add to ready_queue in alphabetical order, otherwise just add the one
        //element to the ready queue
        finished_IO.sort(finished_IO.begin(), finished_IO.end(), sort_by_name);
        while(finished_IO.size() > 0){
            ready_queue.push_back(finished_IO[0]);
            finished_IO.pop_front();
        }

        //check if curent time is equal to next process arrival time. if it is equal to current time, add it to the ready_queue
        //with front and remove it from process_queue with pop_front. Make sure to keep using front to check for process's 
        //arriving at the same time
        while(processes.front()->t_arrival == cput_t){
            ready_queue.push_back(processes.front());
            processes.pop_front();
        }

        //assign next running process from the front of the ready queue and pop it if running is NULL
        if(running == NULL){
            running = ready_queue.front();
            ready_queue.pop_front();
            running->in_cs = true;
            running->cs_time_left = t_cs;
        }
        
        //increment cpu_t
        cpu_t++;
    }
}