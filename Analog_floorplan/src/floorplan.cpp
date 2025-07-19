#include "../inc/floorplan.h" 
#include <iostream>    
#include <fstream>     
#include <sstream>     
#include <vector>   
#include <string>      
#include <algorithm>   
#include <cmath>       
#include <iomanip>    
#include <cctype>      

using namespace std;

// 從模組名稱中提取數字索引 
// 這個函式假設模組名稱中包含一個數字，並返回該數字的整數值
int extractModuleIndex(const std::string& name) {
    size_t pos = std::string::npos;
    for (size_t i = 0; i < name.length(); ++i) {
        if (isdigit(name[i])) {
            pos = i;
            break;
        }
    }
    try {
        return (pos != std::string::npos) ? std::stoi(name.substr(pos)) : -1;
    } catch (const std::exception&) {
        return -1; 
    }
}


Floorplan::Floorplan() : chip_width(0.0), chip_height(0.0), chip_area(0.0), inl_value(0.0) {}

bool Floorplan::parse_input_file(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Cannot open input file: " << filename << endl;
        return false;
    }
    this->modules.clear();
    string line;
    while (getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;
        stringstream ss(line);
        string device_name;
        ss >> device_name;
        if (device_name.empty()) continue;

        Module current_module(device_name);
        char paren_open, paren_close;
        double w, h;
        int cm, rm;
        string shapes_part;
        getline(ss, shapes_part);
        stringstream shape_stream(shapes_part);

        while (shape_stream >> paren_open && paren_open == '(') {
            if (shape_stream >> w >> h >> cm >> rm >> paren_close && paren_close == ')') {
                current_module.shapes.emplace_back(w, h, cm, rm);
            } else {
                cerr << "Warning: Malformed variant for module " << device_name << endl;
                break;
            }
        }
        if (!current_module.shapes.empty()) {
            current_module.set_shape(0);
            this->modules.push_back(move(current_module));
        }
    }
    infile.close();
    return true;
}

void Floorplan::calculate_bounding_box() {
    if (modules.empty()) {
        chip_width = 0.0;
        chip_height = 0.0;
        chip_area = 0.0;
        return;
    }

    double min_x = numeric_limits<double>::max();
    double max_x = numeric_limits<double>::lowest();
    double min_y = numeric_limits<double>::max();
    double max_y = numeric_limits<double>::lowest();

    for (const auto& module : modules) {
        min_x = min(min_x, module.x);
        max_x = max(max_x, module.get_right_x());
        min_y = min(min_y, module.y);
        max_y = max(max_y, module.get_top_y());
    }

    if (min_x > max_x || min_y > max_y) {
        chip_width = 0.0;
        chip_height = 0.0;
    } else {
        chip_width = (max_x - min_x);
        chip_height = (max_y - min_y);
    }
    chip_area = chip_width * chip_height;
}

bool Floorplan::check_overlap() const {
    for (size_t i = 0; i < modules.size(); ++i) {
        for (size_t j = i + 1; j < modules.size(); ++j) {
            const auto& m1 = modules[i];
            const auto& m2 = modules[j];
            bool x_overlap = (m1.get_right_x() > m2.x + 1e-9) && (m1.x < m2.get_right_x() - 1e-9);
            bool y_overlap = (m1.get_top_y() > m2.y + 1e-9) && (m1.y < m2.get_top_y() - 1e-9);
            if (x_overlap && y_overlap) {
                return true;
            }
        }
    }
    return false;
}


std::vector<double> Floorplan::calculate_squared_distances(double Xc, double Yc, const std::vector<Module>& sorted_modules) const {
    std::vector<double> distances_sq;
    distances_sq.reserve(sorted_modules.size());
    for (const auto& module : sorted_modules) {
        double dx = module.get_center_x() - Xc;
        double dy = module.get_center_y() - Yc;
        distances_sq.push_back(dx * dx + dy * dy);
    }
    return distances_sq;
}

std::vector<double> Floorplan::accumulate_distances(const std::vector<double>& squared_distances) const {
    std::vector<double> cumulative_sum;
    if (squared_distances.empty()) return cumulative_sum;
    cumulative_sum.reserve(squared_distances.size());
    double current_sum = 0.0;
    for (double d_sq : squared_distances) {
        current_sum += d_sq;
        cumulative_sum.push_back(current_sum);
    }
    return cumulative_sum;
}
    
RegressionResult Floorplan::linear_regression(const std::vector<double>& S_actual) const {
    int N = S_actual.size();
    if (N < 2) {
        return {0.0, (N == 1 ? S_actual[0] : 0.0)}; // 對 N=0 和 N=1 的簡化處理
    }

    double sum_n = 0.0, sum_S = 0.0, sum_nS = 0.0, sum_n_sq = 0.0;
    for (int i = 0; i < N; ++i) {
        double n_val = static_cast<double>(i + 1);
        double S_val = S_actual[i];
        sum_n += n_val;
        sum_S += S_val;
        sum_nS += n_val * S_val;
        sum_n_sq += n_val * n_val;
    }

    double N_double = static_cast<double>(N);
    double denominator = (N_double * sum_n_sq - sum_n * sum_n);
    double a = 0.0, b = 0.0;

    if (std::abs(denominator) < 1e-9) {
        if (N > 0) b = sum_S / N_double;
    } else {
        a = (N_double * sum_nS - sum_n * sum_S) / denominator;
        b = (sum_S - a * sum_n) / N_double;
    }
    return {a, b}; 
}

