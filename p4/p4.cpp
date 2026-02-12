#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <algorithm>
#include <climits>

using namespace std;

struct Stats {
    double avgResp;
    double avgTA;
    double avgWait;
};

static const int MAXP = 100;

Stats simulateFCFS(const vector<int>& arrival, const vector<int>& burst) {
    int n = arrival.size();
    vector<int> start(n, -1), finish(n, -1);

    int clock = 0;
    for (int i = 0; i < n; i++) {
        if (clock < arrival[i]) clock = arrival[i];
        start[i] = clock;
        clock += burst[i];
        finish[i] = clock;
    }

    double sumResp = 0, sumTA = 0, sumWait = 0;
    for (int i = 0; i < n; i++) {
        int resp = start[i] - arrival[i];
        int ta = finish[i] - arrival[i];
        int wait = ta - burst[i];
        sumResp += resp;
        sumTA += ta;
        sumWait += wait;
    }

    return {sumResp / n, sumTA / n, sumWait / n};
}

Stats simulateSJF(const vector<int>& arrival, const vector<int>& burst) {
    int n = arrival.size();
    vector<int> start(n, -1), finish(n, -1);
    vector<bool> done(n, false);

    int clock = 0;
    int completed = 0;
    int nextArr = 0;

    while (completed < n) {
        // If nothing is ready, jump clock to next arrival
        if (nextArr < n && clock < arrival[nextArr]) {
            bool anyReady = false;
            for (int i = 0; i < n; i++) {
                if (!done[i] && arrival[i] <= clock) {
                    anyReady = true;
                    break;
                }
            }
            if (!anyReady) clock = arrival[nextArr];
        }

        // Choose shortest burst among arrived processes
        int best = -1;
        int bestBurst = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!done[i] && arrival[i] <= clock) {
                if (burst[i] < bestBurst) {
                    bestBurst = burst[i];
                    best = i;
                }
            }
        }

        if (best == -1) continue; // should not happen, but safe

        start[best] = clock;
        clock += burst[best];
        finish[best] = clock;
        done[best] = true;
        completed++;
    }

    double sumResp = 0, sumTA = 0, sumWait = 0;
    for (int i = 0; i < n; i++) {
        int resp = start[i] - arrival[i];
        int ta = finish[i] - arrival[i];
        int wait = ta - burst[i];
        sumResp += resp;
        sumTA += ta;
        sumWait += wait;
    }

    return {sumResp / n, sumTA / n, sumWait / n};
}

Stats simulateSRTF(const vector<int>& arrival, const vector<int>& burst) {
    int n = arrival.size();
    vector<int> remaining = burst;
    vector<int> firstStart(n, -1);
    vector<int> finish(n, -1);

    int clock = 0;
    int completed = 0;
    int nextArr = 0;

    // min-heap: (remaining time, pid)
    using P = pair<int,int>;
    priority_queue<P, vector<P>, greater<P>> pq;

    while (completed < n) {
        // If no ready process, jump to next arrival
        if (pq.empty() && nextArr < n && clock < arrival[nextArr]) {
            clock = arrival[nextArr];
        }

        // Add all processes that have arrived by now
        while (nextArr < n && arrival[nextArr] <= clock) {
            pq.push({remaining[nextArr], nextArr});
            nextArr++;
        }

        if (pq.empty()) continue;

        auto [rem, pid] = pq.top();
        pq.pop();

        if (firstStart[pid] == -1) firstStart[pid] = clock;

        // Determine how long we can run this process before next arrival preempts
        int nextArrivalTime = (nextArr < n ? arrival[nextArr] : INT_MAX);
        int runFor = min(rem, nextArrivalTime - clock);

        clock += runFor;
        remaining[pid] -= runFor;

        // Add arrivals that happened exactly at new clock time
        while (nextArr < n && arrival[nextArr] <= clock) {
            pq.push({remaining[nextArr], nextArr});
            nextArr++;
        }

        if (remaining[pid] == 0) {
            finish[pid] = clock;
            completed++;
        } else {
            pq.push({remaining[pid], pid});
        }
    }

    double sumResp = 0, sumTA = 0, sumWait = 0;
    for (int i = 0; i < n; i++) {
        int resp = firstStart[i] - arrival[i];
        int ta = finish[i] - arrival[i];
        int wait = ta - burst[i];
        sumResp += resp;
        sumTA += ta;
        sumWait += wait;
    }

    return {sumResp / n, sumTA / n, sumWait / n};
}

Stats simulateRR(const vector<int>& arrival, const vector<int>& burst, int quantum) {
    int n = arrival.size();
    vector<int> remaining = burst;
    vector<int> firstStart(n, -1);
    vector<int> finish(n, -1);

    queue<int> q;

    int clock = 0;
    int completed = 0;
    int nextArr = 0;

    // Jump to first arrival
    if (n > 0) clock = arrival[0];

    // Add any arrivals at start time
    while (nextArr < n && arrival[nextArr] <= clock) {
        q.push(nextArr);
        nextArr++;
    }

    while (completed < n) {
        if (q.empty()) {
            // Jump to next arrival
            clock = arrival[nextArr];
            while (nextArr < n && arrival[nextArr] <= clock) {
                q.push(nextArr);
                nextArr++;
            }
            continue;
        }

        int pid = q.front();
        q.pop();

        if (firstStart[pid] == -1) firstStart[pid] = clock;

        int runFor = min(quantum, remaining[pid]);
        clock += runFor;
        remaining[pid] -= runFor;

        // IMPORTANT RULE:
        // Add newly arrived processes BEFORE re-adding the current one.
        while (nextArr < n && arrival[nextArr] <= clock) {
            q.push(nextArr);
            nextArr++;
        }

        if (remaining[pid] == 0) {
            finish[pid] = clock;
            completed++;
        } else {
            q.push(pid);
        }
    }

    double sumResp = 0, sumTA = 0, sumWait = 0;
    for (int i = 0; i < n; i++) {
        int resp = firstStart[i] - arrival[i];
        int ta = finish[i] - arrival[i];
        int wait = ta - burst[i];
        sumResp += resp;
        sumTA += ta;
        sumWait += wait;
    }

    return {sumResp / n, sumTA / n, sumWait / n};
}

void printStats(const Stats& s) {
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
        arrival.push_back(a);
        burst.push_back(b);
        if ((int)arrival.size() >= MAXP) break;
    }

    if (arrival.empty()) return 0;

    cout << "First Come, First Served\n";
    Stats fcfs = simulateFCFS(arrival, burst);
    printStats(fcfs);
    cout << "\n";

    cout << "Shortest Job First\n";
    Stats sjf = simulateSJF(arrival, burst);
    printStats(sjf);
    cout << "\n";

    cout << "Shortest Remaining Time First\n";
    Stats srtf = simulateSRTF(arrival, burst);
    printStats(srtf);
    cout << "\n";

    cout << "Round Robin with Time Quantum of " << quantum << "\n";
    Stats rr = simulateRR(arrival, burst, quantum);
    printStats(rr);

    return 0;
}
