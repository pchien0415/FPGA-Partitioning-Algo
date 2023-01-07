#include<iostream>
#include<queue>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<set>
#include<algorithm>
#include<cstring>
using namespace std;

//Setting
int file[6];
int imformation[6];
int external_degree[200000][43];
int fpga_channel[43][43];
int fpga_max_dis[43];
int fpga_capacity[43];
int fpga_capacity_predict[43];
int node_count[200000];
int capacity;
vector<vector<int> > nets;
vector<vector<int> > Cnode;
vector<vector<int> > Fnode;         //fpga[i]存距離<=1的fpga
vector<vector<int> > cddt;
vector<vector<int> > copy_cddt;
vector<int> F;                      //存固定的node，用在Algo1
vector<int> copy_F;                 //存固定的node，用在Algo1

int partitions[200000];             //存node分配給哪個FPGA(尚未確定分配-1)

priority_queue<pair<pair<int, int>, int>, vector<pair<pair<int, int>, int> > , greater<pair<pair<int, int>, int> > > Q;
priority_queue<pair<pair<int, int>, int>, vector<pair<pair<int, int>, int> >, greater<pair<pair<int, int>, int> > > R[200000];

int traceback = 0;

//ReadFile Setting
vector<string> v;
stringstream ss;
string line, imfor;

void ReadImformation(vector<string> v){
    int n;
    for(int i=0; i<6; i++){
        ss.clear(); ss.str("");      // clear ss
        ss << v[i]; ss >> n;
        imformation[i] = n;
        cout << imformation[i] << " ";
        ss.clear(); ss.str("");      // clear ss
    }
    cout << endl;
}



void ReadFpgaChannel(vector<string> v){
    int src, des;
    ss.str(""); ss.clear();         // clear ss
    ss << v[0]; ss >> src;
    ss.str(""); ss.clear();         // clear ss
    ss << v[1]; ss >> des;
    Fnode[src].push_back(des);
    Fnode[des].push_back(src);
    fpga_channel[src][des] = 1;
    fpga_channel[des][src] = 1;
}

void ReadNode(vector<string> v, int count){
    int num[100] = {0};
    int n;
    for(int j=0; j<count; j++){
        ss.str(""); ss.clear();
        ss << v[j]; ss >> n;
        num[j] = n;
        ss.str(""); ss.clear();
    }
    for(int j=1; j<count; j++){
        Cnode[num[0]].push_back(num[j]);
        Cnode[num[j]].push_back(num[0]);
    }
    for(int i=0; i<count; i++){
        for(int j=i+1; j<count; j++){
            nets[num[i]].push_back(num[j]);
            nets[num[j]].push_back(num[i]);
        }
    }
}

void ReadFixNode(vector<string> v){
    int n, temp[2];
    for(int i=0; i<2; i++){
        ss.clear(); ss.str("");     // clear ss
        ss << v[i]; ss >> n;
        temp[i] = n;
        ss.clear(); ss.str("");     // clear ss
    }
    partitions[temp[0]] = temp[1];
    fpga_capacity[temp[1]]++;
    F.push_back(temp[0]);
    ss.str(""); ss.clear();
}

void UpdateCandidate(int update, int now_node){     // Algo1
    for(int i=0; i<cddt[now_node].size(); i++){
        vector<int> temp;
        set_intersection(cddt[update].begin(), cddt[update].end(), Fnode[cddt[now_node][i]].begin(), Fnode[cddt[now_node][i]].end(), back_inserter(temp));
        cddt[update] = temp;
    }
    if(cddt[update].size() == 1){
        if(fpga_capacity[cddt[update][0]] < capacity){
            F.push_back(update);
            partitions[update] = cddt[update][0];
        }
    }
}       

void UpateAndCheck(int node, int fpga){
    //cout << "======here1======";
    for(int i=0; i<Cnode[node].size(); i++){
        vector<int> temp;
        if(partitions[Cnode[node][i]] == -1){
            //cout << "======here1======";
            copy_cddt[Cnode[node][i]] = cddt[Cnode[node][i]];
            set_intersection(cddt[Cnode[node][i]].begin(), cddt[Cnode[node][i]].end(), Fnode[fpga].begin(), Fnode[fpga].end(), back_inserter(temp));
            cddt[Cnode[node][i]] = temp;
            if(temp.size() == 0){
                //cout << "=======here1=======" << endl;
                traceback = 1;
                break;
            }
        }
    }
}



