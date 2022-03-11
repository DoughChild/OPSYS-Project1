/* Project.c */

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
        // initialize the seed for each algorithm, we want the same set of processes
        srand48(seed);


        // generates the first process --> needs to be looped to create multiple processes
        double rand = next_exp(lambda, upper_bound);
        printf("arrival time is %d\n", (int)floor(rand));
        rand = drand48() * 100;
        printf("number of CPU bursts is %d\n", (int)ceil(rand));
        double CPUBurst, IOBurst;
        for (int i = 0; i < ceil(rand); i++) {
            CPUBurst = next_exp(lambda, upper_bound);
            printf("CPU burst time is  %d\n", (int)ceil(CPUBurst));
            if (i != (ceil(rand) - 1)) {
                IOBurst = next_exp(lambda, upper_bound);
                printf("I/O burst time is  %d\n", (int)ceil(IOBurst) * 10);


            }
        }
        


    }

    return EXIT_SUCCESS;
}