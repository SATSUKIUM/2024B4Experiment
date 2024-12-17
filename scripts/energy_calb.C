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

#include <iomanip>
#include <chrono>
#include <ctime> //時刻情報

#include <fstream>
#include <filesystem>
#include <TSystem.h>

using namespace std;
void energy_calb(TString input_Folder = "./output/"){
    // ifstream ifs("../data/sato_NaI.txt");
    // ifstream ifs("../data/huruno_PMT_2.txt");
    TString input_Filepath = Form("%sdata.txt",input_Folder.Data());
    ifstream ifs(input_Filepath);
    double_t energy, ch, sigma_ch, sigma_gaus;

    TCanvas* canvas = new TCanvas("canvas", Form("%s", input_Filepath.Data()));
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
    gPad->SetGrid();
    graph->SetTitle(";voltage_sum [V];Photoelectric peak energy [keV]");
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.5);
    gStyle->SetOptFit();
    graph->Fit(func);
    graph->Draw("ap"); //axisとpointを描画する

    TString filename_figure = "energy_calb.pdf";

    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName("./figure/" + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("energy_calb_%d.pdf", index);
        index++;
    }

    
    canvas->SaveAs("./figure/" + filename_figure);
}