#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>

bool sort_by_arrival(Process* p1, Process* p2)
{
    return (p2->t_arrival < p1->t_arrival);
}

bool sort_by_name(Process* p1, Process* p2)
{
    return (p2->name < p1->name);
}

void FCFS(deque<Process*> processes, int t_cs)
{
    //cpu time in ms
    int cpu_t = 0;
    int cs_switches = 1;
    int cpu_time = 0;
    int cpu_bursts = 0;
    int cpu_usage = 0;
    int cpu_idle = 0;
    int turn_time = 0;
    int wait_time = 0;
    int preepmtions = 0;

    Process* running = NULL;
    deque<Process*> ready_queue;
    std::vector<Process*> waiting_queue;
    std::vector<Process*> finished_IO;

    //loop through all processes and set their cur_CPUBurst and cur_IOBurst to 0
    for(unsigned int i = 0; i < processes.size(); i++){
        processes[i]->cur_CPUBurst = processes[i]->CPUBursts.front();
        processes[i]->CPUBursts.pop_front();
        processes[i]->cur_IOBurst = processes[i]->IOBursts.front();
        processes[i]->IOBursts.pop_front();
    }

    //sort process deque
    //std::cout << "Sorting by arrival" << std::endl;
    sort(processes.begin(), processes.end(), sort_by_arrival);

    //loop until process queue, waiting queue, running process, and ready queue are all empty
    while(!processes.empty() || !ready_queue.empty() || !waiting_queue.empty() || running != nullptr){
        //decrement running cpu burst's timer, if it is decremted to 0 replace with next burst time from the burst dequeue
        //then add proccess to waiting queue for I/O
        if(running != nullptr){
            if(!running->in_cs && !running->out_cs){
                //std::cout << "Decrementing running process cpu burst timer: " << running->name << std::endl;
                cpu_time++;
                running->cur_CPUBurst--;
                if(running->cur_CPUBurst == 0){
                    //std::cout << "CPU burst is done, going to context switch: " << running->name << std::endl;
                    cpu_bursts++;
                    if(running->CPUBursts.size() > 0){
                        running->cur_CPUBurst = running->CPUBursts.front();
                        running->CPUBursts.pop_front();
                    }
                    running->out_cs = true;
                    running->cs_time_left = t_cs/2;
                }
            }
            //allow running to begin cpu burst once context switch has finished for starting cpu burst
            else if(running->in_cs){
                running->cs_time_left--;
                if(running->cs_time_left == 0){
                    //std::cout << "Context switch is done, starting CPU burst: " << running->name << std::endl;
                    running->in_cs = false;
                }
            }
            //set running equal to NULL once context switch has finished for process ending cpu burst
            else if(running->out_cs){
                running->cs_time_left--;
                if(running->cs_time_left == 0){
                    //std::cout << "Context switch is done, starting IO burst: " << running->name << std::endl;
                    cs_switches++;
                    running->out_cs = false;
                    waiting_queue.push_back(running);
                    running = NULL;
                }
            }
        }
        //loop through waiting_queue and decrement all I/0 burst timers
        //check all waiting_queue I/O burst timers. If they are 0 replace with next burst time from the burst dequeue
        //remove process from waiting queue and add them to finished I/O vector
        for(std::vector<Process*>::iterator it = waiting_queue.begin(); it != waiting_queue.end(); ++it){
            if(it == waiting_queue.begin()){
                //std::cout << "Decrementing IO burst timers for those in waiting queue" << std::endl;
            }
            (*it)->cur_IOBurst--;
            if((*it)->cur_IOBurst == 0){
                //std::cout << "Process has finished IO burst: " << (*it)->name << std::endl;
                if((*it)->IOBursts.size() > 0){
                    (*it)->cur_IOBurst = (*it)->IOBursts.front();
                    (*it)->IOBursts.pop_front();
                }
                finished_IO.push_back((*it));
                it = waiting_queue.erase(it);
                --it;
            }
        }

        //if finished I/O vector is more than one element add to ready_queue in alphabetical order, otherwise just add the one
        //element to the ready queue
        if(finished_IO.size() > 0){
            //std::cout << "Sorting wait queue by name" << std::endl;
            sort(finished_IO.begin(), finished_IO.end(), sort_by_name);
        }
        while(finished_IO.size() > 0){
            //std::cout << "Process finished IO burst: " << finished_IO[0]->name << std::endl;
            if(finished_IO[0]->CPUBursts.size() == 0){
                finished_IO.erase(finished_IO.begin());
                break;
            }
            ready_queue.push_back(finished_IO[0]);
            finished_IO.erase(finished_IO.begin());
        }

        //check that there are processes left on the queue
        //check if curent time is equal to next process arrival time. if it is equal to current time, add it to the ready_queue
        //with front and remove it from process_queue with pop_front. Make sure to keep using front to check for process's 
        //arriving at the same time
        //std::cout << "Checking for process arrivals" << std::endl;
        if(processes.size() > 0){
            while(processes.size() > 0){
                if(processes.front()->t_arrival == cpu_t){
                    //std::cout << "Process arrived at time " << cpu_t << ": " << processes.front()->name << std::endl;
                    ready_queue.push_back(processes.front());
                    processes.pop_front();
                }
                else{
                    break;
                }
            }
        }
        //check if there is a next running process
        //assign next running process from the front of the ready queue and pop it if running is NULL
        if(running == nullptr && ready_queue.size() > 0){
            running = ready_queue.front();
            ready_queue.pop_front();
            running->in_cs = true;
            running->cs_time_left = t_cs/2;
            //std::cout << "Assigning new running process: " << running->name << std::endl;
        }

        //std::cout << "Time is: " << cpu_t << std::endl;
        //increment cpu_t
        cpu_t++;
    }
    //Statistics calculations
    std::cout << fixed;
    std::cout.precision(3);
    float average_burst_time = cpu_time/cpu_bursts;
    float average_wait_time = 0.0f;
    float cpu_util = cpu_time/cpu_t;
    std::cout << "Algorith FCFS" << std::endl;
    std::cout << "-- average CPU burst time: " << average_burst_time << " ms" << std::endl;
    std::cout << "-- average wait time: " << std::endl;
    std::cout << "-- average turnaround time: " << std::endl;
    std::cout << "-- total number of context switches: " << cs_switches << std::endl;
    std::cout << "-- total number of preemptions: 0" << std::endl;
    std::cout << "-- CPU utilization " << cpu_util << "%" << std::endl;
    
}