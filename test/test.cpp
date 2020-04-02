#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

std::vector<std::vector<int>> vec;
std::condition_variable condition;
std::mutex mutex;

void func1()
{
    while (true)
    {
        std::vector<int> tmp = { 1, 2, 3, 4 };
        vec.push_back(tmp);
        condition.notify_one();
    }
}

void func2()
{
    while (true)
    {
        std::unique_lock<std::mutex> ulock(mutex);
        condition.wait(ulock);
        
        int index = vec.size() - 1;
        vec[index][0];  
    }
}

int main()
{
    std::thread t1(func1); 
    std::thread t2(func2); 

    t1.join();
    t2.join();
    return 0;    
}
