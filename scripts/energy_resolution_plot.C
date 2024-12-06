/*
エネルギーとエネルギー分解能の関係のプロット、および、1/√Eでのフィッティングをするマクロ
*/
#include <iostream>
#include <fstream>
#include <TGraph.h>
#include <TGraphErrors.h> 
#include <TStyle.h> 
#include <TString.h>
#include <TCanvas.h>
#include <TF1.h>
using namespace std;
void energy_resolution_plot(TString input_Folder = "./analysis/"){
    TString input_Filepath = Form("%senergy_resolution_test.txt",input_Folder.Data());
    ifstream ifs(input_Filepath);
    double energy, ch, sigma_ch, sigma_gaus, sigma_gaus_energy;

    TCanvas* canvas = new TCanvas();
    TGraph* graph = new TGraph;
    int index_data = 0;
    Double_t R; //energy resolution
    while(ifs >> energy >> ch >> sigma_ch >> sigma_gaus){
        sigma_gaus_energy = sigma_gaus*51.4; //energy error in keV
        R = sigma_gaus_energy*2*sqrt(2*log(2))/(51.4*ch-1.297);
        graph->SetPoint(index_data, energy, R*100);
        cout << index_data << endl;
        index_data++;
    }
    ifs.close();
    graph->SetTitle(";energy [keV];energy resolution (%)");
    graph->SetMarkerStyle(20);
    
    graph->Draw("ap");
    // graph->Fit("pol1");
    TF1 *fitFunc = new TF1("fitFunc", "[0]/sqrt(x)", 10, 600);
    fitFunc->SetParameter(0,30);
    graph->Fit(fitFunc);
    std::cout << "Fitting parameter [0]/sqrt(x) : " << fitFunc->GetParameter(0) << std::endl;

    fitFunc->Draw("same");
    // graph->Draw("ap"); //axisとpointを描画する
    gStyle->SetOptFit();
    canvas->Update();

    canvas->SaveAs("./figure/energy_res.pdf");
}