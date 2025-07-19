#include "../inc/Graph.h"
#include "../inc/lib_Parse.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <regex>
#include <cctype>
#include <iomanip>
#include <vector>
#include <queue>
#include <map>
#include <cmath>


using namespace std;

Cell *Graph::getCell(const string &name)
{ // 回傳值為Cell的pointer，利用查找Cell.name去從Graph裡找到對應的Cell結構
    if (cells.find(name) != cells.end())
        return &cells[name]; //&cells[name] 是取該 cell 的記憶體位置，也就是傳回指標（Cell*）。
    return nullptr;
}

// 1. Parse Verilog Netlist
void Graph::parseVerilog(const string &filename)
{
    ifstream file(filename);
    string line;

    while (getline(file, line))
    {
        // 移除block comment, line comment
        size_t comment_pos = line.find("//");
        if (comment_pos != string::npos)
        {
            line = line.substr(0, comment_pos);
        }
        // 這會將 line 中所有滿足 isspace 條件的字元「移到尾端」，留下不需要刪除的部分在前面，再用line.erase(new_end, line.end())擦掉後面空格;
        size_t startComment = line.find("/*");
        while (startComment != string::npos)
        {
            size_t endComment = line.find("*/", startComment);
            if (endComment != string::npos)
            {
                line.erase(startComment, endComment - startComment + 2);
                startComment = line.find("/*"); // 更新搜索位置，尋找下一個可能的註釋
            }
            else
            {
                // If no closing comment found, remove rest of the line
                line = line.substr(0, startComment);
                break;
            }
        }

        size_t multlinescomment = line.find("*/");
        while (multlinescomment != string::npos)
        {
            line.erase(0, multlinescomment + 2);
            break;
        }

        // Remove leading whitespace
        size_t firstNonSpace = line.find_first_not_of(" \t\n\r\f\v");
        if (firstNonSpace != string::npos)
        {
            line = line.substr(firstNonSpace);
        }
        if (line.empty())
            continue;

        istringstream iss(line);
        string keyword;
        iss >> keyword;
        if (keyword == "input")
        {
            string net;
            while (iss >> net)
            {
                if (net.back() == ',' || net.back() == ';')
                {
                    net.pop_back();
                }
                primary_inputs.push_back(net);
            }
        }
        else if (keyword == "NOR2X1")
        {
            Cell cell;
            cell.type = "NOR";
            size_t pin_pos = line.find("(");
            string instance_declaration_line = line.substr(line.find(keyword), pin_pos);
            cell.name = instance_declaration_line;
            size_t start = line.find("(");
            size_t end = line.find(");");

            string pins = line.substr(start + 1, end - start - 1);
            istringstream pinsStream(pins);
            string pin;

            // 讀取每個 pin 的定義
            while (getline(pinsStream, pin, ','))
            {
                string in1;
                string in2;
                string out;
                size_t openBracket = pin.find('(');
                size_t closeBracket = pin.find(')');

                if (pin.find(".A1") != string::npos)
                {
                    in1 = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.input_nets.push_back(in1);
                }
                if (pin.find(".A2") != string::npos)
                {
                    in2 = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.input_nets.push_back(in2);
                }
                if (pin.find(".ZN") != string::npos)
                {
                    out = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.output_net = out;
                }
                cells[cell.name] = cell; // 將 cell 加入到 cells map 中
            }
        }
        else if (keyword == "NANDX1")
        {
            Cell cell;
            cell.type = "NAND";
            size_t pin_pos = line.find("(");
            string instance_declaration_line = line.substr(line.find(keyword), pin_pos);
            cell.name = instance_declaration_line;
            size_t start = line.find("(");
            size_t end = line.find(");");

            string pins = line.substr(start + 1, end - start - 1);
            istringstream pinsStream(pins);
            string pin;

            // 讀取每個 pin 的定義
            while (getline(pinsStream, pin, ','))
            {

                string in1;
                string in2;
                string out;
                size_t openBracket = pin.find('(');
                size_t closeBracket = pin.find(')');

                if (pin.find(".A1") != string::npos)
                {
                    in1 = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.input_nets.push_back(in1);
                }
                if (pin.find(".A2") != string::npos)
                {
                    in2 = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.input_nets.push_back(in2);
                }
                if (pin.find(".ZN") != string::npos)
                {
                    out = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.output_net = out;
                }
                cells[cell.name] = cell; // 將 cell 加入到 cells map 中
            }
        }
        else if (keyword == "INVX1")
        {
            Cell cell;
            cell.type = "INV";
            size_t pin_pos = line.find("(");
            string instance_declaration_line = line.substr(line.find(keyword), pin_pos);
            cell.name = instance_declaration_line;
            size_t start = line.find("(");
            size_t end = line.find(");");

            string pins = line.substr(start + 1, end - start - 1);
            istringstream pinsStream(pins);
            string pin;

            // 讀取每個 pin 的定義
            while (getline(pinsStream, pin, ','))
            {
                string in;
                string out;
                size_t openBracket = pin.find('(');
                size_t closeBracket = pin.find(')');

                if (pin.find(".I") != string::npos)
                {
                    in = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.input_nets.push_back(in);
                }
                if (pin.find(".ZN") != string::npos)
                {
                    out = pin.substr(openBracket + 1, closeBracket - openBracket - 1);
                    cell.output_net = out;
                }
                cells[cell.name] = cell; // 將 cell 加入到 cells map 中
            }
        }
        else if (keyword == "output")
        {
            string net;
            while (iss >> net)
            {
                if (net.back() == ',' || net.back() == ';')
                {
                    net.pop_back();
                }
                primary_outputs.push_back(net);
            }
        }
    }
}

