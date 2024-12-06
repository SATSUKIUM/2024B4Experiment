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
#include <TF1.h>
using namespace std;
void energy_calb(TString input_Folder = "./output/"){
    // ifstream ifs("../data/sato_NaI.txt");
    // ifstream ifs("../data/huruno_PMT_2.txt");
    TString input_Filepath = Form("%sdata.txt",input_Folder.Data());
    ifstream ifs(input_Filepath);
    double_t energy, ch, sigma_ch, sigma_gaus;

    TCanvas* canvas = new TCanvas();
    TGraphErrors* graph = new TGraphErrors;
    TF1* func = new TF1("func", "[0]*x +[1]", 0, 30);
    func->SetParameters(10, 10);
    int index_data = 0;
    while(ifs >> energy >> ch >> sigma_ch >> sigma_gaus){
        graph->SetPoint(index_data, ch, energy);
        graph->SetPointError(index_data, sigma_ch, 0);
        cout << index_data << endl;
        index_data++;
    }
    ifs.close();
    graph->SetTitle(";voltage_sum [V];Photoelectric peak energy [keV]");
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.5);
    gStyle->SetOptFit();
    graph->Fit(func);
    graph->Draw("ap"); //axisとpointを描画する

    canvas->SaveAs("./figure/energy_calb.pdf");
}