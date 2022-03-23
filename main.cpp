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
int qos, cntTime, cntClient, cntServer; //����ӳ١���ʱ�䡢�ͻ�����������������
vector<string> client(Nc), server(Ns), timeLog(Nt); //�ͻ����ơ����������ơ�ʱ���
int tclient[Nt][Nc], qos_cs[Nc][Ns]; //ʱ��-�ͻ�����Ҫ������С��  �ͻ�-������ �ӳ�
vector<vector<int>> workband(Nc); //ÿ���ͻ������ӵķ�����
vector<int> tserver(Ns); //����������
unordered_map<string, int> client_idx, server_idx; //���ֶ�Ӧ���
vector<bool> vis; //�������Ƿ��ù�
vector<ll> sumBand; //ÿ��ʱ��������
vector<int> sortedTime; //����ʱ��
vector<int> maxServer; //ÿ��������������

// ���ļ�
void readConfig() {
    //config.ini����
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
    //demand.csv����
    ifstream demandFile("/data/demand.csv");
    string demands;

    //�����һ��
    vector<string> demandv;
    getline(demandFile, demands);
    {
        demands = demands.substr(0, demands.size() - 1);
        for (char& c : demands) if (c == ',') c = ' ';
        stringstream ss(demands);
        string str;
        // ���ն��ŷָ� 
        while (ss >> str) {
            demandv.push_back(str);
        }
        cntClient = demandv.size() - 1;
        for (int i = 0; i < cntClient; ++i) client[i] = demandv[i + 1];
    }
    //����ʣ��
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
        // ���ն��ŷָ�  
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
    //�����һ��
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
    //����ʣ��
    {
        int ser = 0;
        while (getline(qosFile, qoss))
        {
            for (char& c : qoss) if (c == ',') c = ' ';
            stringstream ss(qoss);
            string str;
            // ���ն��ŷָ�  
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


//���� 
void work() {

    ofstream out("/output/solution.txt");

    //Ԥ����-----------------------------------------------------------------------------------------------------/
    vis = vector<bool>(cntServer, false);
    sumBand = vector<ll>(cntTime, 0);
    sortedTime = vector<int>(cntTime, 0);
    maxServer = vector<int>(cntServer, 0);
    for (int i = 0; i < cntTime; ++i) sortedTime[i] = i;

    //����ÿ��ʱ��������
    for (int i = 0; i < cntTime; ++i) {
        for (int j = 0; j < cntClient; ++j) {
            sumBand[i] += tclient[i][j];
        }
    }

    //����,�������ʱ������
    sort(sortedTime.begin(), sortedTime.end(), [&](int a, int b) {
        return sumBand[a] > sumBand[b];
        });

    //��ϣ�����ֺ�������Ӧ
    for (int i = 0; i < cntClient; ++i) client_idx[client[i]] = i;
    for (int i = 0; i < cntServer; ++i) server_idx[server[i]] = i;

    vector<int> serverHas(cntServer, 0); //�����������ӵ��û�����

    //Ԥ������Ч�ڵ�
    for (int i = 0; i < cntClient; ++i) {
        for (int j = 0; j < cntServer; ++j) {
            if (qos_cs[i][j] < qos) {
                workband[i].push_back(j);
                serverHas[j]++;
            }
        }
    }


    //�����û������ӵı�Ե�ڵ���������С��������(���ٵ��û���ѡ��
    vector<pair<int, int>> sortedClient; //���ӷ�������-���
    for (int i = 0; i < cntClient; ++i) {
        int cnt = workband[i].size(), idx = i;
        sortedClient.push_back({ cnt, idx });
    }
    sort(sortedClient.begin(), sortedClient.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
        return a.first < b.first;
        });




    //Ԥ����-----------------------------------------------------------------------------------------------------/
    vector<vector<string>> stlog(cntTime); //ÿ��ʱ����־

    //����
    for (int timeIdx = 0; timeIdx < cntTime; ++timeIdx) { //ʱ��
        vector<int> bd = tserver;
        int t = sortedTime[timeIdx];
        //�洢��־��
        vector<unordered_map<string, int>> curlog(cntClient); //�û�i(server, get)�ӱ�Ե��ö�������



        for (int k = 0; k < cntClient; ++k) { //�û�

            //����ѡ�����Ӷ�ġ��ù��ķ�����
            for (int i = 0; i < cntClient; ++i) {
                sort(workband[i].begin(), workband[i].end(), [&](int a, int b) {
                    return serverHas[a] > serverHas[b] || vis[a] > vis[b];
                    });
            }

            int i = sortedClient[k].second; //�û����

            string s = client[i] + ":";
            int need = tclient[t][i]; //��������

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

        //ƴ����־
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


    //�洢��־
    for (int i = 0; i < cntTime; ++i) {
        for (string str : stlog[i]) out << str << endl;
    }

    out.close();
}





//���Ŀ¼/output/solution.txt

int main()
{
    /*ios::sync_with_stdio(false);
    cin.tie(0), cout.tie(0);*/

    readFile();
    work();

    return 0;
}
