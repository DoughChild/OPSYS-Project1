#include <vector>
#include <string>
#include <queue>
#include <iostream>
using namespace std;

class Process {
    public:
        char name;
        int num_bursts;
        int t_arrival;
        int tau;
        int tau_remaining;
        double wait_time;
        int turnaround_time;
        int cur_CPUBurst;
        int cs_time_left;
        int arrived_readyQ;
        int time_for_next_interesting_event;

        char dest;

        //bool in_rq;
        bool in_rq;
        bool preempted;
        bool will_preempt;

        /* will pop CPU and I/O bursts that have finished */
        deque<int> CPUBursts;
        deque<int> IOBursts;

};