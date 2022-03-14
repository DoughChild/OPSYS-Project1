#include <vector>
#include <string>

using namespace std;

class process {
    public:
        char name;
        int num_bursts;
        int t_arrival;
        int tau;
        vector<int> CPUBursts;
        vector<int> IOBursts;



};