void Graph::assignInputCapacitance(const unordered_map<string, unordered_map<string, double>> &cell_caps)
{
    for (auto &cell_pair : cells)
    {
        Cell &cell = cell_pair.second;
        string cellType;
        if (cell.type == "NOR")
        {
            cellType = "NOR2X1";
        }
        else if (cell.type == "NAND")
        {
            cellType = "NANDX1";
        }
        else if (cell.type == "INV")
        {
            cellType = "INVX1";
        }

        if (cell_caps.count(cellType))
        {
            const auto &pin_caps = cell_caps.at(cellType);
            // 根據 cell type 分配電容值
            if (cell.type == "NOR" || cell.type == "NAND")
            {
                if (pin_caps.find("A1") != pin_caps.end())
                {
                    cell.input_capacitance["A1"] = pin_caps.at("A1");
                }
                if (pin_caps.find("A2") != pin_caps.end())
                {
                    cell.input_capacitance["A2"] = pin_caps.at("A2");
                }
            }
            else if (cell.type == "INV")
            {
                if (pin_caps.find("I") != pin_caps.end())
                {
                    cell.input_capacitance["I"] = pin_caps.at("I");
                }
            }
        }
    }
}

void Graph::calculateOutputLoading()
{
    // 首先建立網路到單元的映射
    for (const auto &cell_pair : cells)
    {
        const Cell &cell = cell_pair.second;
        // 將每個輸入網路映射到使用它的單元
        for (const string &input : cell.input_nets)
        {
            net_to_drivers[input].push_back(cell.name); // input_net 被 cell.name 這個 cell 的 input pin 使用
        }
        // 將輸出網路映射到驅動它的單元
        if (!cell.output_net.empty())
        {
            net_to_fanouts[cell.output_net].push_back(cell.name); // output_net 由 cell.name 這個 cell 驅動
        }
    }

    // 計算每個單元的輸出負載
    for (auto &cell_pair : cells)
    {
        Cell &cell = cell_pair.second;
        double total_load = 0;

        // 檢查是否為primary output
        bool is_primary_output = false;
        for (const auto &po : primary_outputs)
        {
            if (cell.output_net == po)
            {
                total_load = 0.030000;
                is_primary_output = true;
            }
        }
        cell.output_load = total_load;

        // 找出所有使用此單元輸出的其他單元
        if (!is_primary_output)
        {
            const string &output_net = cell.output_net;
            if (net_to_drivers.find(output_net) != net_to_drivers.end())
            {
                for (const string &fanout_cell_name : net_to_drivers[output_net])
                {
                    Cell &fanout_cell = cells[fanout_cell_name];
                    if (fanout_cell.type == "INV")
                    {
                        total_load += fanout_cell.input_capacitance["I"];
                    }
                    else
                    {
                        for (size_t i = 0; i < fanout_cell.input_nets.size(); ++i)
                        {
                            if (fanout_cell.input_nets[i] == output_net)
                            {
                                string pin_name = "A" + to_string(i + 1);
                                total_load += fanout_cell.input_capacitance[pin_name];
                            }
                        }
                    }
                }
            }
        }
        cell.output_load = total_load;
    }
}

