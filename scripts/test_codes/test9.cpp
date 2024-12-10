#include <cmath>
#include <TGraph.h>
#include <TF1.h>

double fitting_func(double x, double a, double b){
    double width = 2.0;
    double range_scat_to_absorber = 25.0;
    double width_radian = 2 * atan(width / (2 * range_scat_to_absorber));

    return a * width_radian - b * cos(2 * x) * sin(width_radian);
}

double data_points(double x){
    return 1 - cos(2 * x);
}

void Integral_func(){
    // int l = 10;
    // int points = 5;
    // double width = 2 * atan(1 / 25);
    // double delta = width / l;
    // double x_val[l];
    // double values[points];
    // double value = 0.0;
    // for(int i = 0; i < points; i++){
    //     x_mean[i] = i * M_PI / 4;
    //     for(int j = 0; j < l; j++){
    //         value = 0.0;
    //         x_val[j] = (x_mean[i] - (width / 2)) + j * width / l;
    //         value = fitting_func(x_val[j], a, b) * delta;
    //         values[i] += value;
    //     }
    // }
    // TGraph* graph = new TGraph(points, x_mean, values);
    // graph->Draw();

    int points = 5;
    double x_value[points], y_value[points];
    TGraph* graph = new TGraph();
    for(int i = 0; i < points; i++){
        x_value[i] = i * M_PI / 4;
        y_value[i] = data_points(x_value[i]);
        graph->SetPoint(i, x_value[i], y_value[i]);
    }

    TF1* func = new TF1("func", "fitting_func(x, [0], [1])", 0, M_PI);
    func->SetParameters(10, 10);
    graph->Fit(func);
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.5);
    graph->Draw("ap");

}