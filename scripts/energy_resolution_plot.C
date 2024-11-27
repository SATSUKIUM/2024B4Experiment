/*
PMTのエネルギー較正用の直線フィッティング
*/
#include <iostream>
#include <fstream>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TStyle.h> //gStyleのところ
#include <TString.h>
#include <TCanvas.h>
using namespace std;
void energy_resolution_plot(TString input_Folder = "./analysis/"){
    // ifstream ifs("../data/sato_NaI.txt");
    // ifstream ifs("../data/huruno_PMT_2.txt");
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
    graph->SetTitle(";voltage_sum [V];Photoelectric peak energy [keV]");
    graph->SetMarkerStyle(20);
    gStyle->SetOptFit();
    graph->Fit("pol1");
    graph->Draw("ap"); //axisとpointを描画する

    canvas->SaveAs("./figure/energy_res.pdf");
}