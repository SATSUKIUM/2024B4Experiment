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
#include <iomanip>
#include <chrono>
#include <ctime> //時刻情報

using namespace std;
void energy_resolution(TString input_Folder = "./output/"){
    TString input_Filepath = Form("%ss4_calib.txt",input_Folder.Data());
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
    //graph->SetTitle(";energy [keV];energy resolution (%)");
    graph->SetTitle(Form("energy calibration form %s;Energy [keV];energy resolution [%%]", input_Filepath.Data()));
   
    graph->SetMarkerStyle(20);
    
    graph->Draw("ap");
    // graph->Fit("pol1");
    TF1 *fitFunc = new TF1("fitFunc", "[0]/sqrt(x)", 0, 1300);
    fitFunc->SetParameter(0,300);
    graph->Fit(fitFunc);
    std::cout << "Fitting parameter [0]/sqrt(x) : " << fitFunc->GetParameter(0) << std::endl;
    graph->GetXaxis()->SetLimits(0,1300);

    fitFunc->Draw("same");
    // graph->Draw("ap"); //axisとpointを描画する
    gStyle->SetOptFit();
    canvas->Update();

    TString filename_figure = "energy_res.pdf";
    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName("./figure/" + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("energy_res_%d.pdf", index);
        index++;
    }
     //canvas->SaveAs("./figure/" + filename_figure);

}