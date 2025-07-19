#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std;

struct Cell
{
    string name;
    int group = -1;             // 0: A , 1: B, -1: unassigned
    int gain = 0;               // gain of the cell
    bool locked = false;        // locked or not
    unordered_set<int> netlist; // 代表netlist用int來表示，這樣可以節省記憶體空間
};

struct Net
{
    string name;
    unordered_set<string> celllist; // cells this net belongs to
};

unordered_map<string, Cell> cells; // key = cell name, value = class Cell裡面存的東西(cell name, group, gain, locked, netlist)
vector<Net> nets;                  // 資料型態為Net的向量nets, 其中Net為我定義的動態陣列, 裡面包含這個Net的名稱(n1,n2,...)以及這個Net所連接的Celllist的名稱(cell1, cell2...)

int cut_size()
{
    int cut = 0;
    for (size_t i = 0; i < nets.size(); ++i)
    {
        bool groupA = false;
        bool groupB = false;
        for (unordered_set<string>::iterator it = nets[i].celllist.begin(); it != nets[i].celllist.end(); ++it)
        {                             // 對於每個net，檢查它連接的所有cells
            int g = cells[*it].group; // 指標指向cell的名稱，並且把這個cell的int group值給g
            if (g == 0)
                groupA = true;
            else if (g == 1)
                groupB = true;
        }
        if (groupA && groupB)
            cut++;
    }
    return cut;
}

void parse_input(const string &filename)
{
    ifstream infile(filename);
    string line;
    while (getline(infile, line))
    {
        if (line.substr(0, 3) != "NET")
            continue;
        istringstream iss(line);
        string netword, netname, token;
        iss >> netword >> netname >> token;

        Net net;
        net.name = netname;

        while (iss >> token)
        {
            if (token == "{" || token == "}")
                continue;
            if (token.back() == '}')
                token.pop_back();
            net.celllist.insert(token);
            if (!cells.count(token))
            {
                cells[token].name = token;
            }
            cells[token].netlist.insert(nets.size());
        }
        nets.push_back(net);
    }
}

void initial_partition()
{ // 這個函數的功能是將cell分成兩組，A組和B組，並且將cell的group設為0或1，這樣可以方便後續的計算cut size和gain，使用Greddy Algorithm來進行cell的最初分組

    unordered_set<string> groupA, visited;
    unordered_map<string, int> cellScore;

    //  Step1. 先確認好seed為cells中 unordered_map的第一個cell
    string seed = cells.begin()->first; // 取出第一個cell的名稱作為seed
    groupA.insert(seed);                // 將seed放入groupA中
    visited.insert(seed);               // 將seed放入visited中，表示這個cell已經被訪問過了
    cout << "Seed: " << seed << endl;

    //  Step2. 建立初始分數(與A組有多少net連結)
    for (unordered_set<int>::iterator it = cells[seed].netlist.begin(); it != cells[seed].netlist.end(); ++it)
    {
        int netID = *it; // netID為pointer it指向的記憶體位置上儲存得值
        for (unordered_set<string>::iterator netit = nets[netID].celllist.begin(); netit != nets[netID].celllist.end(); ++netit)
        { // 用&代表是reference，直接利用memory中的資料而不用多做一次copy

            if (visited.count(*netit))
                continue;
            cellScore[*netit]++;
        }
    }
    //   Step3. 反覆將與A關聯最強的cell加入A
    int limit = cells.size() / 2;
    while (groupA.size() < limit)
    {
        string bestCell = "";
        int maxScore = -1;
        for (unordered_map<string, int>::iterator it = cellScore.begin(); it != cellScore.end(); ++it)
        {
            const string &cname = it->first;
            int score = it->second;
            if (visited.count(cname))
                continue;
            if (score > maxScore)
            {
                bestCell = cname;
                maxScore = score;
            }
        }
        /*若在cellScore中已經找不到任何可以被放進groupA的cell(與seedA有關係的cell都被放到GroupA裡了)
        且groupA中沒超過一半的cell，則從GroupB隨機抓一些cell到GroupA中*/
        if (bestCell == "")
        {
            for (unordered_map<string, Cell>::iterator it = cells.begin(); it != cells.end(); ++it)
            {
                const string &cname = it->first;
                if (!visited.count(cname))
                { // 當cname儲存的cell不在visited中時變為true, 也就是不在visited的冗員cells隨便加一個進bestcell中作為group A的partition
                    bestCell = cname;
                    break;
                }
            }
        }

        groupA.insert(bestCell);
        visited.insert(bestCell);
        for (unordered_set<int>::iterator it = cells[bestCell].netlist.begin(); it != cells[bestCell].netlist.end(); ++it)
        {
            int netID = *it;
            for (unordered_set<string>::iterator netit = nets[netID].celllist.begin(); netit != nets[netID].celllist.end(); ++netit)
            {
                if (!visited.count(*netit))
                {
                    cellScore[*netit]++;
                }
            }
        }
    }

    //  Step4. 回傳分組資訊給cells更新每個cells中的group(0 = A, 1 = B)
    for (unordered_map<string, Cell>::iterator it = cells.begin(); it != cells.end(); ++it)
    {
        const string &cname = it->first;
        Cell &c = it->second;
        if (groupA.count(cname))
            c.group = 0;
        else
            c.group = 1;
    }
}

