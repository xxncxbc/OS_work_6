#include <mutex>
#include <thread>
#include<future>
#include <iostream>
using namespace std;

void work(vector<__int16>& arr, int& k, mutex& mtx, condition_variable& cv, bool& k_ready, bool& work_ready) {

    unique_lock<mutex> lock(mtx); //тут зону видимости делать не надо ибо блокируем полностью

    cv.wait(lock, [&k_ready] {return k_ready; }); //ждем пока k будет объявлено

    int j = 0;
    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] < 0 && arr[i] > k) {
            swap(arr[i], arr[j]);
            j++;
        }
    }
    
    work_ready = true;
    cout << endl << "work is done" << endl;
    cv.notify_all();
    
}

void counter(vector<__int16>& arr, int& k, mutex& mtx, condition_variable& cv, bool& work_ready, promise<int>&& prom) {
    int res = 0;

    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [&work_ready] {return work_ready; }); //ждем пока work закончит

    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] < 0) ++res;
    }
    cout << endl << "counter is done" << endl;
    prom.set_value(res);
  

}

int main() {

    mutex mtx;
    condition_variable cv;
    bool k_ready = false, work_ready = false;
    int n, k;
    promise<int> prom;
    future<int> fut = prom.get_future();

    cout << "Enter array size: " << endl;
    cin >> n;
    vector<__int16> arr(n);
    for (int i = 0; i < n; ++i) { cin >> arr[i]; }
    cout << "Your array: " << endl << "size: " << n << endl;
    for (int i = 0; i < n; ++i) cout << arr[i] << ", ";
    cout << endl;

    thread wThread(work, ref(arr), ref(k), ref(mtx), ref(cv), ref(k_ready), ref(work_ready));
    thread cThread(counter, ref(arr), ref(k), ref(mtx), ref(cv), ref(work_ready), move(prom));

    { //на всякий случай обезопашиваем данные, после выхода за зону видимости мьютекс разлочится
        unique_lock<mutex> lock(mtx);
        cout << "Enter k: " << endl;
        cin >> k;
        k_ready = true;
        cv.notify_all();
    }

    { //тут просто оповестить надо заданию, поэтому в зону видимости засунул, чтоб не блокировать main
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&work_ready] {return work_ready; }); //ждем пока work закончит
    }

    cout << "New array: " << endl;
    for (int i = 0; i < n; ++i) cout << arr[i] << ",";
    cout << endl;

    int num_neg = fut.get(); //тут поток main виснет, пока не отработает set_value
    cout << "Number of negative numbers: " << num_neg;

    wThread.join();
    cThread.join();
}