int extract_instance_number(const string &full_name) {
    // 去除尾端空白
    string trimmed = full_name;
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);

    // 抓最後一段（通常是 instance 名稱）
    size_t last_space = trimmed.find_last_of(" ");
    string instance_part = (last_space != string::npos) ? trimmed.substr(last_space + 1) : trimmed;

    // 找出第一個數字開始的位置
    size_t digit_pos = 0;
    while (digit_pos < instance_part.size() && !isdigit(instance_part[digit_pos])) {
        ++digit_pos;
    }

    if (digit_pos == instance_part.size()) {
        cerr << "Warning: No digit found in instance_part: [" << instance_part << "] (from full: [" << full_name << "])" << endl;
        return -1;
    }

    // 往後抓出連續數字
    size_t end_pos = digit_pos;
    while (end_pos < instance_part.size() && isdigit(instance_part[end_pos])) {
        ++end_pos;
    }

    string number_str = instance_part.substr(digit_pos, end_pos - digit_pos);
    return stoi(number_str);
}

void Graph::dumpLoadReport(const string &filename)
{
    ofstream outFile(filename);
    if (!outFile)
    {
        cerr << "Error: Unable to open file " << filename << endl;
        return;
    }
    vector<pair<string, double>> sorted_cells;
    for (const auto &cell_pair : cells)
    {
        sorted_cells.emplace_back(cell_pair.first, cell_pair.second.output_load);
    }

    // 排序：output_load 降序，若相同則根據 instance 名稱中的數字升序
    sort(sorted_cells.begin(), sorted_cells.end(),
        [](const pair<string, double> &a, const pair<string, double> &b) {
            if (a.second != b.second)
                return a.second > b.second;

            int a_num = extract_instance_number(a.first);
            int b_num = extract_instance_number(b.first);
            return a_num < b_num;
        });

    // 輸出排序結果
    for (const auto &p : sorted_cells)
    {
        string name_to_output = p.first;
        size_t first_space = name_to_output.find(' ');
        if (first_space != string::npos)
        {
            name_to_output = name_to_output.substr(first_space + 1);
            size_t open_paren = name_to_output.find('(');
            if (open_paren != string::npos)
            {
                name_to_output = name_to_output.substr(0, open_paren);
            }
        }

        // 清除前後空白
        name_to_output.erase(0, name_to_output.find_first_not_of(" \t"));
        name_to_output.erase(name_to_output.find_last_not_of(" \t") + 1);

        outFile << name_to_output << " " << fixed << setprecision(6) << p.second << endl;
    }

    outFile.close();
}
bool Graph::isCellPIDriven(const Cell &cell) const
{
    if (cell.input_nets.empty() && (cell.type == "NOR" || cell.type == "NAND" || cell.type == "INV"))
    {
        return false;
    }
    for (const string &net : cell.input_nets)
    {
        bool is_pi = false;
        for (const string &pi_net : primary_inputs)
        {
            if (net == pi_net)
            {
                is_pi = true;
                break;
            }
        }
        if (!is_pi)
        {
            return false;
        }
    }
    return true;
}

unordered_map<string, string> Graph::getNetToDrivingCellMap() const
{
    unordered_map<string, string> driver_map;
    for (const auto &pair : cells)
    {
        const Cell &cell = pair.second;
        if (!cell.output_net.empty())
        {
            driver_map[cell.output_net] = cell.name;
        }
    }
    return driver_map;
}

