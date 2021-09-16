#pragma once

#include <modules/tnm067lab1/tnm067lab1moduledefine.h>
#include <inviwo/core/util/glm.h>


namespace inviwo {

template <typename T>
struct float_type {
    using type = double;
};

template <>
struct float_type<float> {
    using type = float;
};
template <>
struct float_type<vec3> {
    using type = float;
};
template <>
struct float_type<vec2> {
    using type = float;
};
template <>
struct float_type<vec4> {
    using type = float;
};

namespace TNM067 {
namespace Interpolation {

#define ENABLE_LINEAR_UNITTEST 0
template <typename T, typename F = double>
T linear(const T& a, const T& b, F x) {
    if (x <= 0) return a;
    if (x >= 1) return b;
    
    T f = a * (1 - x) + b * x;
    
    // y0 = b
    // y1 = a
    
    // b * (x - x0 ) + a * (x1 - x) / (x1 - x0)
    // x0 = 1
    // x1 = 0
    // ger oss
    // a * (x - 1 ) + b * (0 - x) / -1
    // a * (1 - x) + b * (x)
    
    
    // quadraTIC
    // x0 = 2
    // x1 = 1
    // x2 = 0
    // c b a
    // c * ((x - 1) * (x - 0))/((1)*(2-0)) + b * ((x-2)*(x))/((-1)*(1)) + a * ((x-2)*(x-1))/((-2)*(-1))
    
    return f;
}

// clang-format off
/*
 2------3
 |      |
 y|  •   |
 |      |
 0------1
 x
 */
// clang format on
#define ENABLE_BILINEAR_UNITTEST 0
template<typename T, typename F = double> 
T bilinear(const std::array<T, 4> &v, F x, F y) {
    T c1 = linear(v[0], v[1], x);
    T c2 = linear(v[2], v[3], x);
    T c3 = linear(c1, c2, y);
    return c3;
}


// clang-format off
/*
 a--•----b------c
 0  x    1      2
 */
// clang-format on
#define ENABLE_QUADRATIC_UNITTEST 0
template <typename T, typename F = double>
T quadratic(const T& a, const T& b, const T& c, F x) {
    
    //    T color =   a * ((x - 1) * (x - 2)) / ((-1) * (-2)) +
    //                b * ((x) * (x-2)) / ((1) * (-1)) +
    //                c * ((x) * (x-1)) / ((2) * (1));
    T color = (1 - x) * (1 - 2 * x) * a + 4 * x * (1 - x) * b + x * (2 * x - 1) * c;
    
    return color;
}

// clang-format off
/*
  6-------7-------8
  |       |       |
  |       |       |
  |       |       |
  3-------4-------5
  |       |       |
 y|  •    |       |
  |       |       |
  0-------1-------2
  0  x    1       2
 */
// clang-format on
#define ENABLE_BIQUADRATIC_UNITTEST 0
template <typename T, typename F = double>
T biQuadratic(const std::array<T, 9>& v, F x, F y) {
    T c1 = quadratic(v[0], v[1], v[2], x);
    T c2 = quadratic(v[3], v[4], v[5], x);
    T c3 = quadratic(v[6], v[7], v[8], x);
    T c4 = quadratic(c1, c2, c3, y);
    return c4;
}

// clang-format off
/*
   2---------3
   |'-.      |
   |   -,    |
 y |  •  -,  |
   |       -,|
   0---------1
        x
 */
// clang-format on
#define ENABLE_BARYCENTRIC_UNITTEST 1
template <typename T, typename F = double>
T barycentric(const std::array<T, 4>& v, F x, F y) {
    
    // v holds the function values of alpha, beta and gamma, f(A), f(B), f(G), f(A2)
    
    // alpha and beta are effecivly a division of 2 triangles formed by placing a point p inside the bigger triangle.
    // gamma is calculated from alpha and beta according to the formula: 1.0f - alpha - beta
    
    F alpha, beta, gamma, fA;
    F fB = v[1];
    F fG = v[2];
    
    // acording to the formula a + b + g = 1
    if (x + y < 1.0f) {
        alpha = 1.0 - x - y;
        beta = x;
        gamma = y;
        
        fA = v[0];
    } else {
        // is the values of x + y excedes 1, then a + b + g is no longer 1. Therefore:
        alpha = x + y - 1 ;
        beta = 1 - y;
        gamma = 1 - x;
        
        fA = v[3];
    }
    
    return alpha * fA + beta * fB + gamma * fG;
}

}  // namespace Interpolation
}  // namespace TNM067
}  // namespace inviwo
