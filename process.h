#include <vector>
#include <string>
#include <queue>

using namespace std;

class Process {
    public:
        char name;
        int num_bursts;
        int t_arrival;
        int tau;
        int tau_remaining;
        int wait_time;
        int turnaround_time;
        int cur_CPUBurst;
        int cs_time_left;

        bool in_rq;

        /* will pop CPU and I/O bursts that have finished */
        deque<int> CPUBursts;
        deque<int> IOBursts;



};