vector<string> Graph::getTopologicalOrder() const
{
    vector<string> order;
    map<string, int> in_degree; // 使用 map 以便調試時觀察 key 的順序
    queue<string> q;

    // 1. 初始化所有 cell 的入度 (只計算來自其他 gates 的 input)
    for (const auto &pair : cells)
    {
        const string &cell_name = pair.first;
        in_degree[cell_name] = 0; // 初始化
        for (const string &input_net : pair.second.input_nets)
        {
            // 如果這個 input_net 不是 Primary Input，則它必然由另一個 cell 驅動
            bool is_pi_net = false;
            for (const string &pi : primary_inputs)
            {
                if (input_net == pi)
                {
                    is_pi_net = true;
                    break;
                }
            }
            if (!is_pi_net)
            {
                in_degree[cell_name]++;
            }
        }
    }

    // 2. 將所有實際入度為 0 的 cell（通常是 PI-driven gates）加入隊列
    for (const auto &pair : cells)
    {
        const string &cell_name = pair.first;
        if (in_degree.at(cell_name) == 0)
        {
            q.push(cell_name);
        }
    }

    // 3. Kahn's Algorithm
    while (!q.empty())
    {
        string u_name = q.front();
        q.pop();
        order.push_back(u_name);

        if (cells.find(u_name) == cells.end())
        {
            cerr << "Warning: Cell " << u_name << " from queue not in cells map." << endl;
            continue;
        }
        const Cell &u_cell = cells.at(u_name);
        const string &u_output_net = u_cell.output_net;

        if (!u_output_net.empty())
        {
            // net_to_drivers[output_net_of_u_cell] 是 u_cell 的 fanout cells
            if (net_to_drivers.count(u_output_net))
            {
                for (const string &v_name : net_to_drivers.at(u_output_net))
                {
                    if (in_degree.count(v_name))
                    {
                        in_degree[v_name]--;
                        if (in_degree[v_name] == 0)
                        {
                            q.push(v_name);
                        }
                    }
                    else
                    {
                        cerr << "Warning: Fanout cell " << v_name << " not found in in_degree map." << endl;
                    }
                }
            }
        }
    }

    if (order.size() != cells.size())
    {
        if (!cells.empty())
        {
            cerr << "Error: Circuit might have a cycle or topological sort failed!" << endl;
            cerr << "  Sorted " << order.size() << " cells, but total cells are " << cells.size() << endl;
        }
        return {};
    }
    return order;
}

double Graph::getMaxInputTransition(const vector<string> &input_nets,
                                    const unordered_map<string, double> &current_net_transitions) const
{
    double max_trans = 0.0;
    for (const string &net_name : input_nets)
    {
        bool is_pi = false;
        for (const string &pi : primary_inputs)
        {
            if (net_name == pi)
            {
                is_pi = true;
                break;
            }
        }

        if (is_pi)
        {
            // PI 的 transition time 對於下一級的 input transition 貢獻是 0
            max_trans = std::max(max_trans, 0.0); // max_trans 初始為 0
        }
        else
        {
            auto it = current_net_transitions.find(net_name);
            if (it != current_net_transitions.end())
            {
                max_trans = std::max(max_trans, it->second);
            }
            else
            {
                // 在正確的拓撲排序流程中，這裡不應該發生
                cerr << "CRITICAL WARNING: Transition time for internal net " << net_name
                     << " not found during getMaxInputTransition! Assuming 0.0 for it." << endl;
            }
        }
    }
    return max_trans;
}

