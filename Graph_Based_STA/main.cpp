#include <iostream>
#include <unordered_map>
#include <string>
#include <cassert>
#include <fstream>
#include "inc/lib_Parse.h"
#include "inc/Graph.h"

int main(int argc, char *argv[])
{

    // 解析 library 檔案
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <input_verilog_file> <library_file>" << endl;
        return 1; // 返回錯誤碼
    }

    // --- 修改點 3: 從 argv 獲取文件名 ---
    string verilog_filename = argv[1];
    string library_filename = argv[2];

    cout << "Input Verilog File: " << verilog_filename << endl;
    cout << "Library File: " << library_filename << endl;

    LibParser lib_parser;
    lib_parser.parseLibFile(library_filename);
    auto cell_caps = lib_parser.getCellInputCaps();
    LookupTable shared_lut;
    parseLookupTableIndex(library_filename, shared_lut);
    // 解析 timing table

    Graph g;
    g.parseVerilog(verilog_filename);
    g.assignInputCapacitance(cell_caps);
    g.calculateOutputLoading();
    string casename = verilog_filename;
    size_t last_slash = casename.find_last_of("/\\"); // 處理路徑
    if (last_slash != string::npos)
    {
        casename = casename.substr(last_slash + 1);
    }
    size_t dot_pos = casename.rfind('.');
    if (dot_pos != string::npos)
    {
        casename = casename.substr(0, dot_pos);
    }

    string load_report_filename = "Load_113521042_" + casename + ".txt";
    string delay_report_filename = "Delay_113521042_" + casename + ".txt";
    string path_report_filename = "Path_113521042_" + casename + ".txt";

    g.dumpLoadReport(load_report_filename);

    // void parseLookupTableIndex(const string &filename, LookupTable &ref_lut)
    // void parseValuesMatrix1(const string& filename, LookupTable& lut, const string& block_name)
    LookupTable lut_nor_rise = parseTimingWithSharedIndex("test_lib.lib", "NOR2X1", "cell_rise", shared_lut);
    LookupTable lut_inv_rise = parseTimingWithSharedIndex("test_lib.lib", "INVX1", "cell_rise", shared_lut);
    LookupTable lut_nand_rise = parseTimingWithSharedIndex("test_lib.lib", "NANDX1", "cell_rise", shared_lut);

    LookupTable lut_nor_fall = parseTimingWithSharedIndex("test_lib.lib", "NOR2X1", "cell_fall", shared_lut);
    LookupTable lut_inv_fall = parseTimingWithSharedIndex("test_lib.lib", "INVX1", "cell_fall", shared_lut);
    LookupTable lut_nand_fall = parseTimingWithSharedIndex("test_lib.lib", "NANDX1", "cell_fall", shared_lut);

    LookupTable lut_nor_rise_t = parseTimingWithSharedIndex("test_lib.lib", "NOR2X1", "rise_transition", shared_lut);
    LookupTable lut_inv_rise_t = parseTimingWithSharedIndex("test_lib.lib", "INVX1", "rise_transition", shared_lut);
    LookupTable lut_nand_rise_t = parseTimingWithSharedIndex("test_lib.lib", "NANDX1", "rise_transition", shared_lut);

    LookupTable lut_nor_fall_t = parseTimingWithSharedIndex("test_lib.lib", "NOR2X1", "fall_transition", shared_lut);
    LookupTable lut_inv_fall_t = parseTimingWithSharedIndex("test_lib.lib", "INVX1", "fall_transition", shared_lut);
    LookupTable lut_nand_fall_t = parseTimingWithSharedIndex("test_lib.lib", "NANDX1", "fall_transition", shared_lut);

    vector<string> topological_order = g.getTopologicalOrder();

    g.calculateAllCellTimings(topological_order,
                              lut_nor_rise, lut_inv_rise, lut_nand_rise,
                              lut_nor_fall, lut_inv_fall, lut_nand_fall,
                              lut_nor_rise_t, lut_inv_rise_t, lut_nand_rise_t,
                              lut_nor_fall_t, lut_inv_fall_t, lut_nand_fall_t);

    g.dumpDelayReport(delay_report_filename);
    g.findAndReportPaths(path_report_filename);

    return 0;
}

