#include <TH1D.h>
#include <TGraph.h>
#include <TF1.h>
#include <cmath>
#include <TCanvas.h>

double f(double x, double mu){
    double r = 0.5 * sqrt(511 / (2 - cos(mu))) / (2 * sqrt(2 * log(2)));
    
    return pow(2 * M_PI * pow(r, 2), -0.5) * exp(-0.5 * pow((x - mu) / r, 2));
}

void test8(){
    int points = 1;
    int total_bin = 1000;
    double xValue[points], yValue[points], nd[points], areaValue[points];
    double xhist[total_bin], sumhist[total_bin];
    double width = 0.5;
    double sum = 0.0;
    TCanvas* c1 = new TCanvas("c1", "c1");
    // TH1D* h1 = new TH1D("h1", "h1", 10, -10, 10);
    // h1->GetYaxis()->SetRangeUser(0, 0.16);
    // h1->Draw();

    double min = -5.0;
    double max = 10.0;
    
    for(int i = 0; i < points; i++){
        xValue[i] = 1 + i * width;
        yValue[i] = xValue[i];
        areaValue[i] = yValue[i] * width;
        double area = areaValue[i];
        double mu = xValue[i];
        // TF1* func0 = new TF1("f0", "0", -10, 10);
        // func0->Draw();
        // if(i == 0){
        // TF1* func = new TF1("f1", "[0] * f(x, [1])", -10, 10);
        // func->SetParameters(area, mu);
        // func->Draw("same");
        // }
        // else{
        //     TF1* func = new TF1("f1", "[0] * f(x, [1])", -10, 10);
        //     func->SetParameters(area, mu);
        //     func->Draw("same");
        // }
        TH1D* hist = new TH1D("hist", "hist", total_bin, min, max);
        for(int j = 0; j < total_bin; j++){
                xhist[j] = min + (max - min) / total_bin * j;
                hist->Fill(xhist[j], area * f(xhist[j], mu));
                // hist->Fill(xhist[j], 5);
                sumhist[j] = (max - min) / total_bin * area * f(xhist[j], mu);
                sum += sumhist[j];
            }
            
            
            
        
        hist->Draw();
        std::cout << area << std::endl;
        std::cout << sum << std::endl;
    }
}

    // TGraph* graph = new TGraph(points, x, y);
    // graph->Draw();

    // double r = 0.5;
    // double a = 1;

    
