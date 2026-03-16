#pragma once

#include <random>

// Farry Geometric Distribution (Type 2)
//
//   PMF:  P(X = k) = p * q^(k-1),  k = 1, 2, ...
//         where q = 1 - p
//
//   CDF:  F(k) = 1 - q^k
//
//   Inverse transform:  k = ceil( ln(1-U) / ln(q) ),  U ~ Uniform(0,1)

class FarryDistribution {
public:
    explicit FarryDistribution(double p);

    int generate();
    double getP() const { return p_; }

private:
    double p_;
    double logq_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_;
};
