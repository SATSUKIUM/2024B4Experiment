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
    double energy, ch, sigma_ch;

    TCanvas* canvas = new TCanvas();
    TGraph* graph = new TGraph;
    int index_data = 0;
    Double_t R; //energy resolution
    while(ifs >> energy >> ch >> sigma_ch){
        R = sigma_ch*2*sqrt(2*log(2))/ch;
        graph->SetPoint(index_data, ch, R);
        index_data++;
    }
    ifs.close();
    graph->SetTitle(";peak ch(voltage sum) [V];energy resolution (%)");
    graph->SetMarkerStyle(20);
    
    graph->Draw("ap");
    // graph->Fit("pol1");
    TF1 *fitFunc = new TF1("fitFunc", "[0]/sqrt(x)", 2, 20);
    fitFunc->SetParameter(0,1);
    graph->Fit(fitFunc);
    std::cout << "Fitting parameter [0]/sqrt(x) : " << fitFunc->GetParameter(0) << std::endl;

    fitFunc->Draw("same");
    // graph->Draw("ap"); //axisとpointを描画する
    gStyle->SetOptFit();
    canvas->Update();

    canvas->SaveAs("./figure/energy_res.pdf");
}