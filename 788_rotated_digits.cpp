/*
788. Rotated Digits
An integer x is a good if after rotating each digit individually by 180 degrees, we get a valid number that is different from x. Each digit must be rotated - we cannot choose to leave it alone.

A number is valid if each digit remains a digit after rotation. For example:

0, 1, and 8 rotate to themselves,
2 and 5 rotate to each other (in this case they are rotated in a different direction, in other words, 2 or 5 gets mirrored),
6 and 9 rotate to each other, and
the rest of the numbers do not rotate to any other number and become invalid.
Given an integer n, return the number of good integers in the range [1, n].

Example 1:
Input: n = 10
Output: 4
Explanation: There are four good numbers in the range [1, 10] : 2, 5, 6, 9.
Note that 1 and 10 are not good numbers, since they remain unchanged after rotating.

Example 2:
Input: n = 1
Output: 0

Example 3:
Input: n = 2
Output: 1
*/

class Solution {
public:
    int rotatedDigits(int n) {
        unordered_set<int> normDigits = {0, 1, 8};
        unordered_set<int> swapDigits = {2, 5, 6, 9};

        vector<int> state(n + 1, 0);
        int count = 0;

        for (int i = 0; i <= n; ++i) {
            if (i < 10) {
                if (normDigits.count(i)) {
                    state[i] = 1;
                }

                else if (swapDigits.count(i)) {
                    state[i] = 2;
                    ++count;
                }
            }
            
            else {
                int a = state[i / 10];
                int b = state[i % 10];

                if (a == 0 || b == 0) {
                    state[i] = 0;
                }
                
                else if (a == 2 || b == 2) {
                    state[i] = 2;
                    ++count;
                }
                
                else {
                    state[i] = 1;
                }
            }
        }

        return count;
    }
};
