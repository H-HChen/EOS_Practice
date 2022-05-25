#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

class Combination {
private:
    vector < int > pre_result;
    vector < vector < int >> result;

public:
    void traversal(int, int, int, int, int, int);
    vector < vector < int >> combine_sum(int, int, int);
    int count;
};

void Combination::traversal(int current_index, int num, int sum, int max_range, int target_num, int target_sum)
{
    if (sum > target_sum) return;
    if (num > target_num) return;

    count+=1;

    if ((sum == target_sum) && (num == target_num)) {
        result.push_back(pre_result);
        return;
    }

    for (int i = current_index; i <= max_range; i++) {
        pre_result.push_back(i);
        traversal(i, num + 1, sum + i, max_range, target_num, target_sum);
        pre_result.pop_back();
    }
};

vector < vector < int >> Combination::combine_sum(int max_range, int target_num, int target_sum)
{
    traversal(1, 0, 0, max_range, target_num, target_sum);

    return result;
};

int main()
{
    Combination combination;
    auto result = combination.combine_sum(20, 6, 72);
    int count = 0;

    for (vector < int > vec: result) {
        std::cout << "[";
        for (int i: vec) {
            std::cout << i << ' ';
        }
        std::cout << "]" << std::endl;
        count+=1;
    }
    std::cout << count << std::endl;
    std::cout << combination.count << std::endl;
};