void Graph::calculateAllCellTimings(
    const vector<string> &topological_order,
    const LookupTable &lut_nor_rise, const LookupTable &lut_inv_rise, const LookupTable &lut_nand_rise,
    const LookupTable &lut_nor_fall, const LookupTable &lut_inv_fall, const LookupTable &lut_nand_fall,
    const LookupTable &lut_nor_rise_t, const LookupTable &lut_inv_rise_t, const LookupTable &lut_nand_rise_t,
    const LookupTable &lut_nor_fall_t, const LookupTable &lut_inv_fall_t, const LookupTable &lut_nand_fall_t)
{
    net_to_transition_time.clear(); // 開始計算前清空
    net_arrival_time.clear();       // 清空前一級 net 的 arrival time 記錄

    for (const string &pi_net : primary_inputs)
    {
        net_arrival_time[pi_net] = 0.0;
        // PI nets 沒有 "driving cell output transition"，所以 net_to_transition_time 不包含 PI nets
    }

    for (const string &cell_name : topological_order)
    {
        if (cells.find(cell_name) == cells.end())
        {
            cerr << "Error: Cell " << cell_name << " from topological order not found in cells map!" << endl;
            continue;
        }
        Cell &cell = cells.at(cell_name); // 需要修改 cell，所以用引用

        double output_cap = cell.output_load;
        double input_tran_for_current_cell = 0.0;      // 將用於查表的 input transition
        double latest_arrival_time_to_input_pin = 0.0; // 到達此 cell 輸入 pin 的最晚時間
        double delay_r, delay_f, trans_r, trans_f;

        bool pi_driven = isCellPIDriven(cell);

        if (pi_driven)
        {
            input_tran_for_current_cell = 0.0; // PI 的 input transition 為 0

            if (cell.type == "NOR")
            {
                delay_r = calculatePrimaryInputDelayFromLUT(lut_nor_rise, output_cap, input_tran_for_current_cell);
                delay_f = calculatePrimaryInputDelayFromLUT(lut_nor_fall, output_cap, input_tran_for_current_cell);
                trans_r = calculatePrimaryInputDelayFromLUT(lut_nor_rise_t, output_cap, input_tran_for_current_cell);
                trans_f = calculatePrimaryInputDelayFromLUT(lut_nor_fall_t, output_cap, input_tran_for_current_cell);
            }
            else if (cell.type == "INV")
            {
                delay_r = calculatePrimaryInputDelayFromLUT(lut_inv_rise, output_cap, input_tran_for_current_cell);
                delay_f = calculatePrimaryInputDelayFromLUT(lut_inv_fall, output_cap, input_tran_for_current_cell);
                trans_r = calculatePrimaryInputDelayFromLUT(lut_inv_rise_t, output_cap, input_tran_for_current_cell);
                trans_f = calculatePrimaryInputDelayFromLUT(lut_inv_fall_t, output_cap, input_tran_for_current_cell);
            }
            else if (cell.type == "NAND")
            {
                delay_r = calculatePrimaryInputDelayFromLUT(lut_nand_rise, output_cap, input_tran_for_current_cell);
                delay_f = calculatePrimaryInputDelayFromLUT(lut_nand_fall, output_cap, input_tran_for_current_cell);
                trans_r = calculatePrimaryInputDelayFromLUT(lut_nand_rise_t, output_cap, input_tran_for_current_cell);
                trans_f = calculatePrimaryInputDelayFromLUT(lut_nand_fall_t, output_cap, input_tran_for_current_cell);
            }
            else
            {
                cerr << "Unknown PI-driven cell type: " << cell.type << " for cell " << cell.name << endl;
                continue;
            }
        }
        else
        {                                     // Non-PI driven
            double temp_latest_at_pin = -1.0; // 用於比較
            double transition_of_latest_pin = 0.0;
            string dbg_critical_net_for_timing_decision = "";
            // 從 net_to_transition_time 獲取輸入 net 的 transition time
            input_tran_for_current_cell = getMaxInputTransition(cell.input_nets, net_to_transition_time);

            for (const string &input_net : cell.input_nets)
            {
                double pin_at_from_this_net = 0.0;
                double transition_from_this_net_driver = 0.0;

                bool is_this_input_net_pi = (find(primary_inputs.begin(), primary_inputs.end(), input_net) != primary_inputs.end());

                if (is_this_input_net_pi)
                { // 如果某個輸入是 PI
                    pin_at_from_this_net = net_arrival_time.count(input_net) ? net_arrival_time.at(input_net) : 0.0;
                    transition_from_this_net_driver = 0.0; // PI 的 transition 為 0
                }
                else
                { // 內部 net
                    if (net_arrival_time.count(input_net))
                    {
                        pin_at_from_this_net = net_arrival_time.at(input_net) + 0.005; // AT@source + wire delay
                    }
                    else
                    {
                        cerr << "TimingCalc CRITICAL: AT for internal net " << input_net << " (input to " << cell.name << ") not found!" << endl;
                        pin_at_from_this_net = 0.005; // 假設至少有 wire delay，但 AT 來源未知
                    }
                    if (net_to_transition_time.count(input_net))
                    {
                        transition_from_this_net_driver = net_to_transition_time.at(input_net);
                    }
                    else
                    {
                        cerr << "TimingCalc CRITICAL: Transition for internal net " << input_net << " (input to " << cell.name << ") not found!" << endl;
                        // transition_from_this_net_driver 保持 0
                    }
                }

                if (temp_latest_at_pin < 0 || pin_at_from_this_net > temp_latest_at_pin)
                {
                    temp_latest_at_pin = pin_at_from_this_net;
                    transition_of_latest_pin = transition_from_this_net_driver; // <-- 選擇最晚到達訊號的 transition
                }
            }
            input_tran_for_current_cell = transition_of_latest_pin;
            latest_arrival_time_to_input_pin = temp_latest_at_pin;

            if (cell.type == "NOR")
            {
                delay_r = calculateDelayFromLUT(lut_nor_rise, output_cap, input_tran_for_current_cell);
                delay_f = calculateDelayFromLUT(lut_nor_fall, output_cap, input_tran_for_current_cell);
                trans_r = calculateDelayFromLUT(lut_nor_rise_t, output_cap, input_tran_for_current_cell);
                trans_f = calculateDelayFromLUT(lut_nor_fall_t, output_cap, input_tran_for_current_cell);
            }
            else if (cell.type == "INV")
            {
                // 你的 U10 (INV) 輸入 input_tran 是 0.054006，輸出 Delay 0.035620, Trans 0.042872 (Output 0)
                // 檢查 calculateDelayFromLUT 的 INV 部分
                delay_r = calculateDelayFromLUT(lut_inv_rise, output_cap, input_tran_for_current_cell);
                delay_f = calculateDelayFromLUT(lut_inv_fall, output_cap, input_tran_for_current_cell);
                trans_r = calculateDelayFromLUT(lut_inv_rise_t, output_cap, input_tran_for_current_cell);
                trans_f = calculateDelayFromLUT(lut_inv_fall_t, output_cap, input_tran_for_current_cell);
            }
            else if (cell.type == "NAND")
            {
                delay_r = calculateDelayFromLUT(lut_nand_rise, output_cap, input_tran_for_current_cell);
                delay_f = calculateDelayFromLUT(lut_nand_fall, output_cap, input_tran_for_current_cell);
                trans_r = calculateDelayFromLUT(lut_nand_rise_t, output_cap, input_tran_for_current_cell);
                trans_f = calculateDelayFromLUT(lut_nand_fall_t, output_cap, input_tran_for_current_cell);
            }
            else
            {
                cerr << "Unknown non-PI-driven cell type: " << cell.type << " for cell " << cell.name << endl;
                continue;
            }
        }

        int worst_case_val; // 避免與 Cell 結構中的成員衝突
        if (delay_r >= delay_f)
        {
            cell.delay = delay_r;
            cell.output_transition = trans_r;
            worst_case_val = 1;
        }
        else
        {
            cell.delay = delay_f;
            cell.output_transition = trans_f;
            worst_case_val = 0;
        }
        cell.worst_case_output = worst_case_val; // 存儲 worst case output
        cell.arrival_time_at_output = latest_arrival_time_to_input_pin + cell.delay;

        if (!cell.output_net.empty())
        {
            net_to_transition_time[cell.output_net] = cell.output_transition;
            net_arrival_time[cell.output_net] = cell.arrival_time_at_output;
        }
    }
}

