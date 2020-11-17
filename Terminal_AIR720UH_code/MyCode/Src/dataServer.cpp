#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include "dataServer.h"

using namespace std;
vector<array<int, 6>> datas;
stringstream ss;

void initDataServer(const char* filename){
    ss = stringstream();
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    string line;
    string sub;
    array<int, 6> dat;
    datas.clear();
    datas.reserve(1e6);
    while(buffer.rdstate() != stringstream::eofbit)
    {
        line.clear();
        getline(buffer, line);
        if (line.size() < 5) continue;

        stringstream ss;
        ss<<line;

        for (int i=0; i<6; ++i){
            ss>>sub;
            //sub = sub.substr(3);
            dat[i] = std::atoi(sub.data()+3);
        }
        datas.push_back(dat);
    }
}

int id = 0;

bool getData(int* dat){
    if (id >= datas.size()) return 0;
    memcpy(dat, datas[id++].data(), 6*sizeof(int));
    return 1;
}



void putData(float a, float b)
{
    ss<<a<<" "<<b<<endl;
}
int rid = 0;
void dump()
{
    std::ofstream fout;
    fout.open(std::to_string(rid++)+"res.txt");
    fout << ss.str() << std::endl;
    fout.close();
}
