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
void energy_calb(TString input_Folder = "./output/"){
    // ifstream ifs("../data/sato_NaI.txt");
    // ifstream ifs("../data/huruno_PMT_2.txt");
    TString input_Filepath = Form("%sdata.txt",input_Folder.Data());
    ifstream ifs(input_Filepath);
    double energy, ch, sigma_ch;

    TCanvas* canvas = new TCanvas();
    TGraphErrors* graph = new TGraphErrors;
    int index_data = 0;
    while(ifs >> energy >> ch >> sigma_ch){
        graph->SetPoint(index_data, ch, energy);
        graph->SetPointError(index_data, sigma_ch, 0);
        cout << index_data << endl;
        index_data++;
    }
    ifs.close();
    graph->SetTitle(";voltage_sum [V];Photoelectric peak energy [keV]");
    graph->SetMarkerStyle(20);
    gStyle->SetOptFit();
    graph->Fit("pol1");
    graph->Draw("ap"); //axisとpointを描画する

    canvas->SaveAs("./figure/energy_calb.pdf");
}