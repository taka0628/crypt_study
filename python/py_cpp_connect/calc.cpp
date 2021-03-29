#include "calc.h"
using namespace std;

int func(int x, int y)
{
    cout << "hellow from cpp wraping";
    return x + y;
}

bool isPrime(const int64_t value)
{
    if (value <= 1)
    {
        return false;
    }
    double sqrt_vale = sqrt(value);
    bool isPrime = true;
    if (value % 2 == 0)
    {
        return false;
    }
    for (size_t i = 3; i < sqrt_vale; i + 2)
    {
        if (value % i == 0)
        {
            isPrime = false;
            return false;
        }
    }
    if (isPrime)
    {
        return true;
    }
    return false;
}

int main(int argc, char const *argv[])
{
    int64_t value = 0;
    cin >> value;
    if (isPrime(value))
    {
        cout << "素数！" << endl;
    }
    else
    {
        cout << "整数" << endl;
    }
    return 0;
}
