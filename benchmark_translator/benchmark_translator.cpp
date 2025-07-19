#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>    //unordered_map<key(first), value(second)>，其中key存識別名稱(input,output,wire)，value存net name&pins
#include <set>

using namespace std;

struct Net
{
    string name;
    vector<string> pins;
};

struct Instance
{
    string name;
    string type;
};

void Netlist(const string &netlist_file, vector<Instance> &instances, unordered_map<string, Net> &nets)
{
    ifstream file(netlist_file);
    string line;
    while (getline(file, line))
    {
        istringstream iss(line);
        string keyword;
        iss >> keyword;   //每一行的第一個string(用空格分開的第一個string)
        if (keyword == "Inst")
        {
            Instance inst;  // 建立暫時物件來存放解析後的資料
            iss >> inst.name >> inst.type;  //當keyword為Inst時，後面會接著instance name和type(將資料解析到對應的欄位)
            instances.push_back(inst);  // 將格式化後的資料存入向量
        }
        else if (keyword == "NET")
        {
            Net net;
            iss >> net.name;  //net1, net2, ...
            getline(file, line);  //讀取NET那行的下一行(因為netlist的形式是NET行後面一定會接個一個PIN行)
            istringstream findpin(line.substr(4)); // Skip "PIN "
            string pin;
            while (getline(findpin, pin, ','))   //findpin字串根據逗號分割不同的string，並存入pin中
            {
                net.pins.push_back(pin);
            }

            nets[net.name] = net;
        }
    }
}

void Verilog(const string &verilog_file, const vector<Instance> &instances, const unordered_map<string, Net> &nets)
{
    ofstream file(verilog_file);
    file << "`timescale 1ns/1ps\n";
    file << "module " << verilog_file.substr(0, verilog_file.find('.')) << "(";

    set<string> inputs, outputs, wires;
    for (const auto &net : nets)
    {
        bool isInput = false, isOutput = false;
        for (const auto &pin : net.second.pins)
        {
            if (pin.find("inpt") != string::npos)
            {
                isInput = true;
            }
            else if (pin.find("outpt") != string::npos)
            {
                isOutput = true;
            }
        }
        if (isInput)
        {
            inputs.insert(net.first);
        }
        else if(isOutput)
        {
            outputs.insert(net.first);
        }
        else
        {
            wires.insert(net.first);
        }
    }

    for (const auto &input : inputs)
    {
        string wireName = "N" + input.substr(3);
        file << " " << wireName << ",";
    }
    for (const auto &output : outputs)
    {
        string wireName = "N" + output.substr(3);
        file << " " << wireName << ",";
    }

    file.seekp(-1, ios_base::end); 
    file << ");\n\n";

    // input
    file << "input";
    for (const auto &input : inputs)
    {
        string wireName = "N" + input.substr(3);
        file << " " << wireName << ",";
    }
    file.seekp(-1, ios_base::end); 
    file << ";\n\n";

    // output
    file << "output";
    for (const auto &output : outputs)
    {
        string wireName = "N" + output.substr(3);
        file << " " << wireName << ",";
    }
    file.seekp(-1, ios_base::end); 
    file << ";\n\n";

    // wire
    file << "wire ";
    for (const auto &wire : wires)
    {
        file << "N" << wire.substr(3) << ",";
    }
    file.seekp(-1, ios_base::end);
    file << ";\n\n";

    // gates
    for (const auto &inst : instances)
    {
        file << inst.type << " " << inst.type << "_" << inst.name.substr(3) << " (";
        vector<string> connections;
        string out;
        string in1;
        vector<string> inputs;
        //找出該gates的所有連接
        for (const auto &net : nets)
        {
            for (const auto &pin : net.second.pins)
            {
                if (pin.find(inst.name + "/") != string::npos)
                {
                    string port = pin.substr(pin.find("/") + 1);
                    string signal = "N" + net.first.substr(3);
                    if (port == "OUT1") 
                    {
                        out = signal;
                    }
                    else if (port == "IN1" && inst.type == "not") 
                    {
                        in1 = signal;
                    }
                    else if (port.substr(0,2) == "IN") 
                    {
                        //獲取輸入端口號碼
                        int index = stoi(port.substr(2)) - 1;
                        //確保 vector 大小足夠
                        if (inputs.size() <= index) 
                        {
                           inputs.resize(index + 1); 
                        }
                        inputs[index] = signal;
                    }

                }
            }
        }

        //根據邏輯閘的輸入數量輸出
        if (inst.type == "not")
        {
            file << out << ", " << in1;
        }
        else{
            // 其他邏輯閘至少有兩個輸入
            file << out;
            for (size_t i = 0; i < inputs.size(); ++i)
            {
                if (!inputs[i].empty())
                {
                    file << ", " << inputs[i];
                }
            }
        }
        file << ");\n";
    }

    file << "\nendmodule\n";
}

int main(int argc, char *argv[])
{
    vector<Instance> instances;
    unordered_map<string, Net> nets;
    set<string> inputs, outputs, wires;

    for (int i = 1; i < argc; i++) {
        string netlist_file = argv[i];
        string verilog_file;
        size_t dot_pos = netlist_file.find_last_of(".");

        ifstream test_file(netlist_file);
        verilog_file = netlist_file.substr(0, dot_pos) + ".v";

        Netlist(netlist_file, instances, nets);
        Verilog(verilog_file, instances, nets);
    }
    
    return 0;
}
