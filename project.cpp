/* Project.cpp */

/* This program will simulate a CPU running a series of commands
 * It will run 4 different scheduling algorithms, and compare
 * the time taken to run all processes using each algorithm. It
 * will display the time taken at the end of the program. The 4
 * algorithms to be used are FCFS, SJF, SRT, and RR.
 *
 * The program will take 7 arguments:
 *   -num of processes to create, assigning PIDs alphabetically
 *   -seed for random generation, the random generation will
 *    dictatate the interval at which the processes arrive
 *   -lambda value which will average random numbers
 *   -upper bound of random numbers
 *   -context switch time in milliseconds
 *   -alpha, a constant used in estimating CPU burst times
 *   -slice time, the time of each round in RR
 *
 * Ties in arrival to the CPU will be handled in this order:
 *   -CPU Burst completion
 *   -I/O Burst completion
 *   -New process arrival
 * If there is a tie within these categories, it will be decided
 * by PID
 *
 * Each algorithm should track (and display):
 *   -the number of preemptions
 *   -the number of context-switches
 *   -CPU usage
 *   -CPU idle time
 *   -For each CPU burst
 *     -burst time
 *     -turnaround time
 *     -wait time
 */

/* compile using: g++ -Wall -Werror project.cpp -lm */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <iostream>

#include "process.h"

using namespace std;

double next_exp(double lambda, long upper_bound)
{
    /* Generates rand 0.00 to 1.00
     * Averages the random value using log and lambda
     * Makes sure to not exceed upper bound
     */
    double rand, avg_rand;
    for (int i = 0; i < 1; i++)
    {
        rand = drand48();
        avg_rand = -log(rand) / lambda;
        if (avg_rand > upper_bound)
            i--;
    }
    return avg_rand;
}

char getAlpha(int i)
{
    /* uses the iterator of process creation to assign it a character name */
    string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    i = i % 26;
    return alpha[i];
}

void gen_process_info(double lambda, long upper_bound, process * p)
{
    /* generates burst amount and burst lengths for a given process */
    double t_arrival = next_exp(lambda, upper_bound);
    printf("arrival time is %d\n", (int)floor(t_arrival));
    double num_bursts = drand48() * 100;
    p->num_bursts = (int)ceil(num_bursts);
    double CPUBurst, IOBurst;
    for (int k = 0; k < ceil(num_bursts); k++)
    {
        CPUBurst = next_exp(lambda, upper_bound);
        p->CPUBursts.push_back((int)ceil(CPUBurst));
        if (k != (ceil(num_bursts) - 1))
        {
            IOBurst = next_exp(lambda, upper_bound);
            p->IOBursts.push_back((int)ceil(IOBurst) * 10);
        }
    }
}

int calc_tau(double prev_tau, double actual, double alpha)
{
    /* recalculate tau based on previous tau, alpha, and actual time */
    double new_tau = alpha * actual + (1.0 - alpha) * prev_tau;
    return new_tau;
}



int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: project.out <process count> <seed> <lambda> <upper bound> <context switch time> <alpha> <slice time>\n");
        return EXIT_FAILURE;
    }
    for (int i = 1; i < argc; i++)
    {
        if (!isdigit(argv[i][0]))
        {
            fprintf(stderr, "ERROR: Invalid argument(s), all arguments should be numbers.\n");
            return EXIT_FAILURE;
        }
    }

    int num_processes, t_cs, t_slice;
    double alpha, lambda;
    char *ptr;
    long seed, upper_bound;

    num_processes = atoi(argv[1]);
    seed = strtol(argv[2], &ptr, 10);
    lambda = atof(argv[3]);
    upper_bound = strtoul(argv[4], &ptr, 10);
    t_cs = atoi(argv[5]);
    alpha = atof(argv[6]);
    t_slice = atoi(argv[7]);

    // test print
    printf("7 arguments are:\n");
    printf("num processes: %d\nseed: %ld\nlambda: %lf\nupper bound: %ld\n", num_processes, seed, lambda, upper_bound);
    printf("context switch time: %d\nalpha: %lf\nslice time: %d\n", t_cs, alpha, t_slice);

    /* Loops through each algorithm:
     * 1 = FCFS, 2 = SJF, 3 = SRT, 4 = RR
     */
    for (int i = 0; i < 1; i++)
    {
        double tau = 100;

        // initialize the seed for each algorithm, we want the same set of processes
        srand48(seed);

        // makes array with pointers to each process, starting uninitialized
        process *processes[num_processes];

        // generates the first process --> needs to be looped to create multiple processes
        for (int j = 0; j < num_processes; j++)
        {
            process *p = new process();
            processes[j] = p;
            p->name = getAlpha(j);
            gen_process_info(lambda, upper_bound, p);
            

            // test the process.h object
            // cout << "Process name is " << p.name << endl;
            // cout << "there are " << p.num_bursts << " bursts\n";
            // printf("CPU Bursts are:");
            // for (auto i = p.CPUBursts.begin(); i < p.CPUBursts.end(); i++)
            // {
            //     cout << " " << *i;
            // }
            // printf("\n");
            // printf("I/O Bursts are:");
            // for (auto i = p.IOBursts.begin(); i < p.IOBursts.end(); i++)
            // {
            //     cout << " " << *i;
            // }
            // printf("\n");
        }
        cout << "Process name is " << processes[0]->name << endl;
        cout << "there are " << processes[0]->num_bursts << " CPU bursts\n";
        cout << "Process name is " << processes[1]->name << endl;
        cout << "there are " << processes[1]->num_bursts << " CPU bursts\n";
        
    }

    return EXIT_SUCCESS;
}