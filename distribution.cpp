#include "distribution.h"

#include <algorithm>
#include <chrono>
#include <cmath>

FarryDistribution::FarryDistribution(double p)
    : p_(std::clamp(p, 0.001, 0.999)),
      logq_(std::log(1.0 - p_)),
      uniform_(0.0, 1.0)
{
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng_.seed(static_cast<unsigned>(seed));
}

int FarryDistribution::generate()
{
    double u = uniform_(rng_);

    // Guard: u must be in (0, 1) open interval to avoid log(0)
    if (u <= 0.0) u = 1e-12;
    if (u >= 1.0) u = 1.0 - 1e-12;

    // Guard: logq must be negative (q < 1)
    if (logq_ >= 0.0) return 1;

    // Inverse CDF: k = ceil( ln(1 - U) / ln(1 - p) )
    double val = std::log(1.0 - u) / logq_;
    if (!std::isfinite(val) || val <= 0.0) return 1;

    int k = static_cast<int>(std::ceil(val));
    return std::max(1, k);
}
