#include <iostream>         
#include <vector>           
#include <string>           
#include <filesystem>       
#include <algorithm>        
#include <cctype>           
#include <string>
#include <fstream>
#include <sstream> 
#include <chrono>
#include <thread>
#include <cstdlib>
using namespace std;

namespace fs = filesystem;

bool isDigits(const string& s) {
    return !s.empty() && all_of(s.begin(), s.end(), ::isdigit);
}

vector<int> getAllPIDs() {
    vector<int> pids;

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            string name = entry.path().filename().string(); 
            if (isDigits(name)) {
                int pid = stoi(name);
                pids.push_back(pid);
            }
        }
    }
    return pids;
}

struct ProcessInfo{
    int pid;
    string name;
    int memory_kb;
};

ProcessInfo getProcessInfo(int pid){
    ProcessInfo info;
    info.pid = pid;
    info.name = "unknown";
    info.memory_kb = 0;

    string path = "/proc/" + to_string(pid) + "/status";
    ifstream file(path);

    if(!file.is_open()) return info;

    string line;
    while(getline(file, line)){
        if(line.rfind("Name:", 0) == 0){
            istringstream iss(line);
            string lable;
            iss >> lable >> info.name;
        }
        if(line.rfind("VmRSS:", 0) == 0){
            istringstream iss(line);
            string lable;
            int mem;
            string unit;
            iss >> lable >> mem >> unit;
            info.memory_kb = mem;
        }
    }
    return info;
}
bool export_csv(const string& filename, const vector<ProcessInfo>& data) {
    ofstream file(filename);

    if (!file.is_open()) {
        return false;
    }
    
    file << "PID,Name,Memory_kB\n";
    for (const auto& info : data) {
        if(info.memory_kb > 0){
            file << info.pid << "," << info.name << "," << info.memory_kb << "\n";
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
            cout << "Usage:\n"
             << "  " << argv[0] << " csv               Export all processes to output.csv\n"
             << "  " << argv[0] << " live              Show live memory usage\n"
             << "  " << argv[0] << " top N             Show top N memory consumers\n"
             << "  " << argv[0] << " pid <PID>         Show memory info of specific PID\n";
    if (argc == 2 && string(argv[1]) == "csv") {
    auto pids = getAllPIDs();
    vector<ProcessInfo> infos;

    for (int pid : pids)
        infos.push_back(getProcessInfo(pid));

    sort(infos.begin(), infos.end(), [](const auto& a, const auto& b) {
        return a.memory_kb > b.memory_kb;
    });

    if (export_csv("output.csv", infos)){
        cout << "Exported to output.csv\n";
    }
    else{
        cout << "Export failed\n";
    }

    return 0;
    }
    else if (argc == 2 && string(argv[1]) == "live") {
        while (true) {
        system("clear");

        auto pids = getAllPIDs();
        vector<ProcessInfo> infos;

        for (int pid : pids) {
            ProcessInfo info = getProcessInfo(pid);
            infos.push_back(info);
        }

        sort(infos.begin(), infos.end(), [](const auto& a, const auto& b) {
            return a.memory_kb > b.memory_kb;
        });

        for (const auto& info : infos) {
            if (info.memory_kb > 0)
                cout << "PID: " << info.pid
                     << " | Name: " << info.name
                     << " | Memory: " << info.memory_kb << " kB" << endl;
        }

        this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
    }
    else if (argc == 3 && string(argv[1]) == "top"){
        int N = stoi(argv[2]);
        auto pids = getAllPIDs();
        vector<ProcessInfo> infos;

        for (int pid : pids) {
            ProcessInfo info = getProcessInfo(pid);
            infos.push_back(info);
        }

        sort(infos.begin(), infos.end(), [](const auto& a, const auto& b) {
            return a.memory_kb > b.memory_kb;
        });
        int a = 0;
        for(const auto& info : infos) {
            a++;
            if(a <= N){
            if (info.memory_kb > 0){
                cout << "PID: " << info.pid
                     << " | Name: " << info.name
                     << " | Memory: " << info.memory_kb << " kB" << endl;
            }
            }
        }
    }
    else if (argc == 3 && string(argv[1]) == "pid"){
        int pid = stoi(argv[2]);
        ProcessInfo info = getProcessInfo(pid);
        if (info.memory_kb > 0){
            cout << "PID: " << info.pid
                 << " | Name: " << info.name
                 << " | Memory: " << info.memory_kb << " kB" << endl;
        }    
    }
     else {
        cout << "Usage:\n"
             << "  " << argv[0] << " csv               Export all processes to output.csv\n"
             << "  " << argv[0] << " live              Show live memory usage\n"
             << "  " << argv[0] << " top N             Show top N memory consumers\n"
             << "  " << argv[0] << " pid <PID>         Show memory info of specific PID\n";
    }
    return 0;
}
