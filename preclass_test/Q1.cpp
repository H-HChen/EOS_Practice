#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

class SquareSum {
public:
    int n = 10000;
    int max_length = 100;

    int sqr(int num)
    {
        return num * num;
    }

    vector < int > bignumber_convertion(int num)
    {
        int last = 0;
        int next = num;

        vector < int > vec(max_length, 0);

        for (int i = 1; i < max_length; i++) {
            last = next % 10;
            next = next / 10;
            vec[max_length - i] = last;
            if (next == 0) break;
        }

        return vec;
    }

    vector < int > sqr_sum()
    {
        vector < int > ans(max_length, 0);
        int carry = 0;

        for (int i = 1; i <= n; i++) {
            vector < int > temp = bignumber_convertion(sqr(i));
            for (int j = max_length - 1; j >= 0; j--) {
                ans[j] = ans[j] + temp[j] + carry;
                carry = ans[j] / 10;
                ans[j] %= 10;
            }
        }
        return ans;
    }
};

int main()
{
    SquareSum squaresum;
    auto ans = squaresum.sqr_sum();
    bool flag = false;

    for (int i: ans) {
        if ((i != 0) && !flag) flag = true;
        if (flag) cout << i;
    }

    cout << endl;
}
