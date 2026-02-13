#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <string>
#include <limits>

using namespace std;

struct Stats {
    double avgResp;
    double avgTA;
    double avgWait;
};

static const int MAXP = 100;

Stats runFCFS(const vector<int>& arrival, const vector<int>& burst) {
    int n = (int)arrival.size();

    long long sumResp = 0, sumTA = 0, sumWait = 0;

    long long t = 0;

    for (int i = 0; i < n; i++) {
        if (t < arrival[i]) t = arrival[i];

        long long start = t;
        long long finish = t + burst[i];

        long long resp = start - arrival[i];
        long long ta = finish - arrival[i];
        long long wait = ta - burst[i];

        sumResp += resp;
        sumTA += ta;
        sumWait += wait;

        t = finish;
    }

    Stats s;
    s.avgResp = (double)sumResp / n;
    s.avgTA = (double)sumTA / n;
    s.avgWait = (double)sumWait / n;
    return s;
}

Stats runSJF(const vector<int>& arrival, const vector<int>& burst) {
    int n = (int)arrival.size();

    vector<bool> done(n, false);

    long long sumResp = 0, sumTA = 0, sumWait = 0;
    long long t = 0;
    int finished = 0;

    while (finished < n) {
        int best = -1;

        // find all arrived & not done, pick smallest burst
        for (int i = 0; i < n; i++) {
            if (!done[i] && arrival[i] <= t) {
                if (best == -1 || burst[i] < burst[best]) {
                    best = i;
                }
            }
        }

        if (best == -1) {
            // no process ready, jump time to next arrival
            long long nextArr = LLONG_MAX;
            for (int i = 0; i < n; i++) {
                if (!done[i]) nextArr = min(nextArr, (long long)arrival[i]);
            }
            t = nextArr;
            continue;
        }

        long long start = t;
        long long finish = t + burst[best];

        long long resp = start - arrival[best];
        long long ta = finish - arrival[best];
        long long wait = ta - burst[best];

        sumResp += resp;
        sumTA += ta;
        sumWait += wait;

        t = finish;
        done[best] = true;
        finished++;
    }

    Stats s;
    s.avgResp = (double)sumResp / n;
    s.avgTA = (double)sumTA / n;
    s.avgWait = (double)sumWait / n;
    return s;
}

Stats runSRTF(const vector<int>& arrival, const vector<int>& burst) {
    int n = (int)arrival.size();

    vector<int> remaining = burst;
    vector<int> firstStart(n, -1);
    vector<int> finishTime(n, -1);

    long long t = 0;
    int finished = 0;

    while (finished < n) {
        int best = -1;

        // pick arrived with smallest remaining time
        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0 && arrival[i] <= t) {
                if (best == -1 || remaining[i] < remaining[best]) {
                    best = i;
                }
            }
        }

        if (best == -1) {
            // jump to next arrival
            long long nextArr = LLONG_MAX;
            for (int i = 0; i < n; i++) {
                if (remaining[i] > 0) nextArr = min(nextArr, (long long)arrival[i]);
            }
            t = nextArr;
            continue;
        }

        if (firstStart[best] == -1) firstStart[best] = (int)t;

        // Run until either it finishes OR a new process arrives that could preempt.
        long long nextArrival = LLONG_MAX;
        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0 && arrival[i] > t) {
                nextArrival = min(nextArrival, (long long)arrival[i]);
            }
        }

        long long runFor;
        if (nextArrival == LLONG_MAX) {
            runFor = remaining[best];
        } else {
            runFor = min((long long)remaining[best], nextArrival - t);
        }

        remaining[best] -= (int)runFor;
        t += runFor;

        if (remaining[best] == 0) {
            finishTime[best] = (int)t;
            finished++;
        }
    }

    long long sumResp = 0, sumTA = 0, sumWait = 0;

    for (int i = 0; i < n; i++) {
        long long resp = firstStart[i] - arrival[i];
        long long ta = finishTime[i] - arrival[i];
        long long wait = ta - burst[i];

        sumResp += resp;
        sumTA += ta;
        sumWait += wait;
    }

    Stats s;
    s.avgResp = (double)sumResp / n;
    s.avgTA = (double)sumTA / n;
    s.avgWait = (double)sumWait / n;
    return s;
}

Stats runRR(const vector<int>& arrival, const vector<int>& burst, int quantum) {
    int n = (int)arrival.size();

    vector<int> remaining = burst;
    vector<int> firstStart(n, -1);
    vector<int> finishTime(n, -1);

    queue<int> rq;

    long long t = 0;
    int finished = 0;
    int nextToArrive = 0;

    // jump to first arrival
    t = arrival[0];
    rq.push(0);
    nextToArrive = 1;

    while (finished < n) {
        if (rq.empty()) {
            // jump to next arrival
            t = arrival[nextToArrive];
            rq.push(nextToArrive);
            nextToArrive++;
        }

        int p = rq.front();
        rq.pop();

        if (firstStart[p] == -1) firstStart[p] = (int)t;

        int slice = min(quantum, remaining[p]);
        long long endTime = t + slice;

        // IMPORTANT RULE:
        // Add any processes that arrive during this slice BEFORE re-adding p.
        while (nextToArrive < n && arrival[nextToArrive] <= endTime) {
            rq.push(nextToArrive);
            nextToArrive++;
        }

        remaining[p] -= slice;
        t = endTime;

        if (remaining[p] == 0) {
            finishTime[p] = (int)t;
            finished++;
        } else {
            rq.push(p);
        }
    }

    long long sumResp = 0, sumTA = 0, sumWait = 0;

    for (int i = 0; i < n; i++) {
        long long resp = firstStart[i] - arrival[i];
        long long ta = finishTime[i] - arrival[i];
        long long wait = ta - burst[i];

        sumResp += resp;
        sumTA += ta;
        sumWait += wait;
    }

    Stats s;
    s.avgResp = (double)sumResp / n;
    s.avgTA = (double)sumTA / n;
    s.avgWait = (double)sumWait / n;
    return s;
}

static void printStats(const Stats& s) {
    cout << fixed << setprecision(2);
    cout << "Avg. Resp.:" << s.avgResp
         << ", Avg. T.A.:" << s.avgTA
         << ", Avg. Wait:" << s.avgWait << "\n";
}

int main(int argc, char* argv[]) {
    int quantum = 100;
    if (argc >= 2) quantum = stoi(argv[1]);

    vector<int> arrival;
    vector<int> burst;

    int a, b;
    while (cin >> a >> b) {
        if ((int)arrival.size() >= MAXP) break;
        arrival.push_back(a);
        burst.push_back(b);
    }

    if (arrival.empty()) return 0;

    Stats fcfs = runFCFS(arrival, burst);
    Stats sjf = runSJF(arrival, burst);
    Stats srtf = runSRTF(arrival, burst);
    Stats rr = runRR(arrival, burst, quantum);

    cout << "First Come, First Served\n";
    printStats(fcfs);
    cout << "\n";

    cout << "Shortest Job First\n";
    printStats(sjf);
    cout << "\n";

    cout << "Shortest Remaining Time First\n";
    printStats(srtf);
    cout << "\n";

    cout << "Round Robin with Time Quantum of " << quantum << "\n";
    printStats(rr);

    return 0;
}
