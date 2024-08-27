#include <bits/stdc++.h>
using namespace std;
//#define int long long
#include "parser.h"

const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};

string processName(tuple<string, int, int> &A) { return get<0>(A); }
int arrivalTime(tuple<string, int, int> &A) { return get<1>(A); }
int serviceTime(tuple<string, int, int> &A) { return get<2>(A); }
int getPriorityLevel(tuple<string, int, int> &A) { return get<2>(A); }

double calculate_response_ratio(int wait_time, int service_time) { return (wait_time + service_time) * 1.0 / service_time; }
bool descendingly_by_response_ratio(tuple<string, double, int> a, tuple<string, double, int> b) { return get<1>(a) > get<1>(b); }
bool byPriorityLevel(const tuple<int, int, int> &A, const tuple<int, int, int> &B)
{
    if (get<0>(A) == get<0>(B))
        return get<2>(A) > get<2>(B);
    return get<0>(A) > get<0>(B);
}

void fillInWaitTime()
{
    for (int i = 0; i < process_count; i++)
    {
        int arrival_time = arrivalTime(processes[i]);
        for (int j = arrival_time; j < finishTime[i]; j++)
            if (timeline[j][i] != '*')
                timeline[j][i] = '.';
    }
}
void clear_timeline()
{
    for (int i = 0; i < last_instant; i++)
        for (int j = 0; j < process_count; j++)
            timeline[i][j] = ' ';
}

void FCFS()
{
    int curr_time = arrivalTime(processes[0]);
    for (int i = 0; i < process_count; i++)
    {
        int arrival_time = arrivalTime(processes[i]);
        int burst_time = serviceTime(processes[i]);

        finishTime[i] = curr_time + burst_time;
        turnAroundTime[i] = finishTime[i] - arrival_time;
        normTurn[i] = (turnAroundTime[i] * 1.0 / burst_time);

        for (int j = curr_time; j < finishTime[i]; j++)
            timeline[j][i] = '*';
        for (int j = arrival_time; j < curr_time; j++)
            timeline[j][i] = '.';

        curr_time += burst_time;
    }
}

void RoundRobin(int quantum_size)
{
    queue<pair<int, int>> Q;
    int i = 0;
    if (!arrivalTime(processes[i]))
        Q.push({i, serviceTime(processes[i])}), i++;

    int curr_quantum = quantum_size;
    for (int time = 0; time < last_instant; time++)
    {
        if (!Q.empty())
        {
            int index = Q.front().first;
            Q.front().second = Q.front().second - 1;

            int remaining_time = Q.front().second;
            int arrival_time = arrivalTime(processes[index]);
            int burst_time = serviceTime(processes[index]);
            curr_quantum--;

            timeline[time][index] = '*';

            while (i < process_count and arrivalTime(processes[i]) == time + 1)
                Q.push({i, serviceTime(processes[i])}), i++;

            if (!curr_quantum and !remaining_time)
            {
                finishTime[index] = time + 1;
                turnAroundTime[index] = finishTime[index] - arrival_time;
                normTurn[index] = (turnAroundTime[index] * 1.0 / burst_time);

                curr_quantum = quantum_size;
                Q.pop();
            }

            else if (!curr_quantum and remaining_time)
            {
                Q.pop();
                Q.push({index, remaining_time});
                curr_quantum = quantum_size;
            }

            else if (curr_quantum and !remaining_time)
            {
                finishTime[index] = time + 1;
                turnAroundTime[index] = finishTime[index] - arrival_time;
                normTurn[index] = (turnAroundTime[index] * 1.0 / burst_time);

                Q.pop();
                curr_quantum = quantum_size;
            }
        }
        while (i < process_count and arrivalTime(processes[i]) == time + 1)
            Q.push({i, serviceTime(processes[i])}), i++;
    }
    fillInWaitTime();
}

void shortestJobFirst()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> Q;

    int j = 0;
    for (int i = 0; i < last_instant; i++)
    {
        while (j < process_count and arrivalTime(processes[j]) <= i)
            Q.push({serviceTime(processes[j]), j}), j++;

        if (!Q.empty())
        {
            int index = Q.top().second;
            int arrival_time = arrivalTime(processes[index]);
            int burst_time = serviceTime(processes[index]);
            Q.pop();

            int t = arrival_time;
            for (; t < i; t++)
                timeline[t][index] = '.';

            t = i;
            for (; t < i + burst_time; t++)
                timeline[t][index] = '*';

            finishTime[index] = (i + burst_time);
            turnAroundTime[index] = (finishTime[index] - arrival_time);
            normTurn[index] = (turnAroundTime[index] * 1.0 / burst_time);
            i = t - 1;
        }
    }
}

