#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>


struct Shape {
    double width;          
    double height;         
    int col_multiple;      
    int row_multiple;      

    Shape(double w, double h, int cm, int rm)
        : width(w), height(h), col_multiple(cm), row_multiple(rm) {
    }
};

// 代表一個電路模組
struct Module {
    std::string name; 
    std::vector<Shape> shapes; 
    int chosen_shape_idx;      
    double x, y;             
    double width, height;     
    int current_col_multiple;
    int current_row_multiple;

    Module(std::string n) : name(std::move(n)), chosen_shape_idx(0), x(0.0), y(0.0), width(0.0), height(0.0), current_col_multiple(0), current_row_multiple(0) {}

    // 設定模組使用的形態
    void set_shape(int shape_idx) {
        if (shape_idx >= 0 && static_cast<size_t>(shape_idx) < shapes.size()) { // 進行安全的比較
            chosen_shape_idx = shape_idx;
            width = shapes[shape_idx].width;
            height = shapes[shape_idx].height;
            current_col_multiple = shapes[shape_idx].col_multiple;
            current_row_multiple = shapes[shape_idx].row_multiple;
        }
    }

    double get_right_x() const { return x + width; }
    double get_top_y() const { return y + height; }
    double get_center_x() const { return x + width / 2.0; }
    double get_center_y() const { return y + height / 2.0; }
};

#endif // MODULE_H
