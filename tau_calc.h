#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int calc_tau(int prev_tau, int actual, double alpha)
{
    /* recalculate tau based on previous tau, alpha, and actual time */
    // double new_tau = alpha * actual + (1.0 - alpha) * prev_tau;
    // return (int)ceil(new_tau);
    double new_tau = ceil(alpha * actual) + ceil((1.0 - alpha) * prev_tau);
    return (int)new_tau;
}