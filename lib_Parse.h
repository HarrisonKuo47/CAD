#ifndef LIBPARSER_H
#define LIBPARSER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <istream>

using namespace std;

// 通用 LUT 結構，可存放任意 timing table（7×7）
struct LookupTable
{
    vector<double> index_1;        // x-axis values (size 7)
    vector<double> index_2;        // y-axis values (size 7)
    vector<vector<double>> values; // 7×7 matrix: values[row][col]
};
LookupTable parseTimingWithSharedIndex(const std::string &filename, const std::string &cell_name, const std::string &timing_block, const LookupTable &shared_index);
void parseValuesMatrix1(const string &filename, LookupTable &lut, const string &cell_name, const string &block_name);
void parseLookupTableIndex(const string &filename, LookupTable &ref_lut);

double calculatePrimaryInputDelayFromLUT(const LookupTable &lut, double output_cap, double input_tran);
double calculateDelayFromLUT(const LookupTable &lut, double output_cap, double input_tran);

class LibParser
{
public:
    unordered_map<string, unordered_map<string, double>> getCellInputCaps() const;
    void parseValuesMatrix(const string &filename, LookupTable &lut, const string &block_name);
    void parseLibFile(const string &filename);

private:
    unordered_map<string, unordered_map<string, double>> cell_input_caps;
    vector<double> parseStringToDoubleVector(const string &s);
};

#endif