int main(int argc, char *argv[]){
    //ReadFile
    ifstream myFile;
    myFile.open(argv[1]);
    
    //ReadImformation
    getline(myFile, line);
    ss << line;
    while(getline(ss, imfor, ' ')){
        v.push_back(imfor);
    }
    ReadImformation(v);
    v.clear();

    nets.resize(imformation[3]);
    Cnode.resize(imformation[3]);
    cddt.resize(imformation[3]);
    copy_cddt.resize(imformation[3]);
    Fnode.resize(imformation[0]);


    // ReadFpgaChannel
    for(int i=0; i<imformation[0]; i++){        //init
        for(int j=0; j<imformation[0]; j++){
            if(i == j){
                fpga_channel[i][j] = 0;
            }
            else{
                fpga_channel[i][j] = 10000;
            }
        }
    }
    for(int i=0; i<imformation[0]; i++){        // init
        Fnode[i].resize(1, i);
    }
    for(int i=0; i<imformation[1]; i++){
        ss.str(""); ss.clear();                 // clear ss
        getline(myFile, line);
        ss << line;
        while(getline(ss, imfor, ' ')){
            v.push_back(imfor);
        }
        ReadFpgaChannel(v);
        v.clear();
    }
    for(int i=0; i<imformation[0]; i++){        // init
        sort(Fnode[i].begin(), Fnode[i].end());
    }
    //算fpga_max_dis
    for(int k=0; k<imformation[0]; k++){        //Floyd-Warshall
        for(int i=0; i<imformation[0]; i++){
            for(int j=0; j<imformation[0]; j++){
                if(fpga_channel[i][j] > fpga_channel[i][k] + fpga_channel[k][j]){
                    fpga_channel[i][j] = fpga_channel[i][k] + fpga_channel[k][j];
                }
            }
        }
    }
    for(int i=0; i<imformation[0]; i++){
        int max_dis = 0;
        for(int j=0; j<imformation[0]; j++){
            if(max_dis < fpga_channel[i][j]){
                max_dis = fpga_channel[i][j];
            }
        }
        fpga_max_dis[i] = max_dis;
    }  

    // capacity
    capacity = imformation[2];
    
    // ReadNode
    for(int i=0; i<imformation[4]; i++){
        int count = 0;
        ss.str(""); ss.clear();     
        getline(myFile, line);
        ss << line;
        while(getline(ss, imfor, ' ')){
            v.push_back(imfor);
            count++;
        }
        ReadNode(v, count);
        v.clear();
    }

    for(int i=0; i<nets[0].size(); i++){
        cout << nets[0][i] << " ";
    }

    // init
    for(int i=0; i<imformation[3]; i++){
        partitions[i] = -1;
        node_count[i] = 0;
    }

    // ReadFixNode
    for(int i=0; i<imformation[5]; i++){
        ss.str(""); ss.clear();     
        getline(myFile, line);
        ss << line;
        while(getline(ss, imfor, ' ')){
            v.push_back(imfor);
        } 
        ReadFixNode(v);
        v.clear();
    }

    // init cddt
    for(int i=0; i<imformation[3]; i++){
        if(partitions[i] != -1){
            cddt[i].push_back(partitions[i]);
        }
        else{
            for(int j=0; j<imformation[0]; j++){
                cddt[i].push_back(j);
            }
        }
    }

    for(int i=0; i<F.size(); i++){
        cout << F[i] << " ";
    }
    cout << endl;


    // 根據fix_node更新cddt
    while(F.size()){
        for(int j=0; j<Cnode[F[0]].size(); j++){
            if(partitions[Cnode[F[0]][j]] == -1){
                UpdateCandidate(Cnode[F[0]][j], F[0]);
            }            
        }
        F.erase(F.begin());
    }

    // 算出fpga_capacity_predict
    for(int i=0; i<imformation[3]; i++){ 
        for(int j=0; j<cddt[i].size(); j++){
            fpga_capacity_predict[cddt[i][j]]++;
        }
    }

    // using Q and R find partitions
    for(int i=0; i<imformation[3]; i++){
        if(partitions[i] == -1){
            Q.push(make_pair(make_pair(cddt[i].size(), nets[i].size()), i));    //依照1.candidate數量(少) 2.node鄰居數(少)
        }
    }
    for(int i=0; i<Q.top().first.first; i++){
        for(int j=0; j<nets[Q.top().second].size(); j++){
            if(partitions[nets[Q.top().second][j]] != -1){
                if(cddt[Q.top().second][i] != partitions[nets[Q.top().second][j]]){
                    external_degree[Q.top().second][cddt[Q.top().second][i]]++;
                }
            }
        }
        R[Q.top().second].push(make_pair(make_pair(external_degree[Q.top().second][cddt[Q.top().second][i]] ,fpga_capacity_predict[cddt[Q.top().second][i]]), cddt[Q.top().second][i]));    //依照1.external_degree數量(少) 2.predict(少)
    }

    for(int i=0; i<imformation[3]; i++){
        copy_cddt[i] = cddt[i];
    }

    if(imformation[3] > 20000){
        for(int i=0; i<imformation[3]; i++){  //1~200000
            if(partitions[i] == -1){
                for(int j=0; j<imformation[0]; j++){
                    if(fpga_capacity[j] < capacity){
                        partitions[i] = j;
                        fpga_capacity[j]++;
                        for(int k=0; k<nets[i].size(); k++){
                            if(partitions[nets[i][k]] == -1){
                                if(fpga_capacity[j] < capacity){
                                    partitions[nets[i][k]] = j;
                                    fpga_capacity[j]++;
                                }
                                else{
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    else{
        while(Q.size()){
            int v_node, v_fpga;
            v_node = Q.top().second; Q.pop();
            v_fpga = R[v_node].top().second; R[v_node].pop();
            cout << "Q_size: " << Q.size() << endl;
            traceback = 0;

            copy_cddt[v_node] = cddt[v_node];

            if(fpga_capacity[v_fpga] == capacity){
                traceback = 1;
            }
            else{
                UpateAndCheck(v_node, v_fpga);
            }

            if(traceback == 0){
                //cout << "======here2======" << endl;
                fpga_capacity[v_fpga]++;
                partitions[v_node] = v_fpga;
                cddt[v_node].clear();
                cddt[v_node].push_back(v_fpga);

                for(int i=0; i<Cnode[v_node].size(); i++){
                    if(partitions[Cnode[v_node][i]] == -1){
                        for(int j=0; j<copy_cddt[Cnode[v_node][i]].size(); j++){
                            fpga_capacity_predict[copy_cddt[Cnode[v_node][i]][j]]--;
                        }
                    }
                }
                for(int i=0; i<Cnode[v_node].size(); i++){
                    if(partitions[Cnode[v_node][i]] == -1){
                        for(int j=0; j<cddt[Cnode[v_node][i]].size(); j++){
                            fpga_capacity_predict[cddt[Cnode[v_node][i]][j]]++;
                        }
                    }
                }
                for(int i=0; i<copy_cddt[v_node].size(); i++){
                    fpga_capacity_predict[copy_cddt[v_node][i]]--;
                }
                fpga_capacity_predict[cddt[v_node][0]]++;
                
                while(Q.size()){
                    Q.pop();
                }
                memset(external_degree, 0, sizeof(external_degree));
                for(int i=0; i<imformation[3]; i++){
                    if(partitions[i] == -1){
                        Q.push(make_pair(make_pair(cddt[i].size(), nets[i].size()), i));
                    }
                } 

                for(int i=0; i<Q.top().first.first; i++){
                    for(int j=0; j<nets[Q.top().second].size(); j++){
                        if(partitions[nets[Q.top().second][j]] != -1){
                            if(cddt[Q.top().second][i] != partitions[nets[Q.top().second][j]]){
                                external_degree[Q.top().second][cddt[Q.top().second][i]]++;
                            }
                        }
                    }
                    R[Q.top().second].push(make_pair(make_pair(external_degree[Q.top().second][cddt[Q.top().second][i]] ,fpga_capacity_predict[cddt[Q.top().second][i]]), cddt[Q.top().second][i]));    //依照1.external_degree數量(少) 2.predict(少)
                }

            }
            else{
                //cout << "======here3======" << endl;
                cddt[v_node] = copy_cddt[v_node];
                for(int i=0; i<Cnode[v_node].size(); i++){
                    cddt[Cnode[v_node][i]] = copy_cddt[Cnode[v_node][i]];
                }

                for(int i=0; i<cddt[v_node].size(); i++){
                    if(cddt[v_node][i] == v_fpga){
                        cddt[v_node].erase(cddt[v_node].begin()+i);
                    }
                }

                int random_integer = 0 + rand() % imformation[0];

                if(cddt[v_node].size() == 0){
                    while(fpga_capacity[node_count[v_node]] == capacity && node_count[v_node]<imformation[0]){
                        node_count[v_node]++;
                    }
                    cddt[v_node].push_back(node_count[v_node]);
                    fpga_capacity_predict[node_count[v_node]]++;
                    node_count[v_node]++;
                }
                fpga_capacity_predict[v_fpga]--;
                memset(external_degree, 0, sizeof(external_degree));
                Q.push(make_pair(make_pair(cddt[v_node].size(), nets[v_node].size()), v_node));
                while(R[v_node].size()){
                    R[v_node].pop();
                }
                for(int i=0; i<Q.top().first.first; i++){
                    for(int j=0; j<nets[Q.top().second].size(); j++){
                        if(partitions[nets[Q.top().second][j]] != -1){
                            if(cddt[Q.top().second][i] != partitions[nets[Q.top().second][j]]){
                                external_degree[Q.top().second][cddt[Q.top().second][i]]++;
                            }
                        }
                    }
                    R[Q.top().second].push(make_pair(make_pair(external_degree[Q.top().second][cddt[Q.top().second][i]] ,fpga_capacity_predict[cddt[Q.top().second][i]]), cddt[Q.top().second][i]));    //依照1.external_degree數量(少) 2.predict(少)
                } 
            }
        }
    }

    // output answer
    for(int i=0; i<imformation[3]; i++){
        cout << partitions[i] << endl;
    }
    //write file
    ofstream outFile;
    outFile.open(argv[2]);
    for(int i=0; i<imformation[3]; i++){
        outFile << i << " " << partitions[i] << endl;
    }


    return 0;
}