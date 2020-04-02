#include <iostream>
#include <ctime>

int main()
{
    srand(time(nullptr));

    const int n = 300000;
    for (int i = 0; i < n; ++ i)
    {
        std::cout << rand() << "," << rand() << "," << rand() << std::endl;     
    }
    return 0;    
}