// 新增 dumpDelayReport 函數
void Graph::dumpDelayReport(const string &filename) const
{
    ofstream outFile(filename);
    if (!outFile)
    {
        cerr << "Error: Unable to open file " << filename << endl;
        return;
    }

    vector<const Cell *> sorted_cells_ptrs;
    for (const auto &cell_pair : cells)
    {
        sorted_cells_ptrs.push_back(&cell_pair.second);
    }

    sort(sorted_cells_ptrs.begin(), sorted_cells_ptrs.end(),
       [](const Cell *a, const Cell *b)
       {
         if (std::abs(a->delay - b->delay) > 1e-9)
           {
             return a->delay > b->delay; // Delay: descending
           }

           int num_a = extract_instance_number(a->name);
           int num_b = extract_instance_number(b->name);
           return num_a < num_b; // ascending order
       });

    for (const Cell *cell_ptr : sorted_cells_ptrs)
    {
    string instance_name = cell_ptr->name;
    instance_name.erase(instance_name.find_last_not_of(" \t") + 1);

    size_t space_pos = instance_name.find_last_of(" ");
    if (space_pos != string::npos)
        instance_name = instance_name.substr(space_pos + 1);


    outFile << instance_name << " "
            << cell_ptr->worst_case_output << " "
            << fixed << setprecision(6) << cell_ptr->delay << " "
            << fixed << setprecision(6) << cell_ptr->output_transition << endl;
    }
    outFile.close();
}

