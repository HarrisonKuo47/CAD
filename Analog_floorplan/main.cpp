#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <cctype>
#include <limits>
#include <fstream>
#include <sstream>

#include "./inc/floorplan.h" 

// 貪婪局部搜尋演算法
void run_greedy_search(Floorplan& initial_fp, int max_iter, const std::string& output_filename) {
    if (initial_fp.modules.empty()) {
        std::cout << "No modules found. Writing empty output." << std::endl;
        initial_fp.write_output_file(output_filename);
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    Floorplan best_solution = initial_fp;
    double best_cost = std::numeric_limits<double>::max();
    
    std::vector<int> initial_config(initial_fp.modules.size());
    for(size_t i = 0; i < initial_fp.modules.size(); ++i) {
        initial_config[i] = initial_fp.modules[i].chosen_shape_idx;
    }
    std::vector<int> best_config = initial_config;

    for (int iter = 0; iter < max_iter; ++iter) {
        
        Floorplan current_fp;
        current_fp.modules = initial_fp.modules;

        for (size_t i = 0; i < current_fp.modules.size(); ++i) {
            current_fp.modules[i].set_shape(initial_config[i]);
        }

        std::vector<Module> tall_modules, wide_modules;
        for (const auto& mod : current_fp.modules) {
            if (mod.height > mod.width) {
                tall_modules.push_back(mod);
            } else {
                wide_modules.push_back(mod);
            }
        }
        
        // 為了產生一個較為緊湊的PACKING，可以按高度或寬度排序
        std::sort(tall_modules.begin(), tall_modules.end(),
            [](const Module& a, const Module& b) { return a.height > b.height; });
        std::sort(wide_modules.begin(), wide_modules.end(),
            [](const Module& a, const Module& b) { return a.height > b.height; });
        
        current_fp.modules.clear();
        double left_column_y = 0.0;
        double left_column_width = 0.0;

        for (auto& m_left : tall_modules) {
            m_left.x = 0.0;
            m_left.y = left_column_y;
            current_fp.modules.push_back(m_left);
            left_column_y += m_left.height;
            left_column_width = std::max(left_column_width, m_left.width);
        }
        
        double right_column_y = 0.0;
        for (auto& m_right : wide_modules) {
            m_right.x = left_column_width; 
            m_right.y = right_column_y;
            current_fp.modules.push_back(m_right);
            right_column_y += m_right.height;
        }

        current_fp.calculate_bounding_box();
        double current_cost = current_fp.get_cost_area_ar();

        // 更新最佳解 
        if (current_cost < best_cost) {
            best_cost = current_cost;
            best_solution = current_fp; // 儲存整個 Floorplan 物件
            best_config = initial_config; // 儲存導致這個最佳解的形態組合
        }

        // 隨機改變一個模組的形態，為下一次迭代做準備
        if (!initial_fp.modules.empty()) {
            std::uniform_int_distribution<> rand_mod_dist(0, initial_fp.modules.size() - 1);
            int rand_mod_idx = rand_mod_dist(gen);
            if (initial_fp.modules[rand_mod_idx].shapes.size() > 1) {
                std::uniform_int_distribution<> rand_var_dist(0, initial_fp.modules[rand_mod_idx].shapes.size() - 1);
                initial_config[rand_mod_idx] = rand_var_dist(gen);
            }
        }
    }
    
    std::cout << "Greedy Search finished." << std::endl;
    std::cout << "Best cost found (Area/AR only): " << best_cost << std::endl;

    best_solution.write_output_file(output_filename);

    std::cout << "Output successfully written to " << output_filename << std::endl;
}


int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // 1. 解析命令列參數
    std::string input_filename;
    std::string output_filename;
    std::string arg_prefix_input = "input=";
    std::string arg_prefix_output = "output=";

    if (argc == 3) {
        std::string arg1 = argv[1];
        std::string arg2 = argv[2];
        if (arg1.rfind(arg_prefix_input, 0) == 0) {
            input_filename = arg1.substr(arg_prefix_input.length());
        } else if (arg2.rfind(arg_prefix_input, 0) == 0) {
            input_filename = arg2.substr(arg_prefix_input.length());
        }

        if (arg1.rfind(arg_prefix_output, 0) == 0) {
            output_filename = arg1.substr(arg_prefix_output.length());
        } else if (arg2.rfind(arg_prefix_output, 0) == 0) {
            output_filename = arg2.substr(arg_prefix_output.length());
        }
    }

    if (input_filename.empty() || output_filename.empty()) {
        std::cerr << "Usage: ./<executable_name> input=<input_file> output=<output_file>" << std::endl;
        return 1;
    }

    std::cout << "Input file: " << input_filename << std::endl;
    std::cout << "Output file: " << output_filename << std::endl;

    // 2. 建立 Floorplan 物件並解析輸入檔案
    Floorplan solution_fp;
    if (!solution_fp.parse_input_file(input_filename)) {
        std::cerr << "Error parsing input file. Exiting." << std::endl;
        return 1;
    }
    
    // 3. 執行貪婪局部搜尋演算法
    int max_iterations = 1000000; // 設定局部搜尋的迭代次數
    run_greedy_search(solution_fp, max_iterations, output_filename);

    return 0;
}
