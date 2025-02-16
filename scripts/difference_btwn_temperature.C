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
void difference_bten_temperature(TString input_Filepath){
    ifstream ifs(input_Filepath);
    double_t temperature, peak_ch, sigma_ch, sigma_gaus;

    TCanvas* canvas = new TCanvas("canvas", Form("%s", input_Filepath.Data()));
    TGraphErrors* graph = new TGraphErrors;
    TF1* func = new TF1("func", "[0]*x +[1]", 0, 30);
    func->SetParameters(10, 10);
    Int_t index_data = 0;
    while(ifs >> temperature >> peak_ch >> sigma_ch >> sigma_gaus){
        if(index_data % 2 == 1){
            graph->SetPoint(index_data, temperature, peak_ch);
            graph->SetPointError(index_data, 0, sigma_ch);
            cout << index_data << endl;
            std::cout << index_data << std::endl;
        }
        index_data++;
    }
    ifs.close();
    gPad->SetGrid();
    graph->SetTitle(Form("DRS4 board temperature vs its display voltage %s;board temperature [deg Celcius];1274 keV peak ch [V]", input_Filepath.Data()));
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