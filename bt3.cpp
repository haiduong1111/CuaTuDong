#include <iostream>
#include <queue>
#include <vector>

using namespace std;

int main() {
    int n, b;
    cin >> n >> b;

    vector<pair<int, int>> queries(n);
    for (int i = 0; i < n; i++) {
        cin >> queries[i].first >> queries[i].second;
    }

    queue<pair<int, int>> q; // Hàng d?i các yêu c?u
    vector<int> end_times(n, -1); // Th?i di?m hoàn thành c?a các yêu c?u
    long long current_time = 0; // Th?i di?m hi?n t?i c?a máy ch?

    for (int i = 0; i < n; i++) {
        int ti = queries[i].first;
        int di = queries[i].second;

        // X? lý các yêu c?u dã hoàn thành trong hàng d?i tru?c khi x? lý yêu c?u m?i
        while (!q.empty() && current_time <= ti) {
            int query_index = q.front().first;
            int duration = q.front().second;
            q.pop();

            current_time = max(current_time, (long long)queries[query_index].first) + duration;
            end_times[query_index] = current_time;
        }

        if (current_time <= ti) {
            // Máy ch? r?nh t?i th?i di?m ti, b?t d?u x? lý yêu c?u này
            current_time = ti + di;
            end_times[i] = current_time;
        } else if (q.size() < (unsigned int)b) {
            // Máy ch? dang b?n, thêm yêu c?u này vào hàng d?i n?u còn ch?
            q.push({i, di});
        } else {
            // Máy ch? dang b?n và hàng d?i d?y, t? ch?i yêu c?u này
            end_times[i] = -1;
        }
    }

    // X? lý các yêu c?u còn l?i trong hàng d?i
    while (!q.empty()) {
        int query_index = q.front().first;
        int duration = q.front().second;
        q.pop();

        current_time = max(current_time, (long long)queries[query_index].first) + duration;
        end_times[query_index] = current_time;
    }

    for (int i = 0; i < n; i++) {
        cout << end_times[i] << " ";
    }
    cout << endl;

    return 0;
}

