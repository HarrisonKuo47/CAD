#include "../inc/lib_Parse.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm> //For std::remove
#include <regex>
#include <iomanip>

using namespace std;

void LibParser::parseLibFile(const string &filename)
{
    ifstream file(filename);
    string line;
    string current_cell;
    string input_pin;
    bool inpin = 0;

    while (getline(file, line))
    {
        line.erase(0, line.find_first_not_of(" \t"));
        // 找cell名稱(NAND, NOR, INV)
        if (line.find("cell (") == 0)
        { // 因為已經刪去前導空格，所以找到"cell ("的話一定是在那行的第0個字元
            size_t start = line.find("(") + 1;
            size_t end = line.find(")");
            current_cell = line.substr(start, end - start);
            // 初始化這個cell的map
            cell_input_caps[current_cell] = unordered_map<string, double>();
        }
        // 找input pin(A1, A2)
        else if (line.find("pin(") == 0)
        {
            size_t start = line.find("(") + 1;
            size_t end = line.find(")");
            input_pin = line.substr(start, end - start);
            inpin = 1;
        }
        else if (inpin && line.find("}") != string::npos)
        {
            inpin = 0;
            input_pin.clear();
        }
        // 找input pin的capacitance
        else if (inpin && line.find("capacitance") != string::npos && line.find(":") != string::npos)
        {
            size_t colon = line.find(":");
            size_t semicolon = line.find(";");

            string cap_str = line.substr(colon + 1, semicolon - colon - 1);

            // 清除前後空白字元
            cap_str.erase(0, cap_str.find_first_not_of(" \t"));
            cap_str.erase(cap_str.find_last_not_of(" \t\r\n") + 1);

            double capacitance = stod(cap_str);
            if (!current_cell.empty() && !input_pin.empty())
            {
                cell_input_caps[current_cell][input_pin] = capacitance;
            }
        }
    }
}

unordered_map<string, unordered_map<string, double>> LibParser::getCellInputCaps() const
{
    return cell_input_caps;
}

// 自由函式：把 "0.01,0.02,..." 轉成 vector<double>
static vector<double> splitToDoubles(const string &raw)
{
    vector<double> result;
    string cleaned = raw;
    // 去掉所有雙引號
    cleaned.erase(remove(cleaned.begin(), cleaned.end(), '"'), cleaned.end()); // remove(...) 把目標字元移到字串尾端erase(...) 真正把尾端的垃圾清除
    // 將逗號改成空白，方便 stringstream 解析
    replace(cleaned.begin(), cleaned.end(), ',', ' ');
    stringstream ss(cleaned);
    double v;
    while (ss >> v)
    {
        result.push_back(v);
    }
    return result;
}

void parseLookupTableIndex(const string &filename, LookupTable &ref_lut)
{
    ifstream file(filename);
    string line;
    bool inTable = false;

    while (getline(file, line))
    {
        if (line.find("lu_table_template(table10)") != string::npos)
        {
            inTable = true;
            continue;
        }
        if (inTable)
        {
            if (line.find("index_1") != string::npos)
            {
                size_t l = line.find('('), r = line.find(')');
                string raw = line.substr(l + 1, r - l - 1);
                ref_lut.index_1 = splitToDoubles(raw);
            }
            else if (line.find("index_2") != string::npos)
            {
                size_t l = line.find('('), r = line.find(')');
                string raw = line.substr(l + 1, r - l - 1);
                ref_lut.index_2 = splitToDoubles(raw);
            }
            else if (line.find("}") != string::npos)
            {
                break;
            }
        }
    }
}

void parseValuesMatrix1(const string &filename, LookupTable &lut, const string &cell_name, const string &block_name)
{
    lut.values.clear();
    ifstream file(filename);
    string line;
    bool inCell = false, inBlock = false;

    while (getline(file, line))
    {
        if (!inCell && line.find("cell (" + cell_name + ")") != string::npos)
        {
            inCell = true;
        }
        else if (inCell)
        {
            if (line.find("cell (") != string::npos && line.find(cell_name) == string::npos)
                break;

            if (!inBlock && line.find(block_name) != string::npos && line.find("{") != string::npos)
            {
                inBlock = true;
                continue;
            }

            if (inBlock)
            {
                if (line.find("}") != string::npos)
                    break;
                size_t s = line.find('"'), e = line.rfind('"');
                if (s != string::npos && e != string::npos && e > s)
                {
                    string raw = line.substr(s + 1, e - s - 1);
                    lut.values.push_back(splitToDoubles(raw));
                }
            }
        }
    }
}

