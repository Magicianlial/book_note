#include <iostream>  
#include <string>  
#include <vector>  
#include <fstream>  
#include <sstream> 
#include <queue>
#include <algorithm>
#include <unordered_map>


using namespace std;
const int Nt = 9000, Nc = 40, Ns = 200;
int qos, cntTime, cntClient, cntServer; //����ӳ١���ʱ�䡢�ͻ�����������������
vector<string> client(Nc), server(Ns), timeLog(Nt); //�ͻ����ơ����������ơ�ʱ���
int tclient[Nt][Nc], qos_cs[Nc][Ns]; //ʱ��-�ͻ�����Ҫ������С��  �ͻ�-������ �ӳ�
vector<vector<int>> workband(Nc); //ÿ���ͻ������ӵķ�����
vector<int> tserver(Ns); //����������
unordered_map<string, int> client_idx, server_idx; //���ֶ�Ӧ���

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

void debug() {
    cout << "ʱ�����г��ȣ� " << cntTime << endl;
    cout << "�ͻ������� " << cntClient << endl;
    cout << "�ͻ��б� " << endl;
    for (int i = 0; i < cntClient; ++i) cout << client[i] << endl;
    cout << "ʱ���������ʱ��-�ͻ���������excel˳���Ӧ�� " << endl;
    for (int i = 0; i < cntTime; ++i) {
        for (int j = 0; j < cntClient; ++j) {
            cout << tclient[i][j] << " ";
        }
        cout << endl;
    }
    cout << "������������ " << cntServer << endl;
    cout << "���������Ƽ�����" << endl;
    for (int i = 0; i < cntServer; ++i) cout << server[i] << " " << tserver[i] << endl;
    cout << "qos: " << qos << endl;
    cout << "������-�ͻ���Ӧ�ӳ٣� ����excel˳���Ӧ��" << endl;
    for (int i = 0; i < cntServer; ++i) {
        for (int j = 0; j < cntClient; ++j) {
            cout << qos_cs[j][i] << " ";
        }
        cout << endl;
    }

}

//���� 
void work() {

    ofstream out("/output/solution.txt");

    for (int i = 0; i < cntClient; ++i) client_idx[client[i]] = i;
    for (int i = 0; i < cntServer; ++i) server_idx[server[i]] = i;

    vector<int> serverHas(cntServer, 0);

    //Ԥ������Ч�ڵ�
    for (int i = 0; i < cntClient; ++i) {
        for (int j = 0; j < cntServer; ++j) {
            if (qos_cs[i][j] < qos) {
                workband[i].push_back(j);
                serverHas[j]++;
            }
        }
    }

    //����Ե�ڵ������û���������
    for (int i = 0; i < cntClient; ++i) {
        sort(workband[i].begin(), workband[i].end(), [&](int a, int b) {
            return serverHas[a] < serverHas[b];
        });
    }


    //�����û������ӵı�Ե�ڵ�������С��������
    vector<pair<int, int>> sortedClient;
    for (int i = 0; i < cntClient; ++i) {
        int cnt = workband[i].size(), idx = i;
        sortedClient.push_back({ cnt, idx });
    }
    sort(sortedClient.begin(), sortedClient.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
        return a.first < b.first || (a.first == b.first && a.second < b.second);
        });

    //����
    int idxLine = 0, maxLine = cntTime * cntClient - 1;
    for (int t = 0; t < cntTime; ++t) {
        vector<int> bd = tserver;

        for (int k = 0; k < cntClient; ++k) {

            int i = sortedClient[k].second;

            string s = client[i] + ":";
            int need = tclient[t][i]; //��������

            if (!need) {
                out << s;
                if(idxLine < maxLine) out << endl;
                continue;
            }

            int bn = workband[i].size();
            //�洢��־
            unordered_map<string, int> curlog; //(server, get)�ӱ�Ե��ö�������
           /* while (need) {
                int idx = rand() % bn;
                if (bd[idx] == 0) continue;
                int k = min(need, bd[idx]);
                curlog[server[idx]] += k;
                need -= k;
                bd[idx] -= k;
            }*/

            int avg_need = need / bn;
            for (int j = 0; j < bn; ++j) {
                int idx = workband[i][j];
                if (bd[idx] == 0) continue;
                int k = min(avg_need, bd[idx]);
                need -= k;
                bd[idx] -= k;
                //s += "<" + server[idx] + "," + to_string(k) + ">,";
                curlog[server[idx]] += k;
                if (need == 0) break;
            }
            
            for (int j = 0; need && j < bn; ++j) {
                int idx = workband[i][j];
                if (bd[idx] == 0) continue;
                int k = min(need, bd[idx]);
                need -= k;
                bd[idx] -= k;
                //s += "<" + server[idx] + "," + to_string(k) + ">,";
                curlog[server[idx]] += k;
                if (need == 0) break;
            }
            //while (need) cout << 1; //����ʣ��δ���� ->��ʱ����

             //ƴ����־
            for (auto& it : curlog) {
                s += "<" + it.first + "," + to_string(it.second) + ">,";
            }

            s.pop_back();
            out << s;
            if (idxLine < maxLine) out << endl;
            ++idxLine;
        }
    }

    out.close();
}

////��ʽ�ж�
//bool isFormat(string& res, vector<int>& bd) {
//    for(char &c : res) if(c == ':' || c == ',')
//    cout << "��ʽ����" << endl;
//    system("pause");
//}
//
////�ж�����Ƿ�Ϸ�
//void check() {
//    fstream data("/output/solution.txt");
//    string line;
//    for(int i = 0; i < cntTime; ++i) {
//
//        vector<int> bd = tserver;
//
//        for (int j = 0; j < cntClient; ++j) {
//            getline(data, line);
//            cout << data << endl;
//            isFormat(line, bd);
//        }
//    }
//}



//���Ŀ¼/output/solution.txt

int main()
{
    /*ios::sync_with_stdio(false);
    cin.tie(0), cout.tie(0);*/

    readFile();
    work();
    //debug(); //�������

    return 0;
}
