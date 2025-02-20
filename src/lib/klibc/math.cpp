//
// Created by Neko on 02.02.2025.
//

#include "math.h"
#include <cmath>

#define PI 3.14159265358979323846

extern "C"
{

    unsigned long long factorial(int n)
    {
        unsigned long long fact = 1;
        for (int i = 1; i <= n; i++)
        {
            fact *= i;
        }
        return fact;
    }

    double sin(double x)
    {
        // Reduce x to the range -pi to pi to improve convergence
        x = fmod(x, 2 * PI);
        if (x < -PI)
        {
            x += 2 * PI;
        }
        else if (x > PI)
        {
            x -= 2 * PI;
        }

        double result = 0.0;
        double term;
        int n = 1;
        int sign = 1;

        // Approximate sin(x) using the first few terms of the Taylor series
        for (int i = 1; i <= 15; i += 2)
        {
            term = sign * (pow(x, i) / factorial(i));
            result += term;
            sign *= -1; // Alternate signs for each term
        }

        return result;
    }

#define MAX_ITERATIONS 100

    double fmod(double x, double y)
    {
        if (y == 0)
        {
            return 0; // Handling division by zero
        }
        // Use modulo operation to bring x into the range [0, y)
        double result = x - y * (int)(x / y);
        return result;
    }

    double log(double x)
    {
        if (x <= -1)
        {
            return -1;
        }

        double result = 0.0;
        double term = x; // The first term of the series

        for (int n = 1; n <= MAX_ITERATIONS; n++)
        {
            result += term;
            term *= -x * (n) / (n + 1); // Next term in the series: (-x)^n / n+1
        }

        return result;
    }
}