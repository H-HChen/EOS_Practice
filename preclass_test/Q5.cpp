#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    int n = 60;
    vector < int > Aseries;
    vector < int > cand = {2, 3, 5};

    int upper = 0;
    int lower = 0;

    vector < int > countPrimes_linear()
    {
        for (int i: cand) {
            recursive_multi(i);
        }
        return Aseries;
    }

    void recursive_multi(int num)
    {
        if (num > n) return;

        // For debug
        // cout << num << endl;

        Aseries.push_back(num);

        if (upper == 0) {
            upper = 1;
            lower = num;
        } else {
            upper = upper * num + lower;
            lower = num * lower;

            int common = reduction(upper, lower);
            upper /= common;
            lower /= common;
        }

        for (int j = 0; j < cand.size(); ++j) {
            recursive_multi(num * cand[j]);
            if (num % cand[j] == 0) {
                break;
            }
        }
    }

    int reduction(int a, int b)
    {
        if (b == 0)
            return a;
        return reduction( b, a % b );
    }
};

int main()
{
    Solution sol;
    int count = 0;

    auto ans = sol.countPrimes_linear();

    for (int i: ans) {
        count += 1;
        cout << i << endl;
    }

    cout << sol.upper << '/' << sol.lower << endl;
    cout << double(sol.upper) / double(sol.lower) << endl;
    cout << "total count: " << count << endl;
}