void Graph::findAndReportPaths(const string &output_filename)
{
    // 初始化 Net Arrival Times for Primary Inputs
    unordered_map<string, double> net_at; // K: net_name, V: arrival_time 在這個 net 的源頭
    for (const string &pi_net : primary_inputs)
    {
        net_at[pi_net] = 0.0; // PI 的 arrival time 設為 0
    }

    // 獲取拓撲順序
    vector<string> order = getTopologicalOrder();
    if (order.empty() && !cells.empty())
    { // cells 是 private 成員，所以用 getCellCount()
        cerr << "PathCalc Error: Topological sort failed or graph is inconsistent. Cannot find paths." << endl;
        return;
    }
    if (cells.empty())
    { // 或者用 getCellCount() == 0
        // 根據PA要求，即使沒有路徑，也要產生特定格式的輸出文件
        ofstream emptyOutFile(output_filename);
        if (emptyOutFile)
        {
            emptyOutFile << "Longest delay = 0.000000, the path is: " << endl;
            emptyOutFile << "Shortest delay = 0.000000, the path is: " << endl;
            emptyOutFile.close();
        }
        return;
    }

    // 按照拓撲順序計算所有 Cell 的 Output Arrival Time 和 Critical Input ---
    for (const string &cell_name : order)
    {
        Cell &current_cell = cells.at(cell_name); // 需要修改 Cell 成員，所以用引用

        double max_arrival_at_cell_inputs = 0.0; // 對於 PI-driven, 輸入來自 AT=0 的 PI
        string determined_critical_input_net = "";

        // 遍歷當前 cell 的所有輸入 net
        for (const string &input_net_name : current_cell.input_nets)
        {
            double current_pin_arrival_time = 0.0;
            bool is_pi_net = (find(primary_inputs.begin(), primary_inputs.end(), input_net_name) != primary_inputs.end());

            if (is_pi_net)
            {
                // 如果是 PI net，它的 arrival time 是 0 (從 net_at map 中獲取)
                current_pin_arrival_time = net_at.count(input_net_name) ? net_at.at(input_net_name) : 0.0;
            }
            else
            { // 不是 PI net，說明它由其他 cell 的 output 驅動
                if (net_at.count(input_net_name))
                {
                    // Arrival time at this pin = AT of the net + wire delay
                    current_pin_arrival_time = net_at.at(input_net_name) + 0.005;
                }
                else
                {
                    // 嚴重錯誤：如果按拓撲順序，這個 net 的 AT 應該已經計算好了
                    cerr << "PathCalc FATAL: Arrival time for internal net " << input_net_name
                         << " (input to " << cell_name << ") not found! Check order or connectivity." << endl;
                    // 為了程序繼續，可以假設一個值，但這會導致結果錯誤
                    current_pin_arrival_time = -1e9; // 用一個極小值，使其不被選為 critical (或者極大值來暴露問題)
                }
            }

            // 更新最晚到達的輸入 pin 和對應的 net
            if (determined_critical_input_net.empty() || current_pin_arrival_time > max_arrival_at_cell_inputs)
            {
                max_arrival_at_cell_inputs = current_pin_arrival_time;
                determined_critical_input_net = input_net_name;
            }
        }

        current_cell.critical_input_net_for_at = determined_critical_input_net;
        // current_cell.delay 是在 Step 2 中計算的 cell 的 worst-case propagation delay
        current_cell.arrival_time_at_output = max_arrival_at_cell_inputs + current_cell.delay;

        if (!current_cell.output_net.empty())
        {
            net_at[current_cell.output_net] = current_cell.arrival_time_at_output; // 更新 net 的 arrival time
        }
    }

    // 找到 Primary Outputs 的最長和最短路徑延遲並回溯
    double longest_overall_delay = 0.0; // 初始化為0，以處理沒有有效路徑的情況 (PA要求0.0)
    vector<string> longest_overall_path_nets;
    double shortest_overall_delay = numeric_limits<double>::max();
    vector<string> shortest_overall_path_nets;
    bool found_any_valid_po_path = false;

    unordered_map<string, string> net_driver_cell_map = getNetToDrivingCellMap(); // output_net -> driving_cell_name

    for (const string &po_net : primary_outputs)
    {
        double final_po_arrival_time;
        if (net_at.count(po_net))
        {
            // 從驅動 PO 的 cell 的 output net 到 PO 本身，還有一段 wire delay
            final_po_arrival_time = net_at.at(po_net) + 0.005;
            found_any_valid_po_path = true;
        }
        else
        {
            cerr << "PathCalc Warning: Arrival time for primary output net " << po_net << " not calculated. Skipping this PO." << endl;
            continue;
        }

        // 回溯路徑
        vector<string> current_traced_path_nets;
        string current_net_in_trace = po_net;

        while (true)
        {
            current_traced_path_nets.insert(current_traced_path_nets.begin(), current_net_in_trace);

            bool is_current_net_pi = (find(primary_inputs.begin(), primary_inputs.end(), current_net_in_trace) != primary_inputs.end());
            if (is_current_net_pi)
            {
                break; // 到達 PI，路徑結束
            }

            // 找到驅動 current_net_in_trace 的 cell
            if (!net_driver_cell_map.count(current_net_in_trace))
            {
                current_traced_path_nets.clear(); // 標記路徑無效
                break;
            }
            const string &driving_cell_name = net_driver_cell_map.at(current_net_in_trace);

            if (cells.find(driving_cell_name) == cells.end())
            {
                current_traced_path_nets.clear();
                break;
            }
            const Cell &driving_cell = cells.at(driving_cell_name);

            if (driving_cell.critical_input_net_for_at.empty())
            {
                // 對於 PI-driven gate，它的 critical input net 可能是空的，因為所有輸入都是同時到達(AT=0)
                // 我們需要選擇一個實際的 PI net 作為路徑的一部分
                if (isCellPIDriven(driving_cell) && !driving_cell.input_nets.empty())
                {
                    // 根據 PA3："If the instance has multiple input signals that has same arrival time, you can output either one of them."
                    // 我們取第一個輸入 net (它必須是 PI net)
                    current_net_in_trace = driving_cell.input_nets[0];
                    if (find(primary_inputs.begin(), primary_inputs.end(), current_net_in_trace) == primary_inputs.end())
                    {
                        current_traced_path_nets.clear();
                        break;
                    }
                }
            }
            else
            {
                current_net_in_trace = driving_cell.critical_input_net_for_at;
            }
        }

        if (current_traced_path_nets.empty())
            continue; // 如果回溯失敗，跳過此路徑

        if (final_po_arrival_time > longest_overall_delay)
        {
            longest_overall_delay = final_po_arrival_time;
            longest_overall_path_nets = current_traced_path_nets;
        }
        if (final_po_arrival_time < shortest_overall_delay)
        {
            shortest_overall_delay = final_po_arrival_time;
            shortest_overall_path_nets = current_traced_path_nets;
        }
    }

    if (!found_any_valid_po_path)
    {                                 // 如果沒有找到任何有效路徑到PO
        shortest_overall_delay = 0.0; // PA 可能期望對空的路徑延遲為0
        longest_overall_delay = 0.0;
        longest_overall_path_nets.clear();
        shortest_overall_path_nets.clear();
    }

    // 輸出結果到文件
    ofstream outFile(output_filename);
    outFile << "Longest delay = " << fixed << setprecision(6) << longest_overall_delay - 0.005 << ", the path is: ";
    for (size_t i = 0; i < longest_overall_path_nets.size(); ++i)
    {
        outFile << longest_overall_path_nets[i] << (i == longest_overall_path_nets.size() - 1 ? "" : " -> ");
    }
    outFile << endl;

    outFile << "Shortest delay = " << fixed << setprecision(6) << shortest_overall_delay - 0.005 << ", the path is: ";
    for (size_t i = 0; i < shortest_overall_path_nets.size(); ++i)
    {
        outFile << shortest_overall_path_nets[i] << (i == shortest_overall_path_nets.size() - 1 ? "" : " -> ");
    }
    outFile << endl;

    outFile.close();
}