void shortestRemainingTime()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> Q;
    int j = 0;
    for (int i = 0; i < last_instant; i++)
    {
        while (j < process_count and arrivalTime(processes[j]) == i)
            Q.push({serviceTime(processes[j]), j}), j++;

        if (!Q.empty())
        {
            int index = Q.top().second;
            int remainingTime = Q.top().first;
            Q.pop();

            int service_time = serviceTime(processes[index]);
            int arrival_time = arrivalTime(processes[index]);
            timeline[i][index] = '*';

            if (remainingTime == 1)
                finishTime[index] = i + 1,
                turnAroundTime[index] = (finishTime[index] - arrival_time),
                normTurn[index] = (turnAroundTime[index] * 1.0 / service_time);
            else
                Q.push(make_pair(remainingTime - 1, index));
        }
    }
    fillInWaitTime();
}

void highestResponseRatioNext()
{
    vector<tuple<string, double, int>> present_processes;
    int j = 0;
    for (int current_instant = 0; current_instant < last_instant; current_instant++)
    {
        while (j < process_count and arrivalTime(processes[j]) <= current_instant)
            present_processes.push_back(make_tuple(processName(processes[j]), 1.0, 0)), j++;

        for (auto &process : present_processes)
        {
            string name = get<0>(process);
            int index = processToIndex[name];

            int wait_time = current_instant - arrivalTime(processes[index]);
            int service_time = serviceTime(processes[index]);

            get<1>(process) = calculate_response_ratio(wait_time, service_time);
        }

        sort(present_processes.begin(), present_processes.end(), descendingly_by_response_ratio);

        if (!present_processes.empty())
        {
            int index = processToIndex[get<0>(present_processes[0])];
            while (current_instant < last_instant and get<2>(present_processes[0]) != serviceTime(processes[index]))
            {
                timeline[current_instant][index] = '*';
                current_instant++;
                get<2>(present_processes[0])++;
            }
            current_instant--;
            present_processes.erase(present_processes.begin());

            finishTime[index] = current_instant + 1;
            turnAroundTime[index] = finishTime[index] - arrivalTime(processes[index]);
            normTurn[index] = (turnAroundTime[index] * 1.0 / serviceTime(processes[index]));
        }
    }
    fillInWaitTime();
}

void feedbackQ1()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> Q;
    unordered_map<int, int> remainingServiceTime;

    int j = 0;
    if (arrivalTime(processes[0]) == 0)
        Q.push(make_pair(0, j)), remainingServiceTime[j] = serviceTime(processes[j]), j++;

    for (int time = 0; time < last_instant; time++)
    {
        if (!Q.empty())
        {
            int priority_level = Q.top().first;
            int index = Q.top().second;
            Q.pop();

            int arrival_time = arrivalTime(processes[index]);
            int service_time = serviceTime(processes[index]);

            while (j < process_count and arrivalTime(processes[j]) == time + 1)
                Q.push(make_pair(0, j)), remainingServiceTime[j] = serviceTime(processes[j]), j++;

            remainingServiceTime[index]--;
            timeline[time][index] = '*';
            if (!remainingServiceTime[index])
            {
                finishTime[index] = time + 1;
                turnAroundTime[index] = (finishTime[index] - arrival_time);
                normTurn[index] = (turnAroundTime[index] * 1.0 / service_time);
            }
            else
            {
                if (Q.size() >= 1)
                    Q.push({priority_level + 1, index});
                else
                    Q.push({priority_level, index});
            }
        }
        while (j < process_count and arrivalTime(processes[j]) == time + 1)
            Q.push(make_pair(0, j)), remainingServiceTime[j] = serviceTime(processes[j]), j++;
    }
    fillInWaitTime();
}