LookupTable parseTimingWithSharedIndex(const string &filename,const string &cell_name,const string &timing_block,const LookupTable &shared_index)
{
    LookupTable lut;
    lut.index_1 = shared_index.index_1;
    lut.index_2 = shared_index.index_2;
    parseValuesMatrix1(filename, lut, cell_name, timing_block);
    return lut;
}

double calculatePrimaryInputDelayFromLUT(const LookupTable &lut, double output_cap, double input_tran)
{
    // 第一行代表 input_transition_time = index_2[0]，要對 index_2 外插回 input_transition_time = 0
    double x1 = lut.index_2[0]; // e.g., 0.0208
    double x2 = lut.index_2[1]; // e.g., 0.0336

    // 先針對每個 index_1 對應的 delay 值做一次 input_transition_time 外插
    vector<double> extrapolated_row;
    for (size_t i = 0; i < lut.index_1.size(); ++i)
    {
        double y1 = lut.values[0][i]; // row 0, input_transition_time = 0.0208
        double y2 = lut.values[1][i]; // row 1, input_transition_time = 0.0336

        // 線性外插
        double y0 = y1 + (y1 - y2) * (x1 / (x2 - x1)); // 向左外推到 x=0
        extrapolated_row.push_back(y0);
    }

    // 現在在 extrapolated_row 上針對 output_capacitance 做內插
    for (size_t i = 1; i < lut.index_1.size(); ++i)
    {
        double x_prev = lut.index_1[i - 1];
        double x_next = lut.index_1[i];
        if (output_cap <= x_next)
        {
            double y_prev = extrapolated_row[i - 1];
            double y_next = extrapolated_row[i];
            // 線性內插
            double delay = y_prev + (y_next - y_prev) * (output_cap - x_prev) / (x_next - x_prev);
            return delay;
        }
    }

    // 若 output_cap > 最大 index_1，做外插
    size_t n = lut.index_1.size();
    double x_last = lut.index_1[n - 2];
    double x_max = lut.index_1[n - 1];
    double y_last = extrapolated_row[n - 2];
    double y_max = extrapolated_row[n - 1];
    return y_max + (y_max - y_last) * (output_cap - x_max) / (x_max - x_last);
}

double calculateDelayFromLUT(const LookupTable &lut, double output_cap, double input_tran)
{
    const vector<double> &x_axis = lut.index_1; // output_capacitance
    const vector<double> &y_axis = lut.index_2; // input_transition_time
    const vector<vector<double>> &values = lut.values;

    // 找 x (output cap) 所在的 index 範圍
    size_t xi = 0;
    while (xi + 1 < x_axis.size() && output_cap > x_axis[xi + 1])
        ++xi;

    // 找 y (input transition) 所在的 index 範圍
    size_t yi = 0;
    while (yi + 1 < y_axis.size() && input_tran > y_axis[yi + 1])
        ++yi;

    // 若在邊界之外，夾在最後兩點之間做外插
    if (xi + 1 >= x_axis.size())
        xi = x_axis.size() - 2;
    if (yi + 1 >= y_axis.size())
        yi = y_axis.size() - 2;

    double x1 = x_axis[xi], x2 = x_axis[xi + 1];
    double y1 = y_axis[yi], y2 = y_axis[yi + 1];

    double Q11 = values[yi][xi];
    double Q21 = values[yi][xi + 1];
    double Q12 = values[yi + 1][xi];
    double Q22 = values[yi + 1][xi + 1];

    // 雙線性內插公式
    double denom = (x2 - x1) * (y2 - y1);
    double interpolated =
        Q11 * (x2 - output_cap) * (y2 - input_tran) +
        Q21 * (output_cap - x1) * (y2 - input_tran) +
        Q12 * (x2 - output_cap) * (input_tran - y1) +
        Q22 * (output_cap - x1) * (input_tran - y1);
    return interpolated / denom;
}
