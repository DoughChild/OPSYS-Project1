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
        double wait_time;
        int turnaround_time;
        int cur_CPUBurst;
        int cs_time_left;
        int arrived_readyQ;
        int time_for_next_interesting_event;

        //bool in_rq;
        bool in_rq;

        /* will pop CPU and I/O bursts that have finished */
        deque<int> CPUBursts;
        deque<int> IOBursts;

        void printProcess() const {
            cout << "t_arrival " << t_arrival << endl;
            cout << "name " << name << endl;
            cout << "tau " << tau << endl;
            if (CPUBursts.size() > 0) {
                cout << "CPUBurst[0] " << CPUBursts[0] << endl;
            }
            cout << endl;
        }

        void copy(const Process* p) {
            name = p->name;
            num_bursts = p->num_bursts;
            t_arrival = p->t_arrival;
            tau = p->tau;
            tau_remaining = p->tau_remaining;
            wait_time = p->wait_time;
            turnaround_time = p->turnaround_time;
            cur_CPUBurst = p->cur_CPUBurst;
            cs_time_left = p->cs_time_left;
            in_rq = p->in_rq;
            time_for_next_interesting_event = p->time_for_next_interesting_event;
            arrived_readyQ = p->arrived_readyQ;
            for (int i = 0; i < p->CPUBursts.size(); i++) {
                CPUBursts.push_back(p->CPUBursts[i]);
            }
            for (int i = 0; i < p->IOBursts.size(); i++) {
                IOBursts.push_back(p->IOBursts[i]);
            }
        }


};