void feedbackQ2i()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> Q;
    unordered_map<int, int> remainingServiceTime;

    int j = 0;
    if (!arrivalTime(processes[0]))
        Q.push({0, j}), remainingServiceTime[j] = serviceTime(processes[j]), j++;

    for (int time = 0; time < last_instant; time++)
    {
        if (!Q.empty())
        {
            int priority_level = Q.top().first;
            int index = Q.top().second;
            Q.pop();

            int arrival_time = arrivalTime(processes[index]);
            int service_time = serviceTime(processes[index]);

            while (j < process_count and arrivalTime(processes[j]) <= time + 1)
                Q.push({0, j}), remainingServiceTime[j] = serviceTime(processes[j]), j++;

            int curr_quantum = pow(2LL, priority_level);
            int t = time;
            while (curr_quantum and remainingServiceTime[index])
            {
                curr_quantum--;
                remainingServiceTime[index]--;
                timeline[t][index] = '*', t++;
            }

            if (!remainingServiceTime[index])
            {
                finishTime[index] = t;
                turnAroundTime[index] = (finishTime[index] - arrival_time);
                normTurn[index] = (turnAroundTime[index] * 1.0 / service_time);
            }
            else
            {
                if (Q.size() >= 1)
                    Q.push(make_pair(priority_level + 1, index));
                else
                    Q.push(make_pair(priority_level, index));
            }
            time = t - 1;
        }
        while (j < process_count and arrivalTime(processes[j]) <= time + 1)
            Q.push({0, j}), remainingServiceTime[j] = serviceTime(processes[j]), j++;
    }
    fillInWaitTime();
}

void aging(int quantum_size)
{
    vector<tuple<int, int, int>> nums;
    int j = 0, curr_process = -1;
    for (int time = 0; time < last_instant; time++)
    {
        while (j < process_count and arrivalTime(processes[j]) <= time)
            nums.push_back(make_tuple(getPriorityLevel(processes[j]), j, 0)), j++;

        for (int i = 0; i < nums.size(); i++)
        {
            if (get<1>(nums[i]) == curr_process)
                get<2>(nums[i]) = 0, get<0>(nums[i]) = getPriorityLevel(processes[curr_process]);
            else
                get<0>(nums[i])++, get<2>(nums[i])++;
        }
        sort(nums.begin(), nums.end(), byPriorityLevel);
        curr_process = get<1>(nums[0]);

        int curr_quantum = quantum_size;
        while (curr_quantum-- and time < last_instant)
            timeline[time][curr_process] = '*', time++;
        time--;
    }
    fillInWaitTime();
}

void printTimeline(int algorithm_index)
{
    for (int i = 0; i <= last_instant; i++)
        cout << i % 10 << " ";
    cout << "\n";
    cout << "------------------------------------------------\n";
    for (int i = 0; i < process_count; i++)
    {
        cout << processName(processes[i]) << "     |";
        for (int j = 0; j < last_instant; j++)
            cout << timeline[j][i] << "|";
        cout << " \n";
    }
    cout << "------------------------------------------------\n";

    std::ofstream file("timeline.json");
    file << "[\n";
    for (size_t i = 0; i < process_count; ++i)
    {
        file << "  { \"process\": \"" << processName(processes[i]) << "\", \"slots\": [";
        for (size_t j = 0; j < last_instant; ++j)
        {
            file << "\"" << timeline[j][i] << "\"";
            if (j < last_instant - 1)
                file << ", ";
        }
        file << "] }";
        if (i < process_count - 1)
            file << ",";
        file << "\n";
    }
    file << "]";
    file.close();

    std::cout << "Timeline data exported to timeline.json" << std::endl;
}

void printAlgorithm(int algorithm_index)
{
    int algorithm_id = algorithms[algorithm_index].first - '0';
    if (algorithm_id == 2)
        cout << ALGORITHMS[algorithm_id] << algorithms[algorithm_index].second << endl;
    else
        cout << ALGORITHMS[algorithm_id] << endl;
}

void printProcesses()
{
    cout << "Process    ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << processName(processes[i]) << "  ";
    cout << "|\n";
}

void printArrivalTime()
{
    cout << "Arrival    ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << arrivalTime(processes[i]) << "  ";
    cout << "|\n";
}

void printServiceTime()
{
    cout << "Service    ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << serviceTime(processes[i]) << "  ";
    cout << "| Mean |\n";
}

void printFinishTime()
{
    cout << "Finish     ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << finishTime[i] << "  ";
    cout << "|-----|\n";
}

