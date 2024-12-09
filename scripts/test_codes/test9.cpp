#include <cmath>

double fitting_func(double x, double a, double b){
    return a - b * cos(2 * x);
}

double Integral_func(double x, double x_min, double x_max, double a, double b){
    int l = 100;
    double delta = (x_max - x_min) / l;
    double x_val[l];
    double value = 0.0;
    for(int i = 0; i < l; i++){
        x_val[i] = x_min + i * delta;
        value += fitting_func(x_val[i], a, b) * delta;
    }
    return value;
}