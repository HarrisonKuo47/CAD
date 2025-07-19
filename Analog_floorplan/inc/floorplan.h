#ifndef FLOORPLAN_H
#define FLOORPLAN_H

#include <string>       
#include <vector>       
#include <limits>       
#include <algorithm>    

#include "module.h"    


struct RegressionResult {
    double a; // 斜率
    double b; // 截距
};


// 代表整個晶片的佈局
class Floorplan {
public:
    std::vector<Module> modules;
    double chip_width;
    double chip_height;
    double chip_area;
    double inl_value;   

    Floorplan(); 


    bool parse_input_file(const std::string& filename);
    void calculate_bounding_box();
    bool check_overlap() const; 
    void calculate_inl();
    double calculate_aspect_ratio_penalty() const;
    double get_cost_area_ar();
    double get_total_cost(double weight_inl); 
    bool write_output_file(const std::string& filename);

private:
    // INL 計算的輔助函數 - 宣告 
    std::vector<double> calculate_squared_distances(double Xc, double Yc, const std::vector<Module>& sorted_modules) const;
    std::vector<double> accumulate_distances(const std::vector<double>& squared_distances) const;
    RegressionResult linear_regression(const std::vector<double>& S_actual) const;
};

#endif 
