#include <iostream>  
#include <string>  
#include <vector>  
#include <fstream>  
#include <sstream> 
#include <queue>
#include <algorithm>
#include <unordered_map>


using namespace std;
using pii = pair<int, int>;
using ll = long long;
const int Nt = 9000, Nc = 40, Ns = 200;
int qos, cntTime, cntClient, cntServer; //最大延迟、总时间、客户数量、服务器数量
vector<string> client(Nc), server(Ns), timeLog(Nt); //客户名称、服务器名称、时间戳
int tclient[Nt][Nc], qos_cs[Nc][Ns]; //时间-客户：需要流量大小，  客户-服务器 延迟
vector<vector<int>> workband(Nc); //每个客户能连接的服务器
vector<int> tserver(Ns); //服务器带宽
unordered_map<string, int> client_idx, server_idx; //名字对应序号
vector<bool> vis; //服务器是否用过
vector<ll> sumBand; //每个时刻总流量
vector<int> sortedTime; //排序时刻
vector<int> maxServer; //每个服务器最多分配

// 读文件
void readConfig() {
    //config.ini读入
    ifstream configFile("/data/config.ini");
    string configs;
    // while (getline(configFile, configs)) cout << configs << endl;

    getline(configFile, configs);
    getline(configFile, configs);
    string configlog = "qos_constraint=";
    int nconfig = configlog.size();
    //   cout << nconfig << endl;
    //   cout << configs << endl;
    qos = stoi(configs.substr(nconfig));
    configFile.close();

}

void readDemand() {
    //demand.csv读入
    ifstream demandFile("/data/demand.csv");
    string demands;

    //处理第一行
    vector<string> demandv;
    getline(demandFile, demands);
    {
        demands = demands.substr(0, demands.size() - 1);
        for (char& c : demands) if (c == ',') c = ' ';
        stringstream ss(demands);
        string str;
        // 按照逗号分隔 
        while (ss >> str) {
            demandv.push_back(str);
        }
        cntClient = demandv.size() - 1;
        for (int i = 0; i < cntClient; ++i) client[i] = demandv[i + 1];
    }
    //处理剩余
    {
        int time = 0;
        while (getline(demandFile, demands))
        {
            for (char& c : demands) if (c == ',') c = ' ';
            stringstream ss(demands);
            string str;
            int i = 0;
            while (ss >> str) {
                demandv[i++] = str;
            }

            timeLog[time] = demandv[0];
            for (int j = 0; j < cntClient; ++j)
                tclient[time][j] = stoi(demandv[j + 1]);

            ++time;
        }
        cntTime = time;
    }
    demandFile.close();
}

void readBandwidth() {
    ifstream bandFile("/data/site_bandwidth.csv");
    string bands;
    getline(bandFile, bands);

    cntServer = 0;
    vector<string> tmp(2);
    while (getline(bandFile, bands))
    {
        for (char& c : bands) if (c == ',') c = ' ';
        stringstream ss(bands);
        string str;
        // 按照逗号分隔  
        int i = 0;
        while (ss >> str) {
            tmp[i++] = str;
        }
        server[cntServer] = tmp[0];
        tserver[cntServer] = stoi(tmp[1]);

        ++cntServer;
    }
    bandFile.close();
}

void readQos() {
    ifstream qosFile("/data/qos.csv");
    string qoss;
    //处理第一行
    vector<string> qosvs;
    getline(qosFile, qoss);
    {
        qoss = qoss.substr(0, qoss.size() - 1);
        for (char& c : qoss) if (c == ',') c = ' ';
        stringstream ss(qoss);
        string str;
        // 
        while (ss >> str) {
            qosvs.push_back(str);
        }
    }
    //处理剩余
    {
        int ser = 0;
        while (getline(qosFile, qoss))
        {
            for (char& c : qoss) if (c == ',') c = ' ';
            stringstream ss(qoss);
            string str;
            // 按照逗号分隔  
            int i = 0;
            while (ss >> str) {
                qosvs[i++] = str;
            }
            for (int j = 0; j < cntClient; ++j)
                qos_cs[j][ser] = stoi(qosvs[j + 1]);

            ++ser;
        }
    }
    qosFile.close();
}