void calculate_gains()
{
    for (unordered_map<string, Cell>::iterator it = cells.begin(); it != cells.end(); ++it)
    {
        Cell &cell = it->second; // 建立Cell物件的引用，這樣對c的修改會直接反映在原始物件Cell上，可以避免複製整個Cell物件，提高效能及節省既已體使用
        cell.gain = 0;           // reset gain to 0
        for (unordered_set<int>::iterator net_it = cell.netlist.begin(); net_it != cell.netlist.end(); ++net_it)
        {
            int from_group = 0, to_group = 0;
            for (unordered_set<string>::iterator cname = nets[*net_it].celllist.begin(); cname != nets[*net_it].celllist.end(); ++cname)
            {
                if (*cname == cell.name)
                    continue;
                if (cells[*cname].group == cell.group)
                    from_group++; // 該cell跟選中的seed cell在同一個group，from_group++
                else
                    to_group++; // 該cell跟選中的seed cell不同個group，to_group++
            }
            cell.gain = (to_group - from_group);
        }
    }
}

void update_gains(const std::string &seedCellName)
{
    // 移動 cell 後，更新與它連接的所有 net 上的其他 cell 的 gain，這邊命名移動的cell為seedCell
    Cell &seedCell = cells[seedCellName];
    for (int netID : seedCell.netlist)
    {
        for (const std::string &otherCellName : nets[netID].celllist)
        {
            if (otherCellName != seedCellName)
            {
                Cell &otherCell = cells[otherCellName];
                int from_group = 0, to_group = 0;
                for (const std::string &cname : nets[netID].celllist)
                {
                    if (cname == otherCellName)
                        continue;
                    if (cells[cname].group == otherCell.group)
                        from_group++;
                    else
                        to_group++;
                }
                // gain = (連到另一組的cell數 - 同組cell數)
                otherCell.gain = (to_group - from_group);
            }
        }
    }
}

void Fiduccia_Mattheyses_Algorithm()
{
    vector<string> moveOrder;
    vector<int> gainHistory;
    for (size_t step = 0; step < cells.size(); ++step)
    {

        string bestCell = "";
        int bestGain = -999999;

        // 找到當前未被 lock 且 gain 最大的 cell
        for (auto &pair : cells)
        {
            Cell &c = pair.second; // 隨意選的cell，利用reference c來操作這個Cell
            if (!c.locked && c.gain > bestGain)
            {
                bestGain = c.gain;
                bestCell = c.name;
            }
        }

        // 計算每個 group 中的 cell 數量
        int countA = 0, countB = 0;
        for (const auto &pair : cells)
        {
            if (pair.second.group == 0)
                countA++;
            else if (pair.second.group == 1)
                countB++;
        }

        int n = cells.size(); // 總 cell 數
        int imbalance = abs(countA - countB);

        if (imbalance < n / 5)
        {
            // 可以進行 cell 移動
            cout << "Best cell to move: " << bestCell << " with gain: " << bestGain << endl;
        }
        else if (bestCell == "")
        {
            cout << "No more movable cells.\n";
            break;
        }
        else
        {
            cout << "Imbalance too large. Skip moving to keep balance.\n";
            break;
        }

        // 將選中的 cell 移動到另一組
        Cell &cellToMove = cells[bestCell];
        cellToMove.locked = true;             // 將這個cell鎖住，表示這個cell已經被移動過了
        cellToMove.group = !cellToMove.group; // 將這個cell的group改為另一組
        moveOrder.push_back(bestCell);        // 將這個cell的名稱放入moveOrder中
        // 計算目前cutsize
        int cut = cut_size();
        gainHistory.push_back(cut);
        cout << "Current cut size: " << cut << endl;

        // 更新與這個 cell 連接的 net 上的其他 cell 的 gain
        update_gains(bestCell);
    }
    // 找最小 cut size 的步驟，恢復到該狀態
    int min_cut = gainHistory[0], min_index = 0;
    for (size_t i = 1; i < gainHistory.size(); ++i)
    {
        if (gainHistory[i] < min_cut)
        {
            min_cut = gainHistory[i];
            min_index = i;
        }
    }

    cout << "\nOptimal cut size at step " << min_index + 1 << ": " << min_cut << endl;

    // Rollback 剩下的移動
    for (size_t i = min_index + 1; i < moveOrder.size(); ++i)
    {
        const string &cellName = moveOrder[i];
        cells[cellName].group = 1 - cells[cellName].group; // revert
    }
    cout << "Rollback to optimal cut size: " << min_cut << endl;
}

void output_result(const string &filename)
{
    ofstream out(filename);
    out << "cut_size " << cut_size() << "\n";
    out << "A\n";
    for (unordered_map<string, Cell>::iterator it = cells.begin(); it != cells.end(); ++it)
        if (it->second.group == 0)
            out << it->first << "\n";
    out << "B\n";
    for (unordered_map<string, Cell>::iterator it = cells.begin(); it != cells.end(); ++it)
        if (it->second.group == 1)
            out << it->first << "\n";
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: ./pa2 input_file output_file\n";
        return 1;
    }

    auto start = chrono::high_resolution_clock::now(); // 計時開始

    parse_input(argv[1]);
    initial_partition();
    calculate_gains();
    Fiduccia_Mattheyses_Algorithm();
    output_result(argv[2]);

    auto end = chrono::high_resolution_clock::now(); // 計時結束

    chrono::duration<double> runtime = end - start; // 計算經過的時間

    int cut = cut_size();
    cout << "Cut size: " << cut << endl;
    cout << "Runtime: " << runtime.count() << " s\n" << endl; // 輸出經過的時間

    return 0;
}
