#pragma once
#include <iostream>
#include <cmath>
#include <random>
#include <chrono> 
#undef M_PI
#define M_PI 3.141592653589793f

extern const float  EPSILON;
const float kInfinity = std::numeric_limits<float>::max();

inline float clamp(const float &lo, const float &hi, const float &v)
{ return std::max(lo, std::min(hi, v)); }

inline  bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = - 0.5 * b / a;
    else {
        float q = (b > 0) ?
                  -0.5 * (b + sqrt(discr)) :
                  -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);
    return true;
}
/*
inline float get_random_float()
{
    
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]
    
    return dist(rng);
    
    static thread_local std::mt19937 generator(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    return distribution(generator);
}
*/
inline float get_random_float()
{
    // static thread_local 保证每个线程只初始化一次 generator
    static thread_local std::mt19937 generator([]() {
        // 1. 获取高精度时间（纳秒级）
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        // 2. 尝试获取 random_device (虽然在MinGW下可能固定，但混合进去也没坏处)
        std::random_device rd;

        // 3. 将两者异或作为种子，确保种子既包含随机性又包含时间性
        // 这样即使 rd() 是固定的，时间戳的差异也能区分不同线程
        return std::mt19937(now ^ rd());
        }());

    // 分布对象可以是静态的，因为它没有内部状态（或者状态很轻微），
    // 关键是 generator 的状态必须独立。
    static thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    return distribution(generator);
}
inline void UpdateProgress(float progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
};
