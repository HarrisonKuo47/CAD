#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include "lib_Parse.h"

using namespace std;

struct Cell
{
    string name;
    string type;
    string output_net;
    vector<string> input_nets;
    unordered_map<string, double> input_capacitance; // from lib

    double output_load = 0;
    double delay = 0;             // <-- 記錄 cell 的 delay
    double output_transition = 0; // <-- 記錄 cell 的 output transition (rise/fall max)
    int worst_case_output;

    double arrival_time_at_output = 0.0; // 到達此 cell output net 的時間 (不含驅動下一級的 wire delay)
    string critical_input_net_for_at;    // 導致最晚 arrival time 到達此 cell 的那個 input net
};

class Graph
{
public:
    void parseVerilog(const string &filename);
    void assignInputCapacitance(const unordered_map<string, unordered_map<string, double>> &cell_caps);
    void calculateOutputLoading();
    void dumpLoadReport(const string &filename);

    void calculateAllCellTimings(
        const vector<string> &topological_order, // 接收拓撲順序
        const LookupTable &lut_nor_rise, const LookupTable &lut_inv_rise, const LookupTable &lut_nand_rise,
        const LookupTable &lut_nor_fall, const LookupTable &lut_inv_fall, const LookupTable &lut_nand_fall,
        const LookupTable &lut_nor_rise_t, const LookupTable &lut_inv_rise_t, const LookupTable &lut_nand_rise_t,
        const LookupTable &lut_nor_fall_t, const LookupTable &lut_inv_fall_t, const LookupTable &lut_nand_fall_t);

    void findAndReportPaths(const string &output_filename);
    vector<string> getTopologicalOrder() const;
    size_t getCellCount() const { return cells.size(); }
    void dumpDelayReport(const string &filename) const;

private:
    unordered_map<string, Cell> cells;                    // cells 是一個 unordered_map<string, Cell>，它把 cell 名（如 "U3"）對應到 Cell 結構。
    unordered_map<string, vector<string>> net_to_drivers; // net -> cell (drivers)
    unordered_map<string, vector<string>> net_to_fanouts; // net -> cells (fanouts)
    vector<string> primary_inputs;                        // 存儲主要輸入網路
    vector<string> primary_outputs;                       // 存儲主要輸出網路
    unordered_map<string, double> net_to_transition_time;
    unordered_map<string, double> net_arrival_time; // Key: net_name, Value: arrival time at this net's source

    Cell *getCell(const string &name);

    unordered_map<string, string> getNetToDrivingCellMap() const; // output_net -> driving cell name
    bool isCellPIDriven(const Cell &cell) const;
    double getMaxInputTransition(const vector<string> &input_nets,
                                 const unordered_map<string, double> &current_net_transitions) const;
};

#endif
