/*
θ分解能をプロットするマクロ
*/
#include <iostream>
#include <fstream>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TStyle.h> 
#include <TString.h>
#include <TCanvas.h>
#include <TF1.h>
#include <vector>
#include <math.h>

void theta_resolution_Plot(TString input_Folder = "./analysis/"){
    

    Double_t energy_plot_index = 0;
    Double_t energy_lower_bound = 200;
    Double_t energy_upper_bound = 400;
    Int_t div_num = 1000;
    Double_t delta = (energy_upper_bound-energy_lower_bound)/div_num;
    std::vector<Double_t> energy_vec, theta_vec, sigma_theta_vec;
    for(Int_t i=0; i<div_num; i++){
        energy_plot_index = energy_lower_bound + i*delta;
        // std::cout << energy_plot_index << std::endl;
        energy_vec.push_back(energy_plot_index);
        theta_vec.push_back(acos(2-511/energy_plot_index));
        sigma_theta_vec.push_back(abs((-1/(1-pow((2-511/energy_plot_index),2)))*(-511/pow(energy_plot_index,2))*((5.076*sqrt(energy_plot_index))/(2*sqrt(2*log(2))))));
        // std::cout << i << std::endl;
    }

    TCanvas* canvas = new TCanvas();
    TGraph* graph = new TGraph;
    int index_data = 0;

    auto it1 = theta_vec.begin();
    auto it2 = sigma_theta_vec.begin();

    while(it1 != theta_vec.end() && it2 != sigma_theta_vec.end()){
        double theta = *it1;
        double sigma_theta = *it2;
        ++it1;
        ++it2;
        std::cout << theta << " || " << sigma_theta << std::endl;
        graph->SetPoint(index_data, theta*(360/(2*M_PI)), sigma_theta*(360/(2*M_PI)));
        index_data++;
    }

    graph->SetTitle(";theta [deg];sigma theta [deg]");
    graph->SetMarkerStyle(20);
    
    graph->Draw("ap");
    // graph->Fit("pol1");
    // TF1 *fitFunc = new TF1("fitFunc", "[0]/sqrt(x)", 10, 600);
    // fitFunc->SetParameter(0,30);
    // graph->Fit(fitFunc);
    // std::cout << "Fitting parameter [0]/sqrt(x) : " << fitFunc->GetParameter(0) << std::endl;

    // fitFunc->Draw("same");
    // graph->Draw("ap"); //axisとpointを描画する
    // gStyle->SetOptFit();
    canvas->Update();

    canvas->SaveAs("./figure/theta_res_res.pdf");

}