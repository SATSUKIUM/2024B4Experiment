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
#include <TSystem.h>

void Load_EnergycalbData(TString key, Double_t p0[2][4], Double_t p0e[2][4], Double_t p1[2][4], Double_t p1e[2][4]){
    Double_t p0_buf, p1_buf, p0e_buf, p1e_buf;
    TString calb_data_filepath = Form("./cfg/%s/data.txt", key.Data());
    std::ifstream ifs(calb_data_filepath);
    Int_t line_index = 0;
    while(ifs >> p0_buf >> p0e_buf >> p1_buf >> p1e_buf){
        if(line_index % 4 == line_index){
            p0[0][line_index] = p0_buf;
            p0e[0][line_index] = p0e_buf;
            p1[0][line_index] = p1_buf;
            p1e[0][line_index] = p1e_buf;
            std::cout << Form("\tiBoard : 0, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            std::cout << Form("\t\t%lf %lf %lf %lf", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        else if((line_index-4) % 4 == line_index){
            p0[1][line_index] = p0_buf;
            p0e[1][line_index] = p0e_buf;
            p1[1][line_index] = p1_buf;
            p1e[1][line_index] = p1e_buf;
            std::cout << Form("\tiBoard : 1, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            std::cout << Form("\t\t%lf %lf %lf %lf", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        line_index++;
        if(line_index == 8){
            break;
        }
    }
    ifs.close();
}



using namespace std;
void energy_resolution(TString input_Folder = "./output/"){
    TString input_Filepath = Form("%ss4_calib.txt",input_Folder.Data());
    std::ifstream ifs(input_Filepath);
    double energy, ch, sigma_ch, sigma_gaus, sigma_gaus_energy;

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e);

    TCanvas* canvas = new TCanvas();
    TGraph* graph = new TGraph;
    int index_data = 0;
    Double_t R; //energy resolution
    while(ifs >> energy >> ch >> sigma_ch >> sigma_gaus){
        sigma_gaus_energy = sigma_gaus*p1[iBoard][iCh]; //energy error in keV
        R = sigma_gaus_energy*2*sqrt(2*log(2))/(p1[iBoard][iCh]*ch);
        graph->SetPoint(index_data, energy, R*100);
        std::cout << index_data << std::endl;
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