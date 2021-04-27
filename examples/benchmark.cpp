
#include <chrono>
#include <iostream>
#include <vector>

#include "hidecall.h"

#define rounds 10

struct Measurement
{
    std::chrono::duration<double> bias;
    std::chrono::duration<double> normal;
    std::chrono::duration<double> hidecall;

    Measurement& operator/(size_t count)
    {
        this->bias /= count;
        this->normal /= count;
        this->hidecall /= count;

        return *this;
    }

    void operator+=(Measurement& other)
    {
        this->bias += other.bias;
        this->normal += other.normal;
        this->hidecall += other.hidecall;
    }
};

Measurement getAverage(std::vector<Measurement>& measurements)
{
    Measurement mRet = {};
    size_t count = 0;

    for (auto m : measurements)
        mRet += m;

    return mRet / measurements.size();
}

HIDECALL(__declspec(noinline), void, BM_HC, ())
{
    __asm nop;
}

__declspec(noinline) void BM_NORMAL()
{
    __asm nop;
}

int main()
{
    std::vector<Measurement> times;
    times.reserve(rounds);

    std::cout << "WARNING!\nBenchmark can take a long time, depending on the number of rounds. Please be patient!\n\n";

    for (size_t r = 0; r < rounds; r++)
    {
        Measurement m;
        auto bias_start = std::chrono::steady_clock::now();

        for (size_t i = 0; i < INT_MAX; i++)
        {
            __asm nop;
        }

        auto bias_end = std::chrono::steady_clock::now();
        m.bias = bias_end - bias_start;

        auto start = std::chrono::steady_clock::now();

        for (size_t i = 0; i < INT_MAX; i++)
            BM_NORMAL();

        auto end = std::chrono::steady_clock::now();
        m.normal = end - start - m.bias;


        auto hc_start = std::chrono::steady_clock::now();

        for (size_t i = 0; i < INT_MAX; i++)
            BM_HC();

        auto hc_end = std::chrono::steady_clock::now();
        m.hidecall = hc_end - hc_start - m.bias;

        times.push_back(m);
    }

    Measurement avg = getAverage(times);

    std::cout << "Bias time: " << avg.bias.count() << "s\n";
    std::cout << "Normal time: " << avg.normal.count() << "s\n";
    std::cout << "Hidecall time: " << avg.hidecall.count() << "s\n";
    std::cout << "Hidecall is " << avg.hidecall / avg.normal << " times slower.\n";

    std::cin.get();
}