void Floorplan::calculate_inl() {
    if (modules.size() < 2) { // 模組數小於2，INL 可視為0
        this->inl_value = 0.0;
        return;
    }
    double X_min_inl = std::numeric_limits<double>::max();
    double X_max_inl = std::numeric_limits<double>::lowest();
    double Y_min_inl = std::numeric_limits<double>::max();
    double Y_max_inl = std::numeric_limits<double>::lowest();

    for (const auto& module : this->modules) {
        X_min_inl = std::min(X_min_inl, module.x);
        X_max_inl = std::max(X_max_inl, module.get_right_x());
        Y_min_inl = std::min(Y_min_inl, module.y);
        Y_max_inl = std::max(Y_max_inl, module.get_top_y());
    }
    
    if (X_min_inl > X_max_inl || Y_min_inl > Y_max_inl) {
        this->inl_value = 0.0;
        return;
    }

    double Xc = (X_min_inl + X_max_inl) / 2.0;
    double Yc = (Y_min_inl + Y_max_inl) / 2.0;

    std::vector<Module> sorted_modules = this->modules;
    // 按模組名稱中的數字進行排序
    // 如果模組名稱中有數字，則按數字排序，否則按字典序排序
    std::sort(sorted_modules.begin(), sorted_modules.end(), 
              [](const Module& a, const Module& b) {
                  int num_a = extractModuleIndex(a.name);
                  int num_b = extractModuleIndex(b.name);
                  if (num_a != -1 && num_b != -1) {
                      if (num_a != num_b) return num_a < num_b;
                  }
                  // 如果數字相同，或有任一無法解析，退回字典序
                  return a.name < b.name; 
              });

    std::vector<double> dk_sq = calculate_squared_distances(Xc, Yc, sorted_modules);
    std::vector<double> S_actual_values = accumulate_distances(dk_sq);
    
    RegressionResult regression = linear_regression(S_actual_values);
    double a = regression.a;
    double b = regression.b;

    double max_deviation = 0.0;
    for (size_t i = 0; i < S_actual_values.size(); ++i) {
        double n_val = static_cast<double>(i + 1);
        double S_ideal_n = a * n_val + b;
        max_deviation = std::max(max_deviation, std::abs(S_actual_values[i] - S_ideal_n));
    }
    this->inl_value = max_deviation;
}

// 成本函數
double Floorplan::calculate_aspect_ratio_penalty() const {
    if (this->chip_height < 1e-9) {
        return (this->chip_width < 1e-9) ? 0.0 : 1e9;
    }
    double aspect_ratio_val = std::max(this->chip_width / this->chip_height, this->chip_height / this->chip_width);
    double f_AR = 0.0;
    if (aspect_ratio_val > 2.0) {
        f_AR = aspect_ratio_val - 2.0;
    }
    return f_AR;
}

double Floorplan::get_cost_area_ar() {
    return this->chip_area * (1.0 + calculate_aspect_ratio_penalty());
}

double Floorplan::get_total_cost(double weight_inl) {
    this->calculate_bounding_box();
    this->calculate_inl();
    double cost_area_ar = get_cost_area_ar();
    double total_c = cost_area_ar + weight_inl * this->inl_value;
    if (this->check_overlap()) {
        total_c += std::numeric_limits<double>::max() / 2.0;
    }
    return total_c;
}

// 輸出檔案
bool Floorplan::write_output_file(const std::string& filename) {
    if (!modules.empty()) {
    // 步驟 1: 找出當前佈局的最小 x 和 y 座標
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    for (const auto& module : this->modules) {
        min_x = std::min(min_x, module.x);
        min_y = std::min(min_y, module.y);
    }

    // 步驟 2: 如果佈局沒有對齊到 (0,0)，則平移所有模組
    if (min_x > 1e-9 || min_y > 1e-9) {
        for (auto& module : this->modules) {
            module.x -= min_x;
            module.y -= min_y;
        }
    }
}
    this->calculate_bounding_box();
    this->calculate_inl();

    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Cannot open output file: " << filename << std::endl;
        return false;
    }

    outfile << std::fixed; 
    outfile << std::setprecision(4) << this->chip_area << std::endl;
    outfile << std::setprecision(2) << this->chip_width << " " << this->chip_height << std::endl;
    outfile << std::setprecision(2) << this->inl_value << std::endl;

    std::vector<Module> sorted_output_modules = this->modules;
    // 輸出時也使用按數字比較的排序邏輯
    std::sort(sorted_output_modules.begin(), sorted_output_modules.end(),
         [](const Module& a, const Module& b) {
             int num_a = extractModuleIndex(a.name);
             int num_b = extractModuleIndex(b.name);
             if (num_a != -1 && num_b != -1) {
                 if (num_a != num_b) return num_a < num_b;
             }
             return a.name < b.name;
         });

    for (const auto& module : sorted_output_modules) {
        outfile << module.name << " "
                << std::setprecision(3) << module.x << " "
                << std::setprecision(3) << module.y << " "
                << "(" << std::setprecision(2) << module.width << " "
                << std::setprecision(2) << module.height << " "
                << module.current_col_multiple << " "
                << module.current_row_multiple << ")"
                << std::endl;
    }
    outfile.close();
    return outfile.good();
}