void readFile() {
    readDemand();
    readBandwidth();
    readQos();
    readConfig();
}


//调度 
void work() {

    ofstream out("/output/solution.txt");

    //预处理-----------------------------------------------------------------------------------------------------/
    vis = vector<bool>(cntServer, false);
    sumBand = vector<ll>(cntTime, 0);
    sortedTime = vector<int>(cntTime, 0);
    maxServer = vector<int>(cntServer, 0);
    for (int i = 0; i < cntTime; ++i) sortedTime[i] = i;

    //计算每个时刻总流量
    for (int i = 0; i < cntTime; ++i) {
        for (int j = 0; j < cntClient; ++j) {
            sumBand[i] += tclient[i][j];
        }
    }

    //排序,流量大的时刻先排
    sort(sortedTime.begin(), sortedTime.end(), [&](int a, int b) {
        return sumBand[a] > sumBand[b];
        });

    //哈希表：名字和索引对应
    for (int i = 0; i < cntClient; ++i) client_idx[client[i]] = i;
    for (int i = 0; i < cntServer; ++i) server_idx[server[i]] = i;

    vector<int> serverHas(cntServer, 0); //服务器能连接的用户数量

    //预处理有效节点
    for (int i = 0; i < cntClient; ++i) {
        for (int j = 0; j < cntServer; ++j) {
            if (qos_cs[i][j] < qos) {
                workband[i].push_back(j);
                serverHas[j]++;
            }
        }
    }


    //按照用户能连接的边缘节点数量，从小到大排序(边少的用户先选）
    vector<pair<int, int>> sortedClient; //连接服务器数-序号
    for (int i = 0; i < cntClient; ++i) {
        int cnt = workband[i].size(), idx = i;
        sortedClient.push_back({ cnt, idx });
    }
    sort(sortedClient.begin(), sortedClient.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
        return a.first < b.first;
        });




    //预处理-----------------------------------------------------------------------------------------------------/
    vector<vector<string>> stlog(cntTime); //每个时刻日志

    //处理
    for (int timeIdx = 0; timeIdx < cntTime; ++timeIdx) { //时间
        vector<int> bd = tserver;
        int t = sortedTime[timeIdx];
        //存储日志组
        vector<unordered_map<string, int>> curlog(cntClient); //用户i(server, get)从边缘获得多少流量



        for (int k = 0; k < cntClient; ++k) { //用户

            //优先选择连接多的、用过的服务器
            for (int i = 0; i < cntClient; ++i) {
                sort(workband[i].begin(), workband[i].end(), [&](int a, int b) {
                    return serverHas[a] > serverHas[b] || vis[a] > vis[b];
                    });
            }

            int i = sortedClient[k].second; //用户序号

            string s = client[i] + ":";
            int need = tclient[t][i]; //所需流量

            if (!need) {
                //out << s << endl;
                stlog[t].push_back(s);
                continue;
            }


            for (int j = 0; need && j < workband.size(); ++j) {
                int idx = workband[i][j];
                if (bd[idx] == 0) continue;
                int k = min(need, bd[idx]);
                vis[idx] = 1;
                need -= k;
                bd[idx] -= k;
                curlog[i][server[idx]] += k;
            }



        }

        //拼接日志
        for (int ci = 0; ci < cntClient; ++ci) {
            if (curlog[ci].empty()) continue;
            string s = client[ci] + ":";
            for (auto& it : curlog[ci]) {
                s += "<" + it.first + "," + to_string(it.second) + ">,";
            }

            s.pop_back();
            stlog[t].push_back(s);
            //out << s << endl;
        }

        for (int q = 0; q < cntServer; ++q) maxServer[q] = max(maxServer[q], tserver[q] - bd[q]);
    }


    //存储日志
    for (int i = 0; i < cntTime; ++i) {
        for (string str : stlog[i]) out << str << endl;
    }

    out.close();
}





//输出目录/output/solution.txt

int main()
{
    /*ios::sync_with_stdio(false);
    cin.tie(0), cout.tie(0);*/

    readFile();
    work();

    return 0;
}
