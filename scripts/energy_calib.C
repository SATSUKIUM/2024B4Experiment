/*
PMTのエネルギー較正用の直線フィッティング
*/
#include <iostream>
#include <fstream>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TAxis.h>
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

TString Makedir_Date(){
    //YYYYMMDDのフォルダを作る関数。呼び出せば勝手にYYYYMMDDのフォルダができる。
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm);
    TString folderPath = TString::Format("./figure/%s", date);

    if(gSystem->AccessPathName(folderPath)){
        if(gSystem->mkdir(folderPath, true) != 0){
                std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
                return -1;
        }
    }
    return (folderPath);
}

void energy_calib(TString input_Folder = "./output/"){
    TString input_Filepath = Form("%sdata.txt",input_Folder.Data());
    std::ifstream ifs(input_Filepath);

    TCanvas* canvas = new TCanvas("canvas", Form("%s", input_Filepath.Data()));
    TGraphErrors* graph = new TGraphErrors;
    graph->SetTitle(Form("energy calibration form %s;Voltage sum [V];Photoelectric peak energy [keV]", input_Filepath.Data()));
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.5);
    gPad->SetGrid();
    gStyle->SetOptFit();

    double_t energy, ch, sigma_ch, sigma_gaus;
    Int_t index_data = 0;
    Double_t max_ch = 0;
    Double_t max_energy = 0;
    while(ifs >> energy >> ch >> sigma_ch >> sigma_gaus){
        graph->SetPoint(index_data, ch, energy);
        graph->SetPointError(index_data, sigma_ch, 0);
        std::cout << "Plot point : " << index_data << std::endl;
        index_data++;
        if(ch > max_ch){
            max_ch = ch;
        }
        if(energy > max_energy){
            max_energy = energy;
        }
    }
    ifs.close();
    graph->GetXaxis()->SetLimits(0.0, max_ch*1.1);
    graph->GetYaxis()->SetRangeUser(0.0, max_energy*1.1);

    TF1* func = new TF1("func", "[0]+[1]*x", 0, 0.5);
    func->SetParameters(0, 5);
    graph->Fit(func);
    graph->Draw("ap"); //axisとpointを描画する
    // フィット情報（統計ボックス）の位置を左上に移動
    gStyle->SetStatX(0.5);  // X座標（左寄せ）
    gStyle->SetStatY(0.9);  // Y座標（上寄せ）
    std::cout << Form("================================================================\nFitting parameter for %s\n\t%f %f %f %f\n\tPlease copy and paste to scripts/cfg/key/data.txt\n", input_Filepath.Data(), func->GetParameter(0), func->GetParError(0), func->GetParameter(1), func->GetParError(1)) << std::endl;
    
    TString filename_figure = "energy_calib.pdf";
    TString YYYYMMDD_folder = Makedir_Date();
    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName(YYYYMMDD_folder + "/" + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("energy_calib_%d.pdf", index);
        index++;
    }

    canvas->SaveAs(YYYYMMDD_folder + "/"+ filename_figure);
}