void printTurnAroundTime()
{
    cout << "Turnaround ";
    long long sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        cout << "|  " << turnAroundTime[i] << "  ";
        sum += turnAroundTime[i];
    }
    double mean = (process_count > 0) ? (1.0 * sum / process_count) : 0;
    cout << fixed << setprecision(2) << mean << "\n";
}

void printNormTurn()
{
    cout << "NormTurn   ";
    double sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        cout << "|  " << fixed << setprecision(2) << normTurn[i] << "  ";
        sum += normTurn[i];
    }
    double mean = (process_count > 0) ? (1.0 * sum / process_count) : 0;
    cout << fixed << setprecision(2) << mean << "\n";
}

void printStats(int algorithm_index)
{
    printAlgorithm(algorithm_index);
    printProcesses();
    printArrivalTime();
    printServiceTime();
    printFinishTime();
    printTurnAroundTime();
    printNormTurn();

    std::ofstream file("stats.json");
    file << "[\n";

    file << "  { \"process\": \"" << "Process" << "\", \"slots\": [";
    for (int i = 0; i < process_count; i++)
    {
        file << "\"" << processName(processes[i]) << "\"";
        if (i < process_count - 1)
            file << ", ";
    }
    file << "] }", file << ",", file << "\n";

    file << "  { \"process\": \"" << "Arrival" << "\", \"slots\": [";
    for (int i = 0; i < process_count; i++)
    {
        file << "\"" << arrivalTime(processes[i]) << "\"";
        if (i < process_count - 1)
            file << ", ";
    }
    file << "] }", file << ",", file << "\n";

    file << "  { \"process\": \"" << "Service" << "\", \"slots\": [";
    for (int i = 0; i < process_count; i++)
    {
        file << "\"" << serviceTime(processes[i]) << "\"";
        if (i < process_count - 1)
            file << ", ";
    }
    file << "] }", file << ",", file << "\n";

    file << "  { \"process\": \"" << "Finish" << "\", \"slots\": [";
    for (int i = 0; i < process_count; i++)
    {
        file << "\"" << finishTime[i] << "\"";
        if (i < process_count - 1)
            file << ", ";
    }
    file << "] }", file << ",", file << "\n";

    file << "  { \"process\": \"" << "Turnaround" << "\", \"slots\": [";
    for (int i = 0; i < process_count; i++)
    {
        file << "\"" << turnAroundTime[i] << "\"";
        if (i < process_count - 1)
            file << ", ";
    }
    file << "] }", file << ",", file << "\n";

    file << "  { \"process\": \"" << "NormTurn" << "\", \"slots\": [";
    for (int i = 0; i < process_count; i++)
    {
        file << "\"" << fixed << setprecision(2) << normTurn[i] << "\"";
        if (i < process_count - 1)
            file << ", ";
    }
    file << "] }", file << "\n";
    file << "]";
    file.close();

    std::cout << "Timeline data exported to stats.json" << std::endl;
}

void execute_algorithm(char algorithm_id, int quantum, string operation)
{
    switch (algorithm_id)
    {
    case '1':
        if (operation == TRACE)
            cout << "FCFS  ";
        FCFS();
        break;
    case '2':
        if (operation == TRACE)
            cout << "RR-" << quantum << "  ";
        RoundRobin(quantum);
        break;
    case '3':
        if (operation == TRACE)
            cout << "SPN   ";
        shortestJobFirst();
        break;
    case '4':
        if (operation == TRACE)
            cout << "SRT   ";
        shortestRemainingTime();
        break;
    case '5':
        if (operation == TRACE)
            cout << "HRRN  ";
        highestResponseRatioNext();
        break;
    case '6':
        if (operation == TRACE)
            cout << "FB-1  ";
        feedbackQ1();
        break;
    case '7':
        if (operation == TRACE)
            cout << "FB-2i ";
        feedbackQ2i();
        break;
    case '8':
        if (operation == TRACE)
            cout << "Aging ";
        aging(quantum);
        break;
    default:
        break;
    }
}

signed main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    parse();
    for (int idx = 0; idx < (int)algorithms.size(); idx++)
    {
        clear_timeline();
        execute_algorithm(algorithms[idx].first, algorithms[idx].second, operation);
        if (operation == TRACE)
            printTimeline(idx);
        else if (operation == SHOW_STATISTICS)
            printStats(idx);
        cout << "\n";
    }
    return 0;
}
