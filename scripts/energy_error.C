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

// Eを較正後になおすためのデータ読み込み
void Load_EnergycalbData(TString key, Double_t p0[2][4], Double_t p0e[2][4], Double_t p1[2][4], Double_t p1e[2][4], Double_t p0_res[2][4], Double_t p0e_res[2][4]){
    Double_t p0_buf, p1_buf, p0e_buf, p1e_buf, p0_res_buf, p0e_res_buf;
    TString calb_data_filepath = Form("./cfg/%s/data.txt", key.Data());
    std::ifstream ifs(calb_data_filepath);
    Int_t line_index = 0;
    while(ifs >> p0_buf >> p0e_buf >> p1_buf >> p1e_buf){
        
        if(line_index < 4){
            p0[0][line_index] = p0_buf;
            p0e[0][line_index] = p0e_buf;
            p1[0][line_index] = p1_buf;
            p1e[0][line_index] = p1e_buf;
            std::cout << Form("\tiBoard : 0, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            std::cout << Form("\t\t%lf %lf %lf %lf\n", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        else if(line_index < 8){
            p0[1][line_index-4] = p0_buf;
            p0e[1][line_index-4] = p0e_buf;
            p1[1][line_index-4] = p1_buf;
            p1e[1][line_index-4] = p1e_buf;
            std::cout << Form("\tiBoard : 1, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            std::cout << Form("\t\t%lf %lf %lf %lf\n", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        line_index++;
        if(line_index == 8){
            break;
        }
    }
    printf("\tline_index %d\n",line_index);
    Double_t dummy1,dummy2;
    while(ifs >> p0_res_buf >> p0e_res_buf >> dummy1 >> dummy2){
        if(line_index < 8){
            line_index++;
            continue;
        }
        if(line_index == 16){
            break;
        }
        if(line_index < 12){
            p0_res[0][line_index-8] = p0_res_buf;
            p0e_res[0][line_index-8] = p0e_res_buf;
            std::cout << Form("\tiBoard : 0, iCh : %d || energy resolution data loaded.\n", line_index % 4);
            printf("\tline_index %d\n",line_index);
            std::cout << Form("\t\t%lf %lf\n", p0_res_buf, p0e_res_buf);
        }
        else if(line_index < 16){
            p0_res[1][line_index-12] = p0_res_buf;
            p0e_res[1][line_index-12] = p0e_res_buf;
            std::cout << Form("\tiBoard : 1, iCh : %d || energy resolution data loaded.\n", line_index % 4);
            printf("\tline_index %d\n",line_index);
            std::cout << Form("\t\t%lf %lf\n", p0_res_buf, p0e_res_buf);
        }
        line_index++;
    }
    ifs.close();
}


// energy resolution plot
void energy_error(TString input_Folder = "./output/", TString key = "0204", Int_t iBoard = 0, Int_t iCh= 0){

    TString input_Filepath = Form("%sdata.txt", input_Folder.Data());
    std::ifstream ifs(input_Filepath);
    double energy, ch, sigma_ch, sigma_gaus, sigma_gaus_energy;

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    TCanvas* canvas = new TCanvas();
    TGraph *graph = new TGraph;
    int index_data = 0;
    Double_t energy_calib;
    Double_t max_energy_calib = 0;

    while(ifs >> energy >> ch >> sigma_ch >> sigma_gaus){
        std::cout << "Energy: " << energy << ", Ch: " << ch << ", Sigma_ch: " << sigma_ch << ", Sigma_gaus: " << sigma_gaus << std::endl;
        
        sigma_gaus_energy = sigma_gaus * p1[iBoard][iCh]; // energy error in keV
        graph->SetPoint(index_data, energy, sigma_gaus_energy);
        energy_calib = p1[iBoard][iCh] * ch + p0[iBoard][iCh];

        std::cout << p0[iBoard][iCh] << " " << p1[iBoard][iCh] << std::endl;

        if(energy_calib > max_energy_calib){
            max_energy_calib = energy_calib;
        }
        index_data++;
    }
    ifs.close();
    graph->SetTitle(Form("energy error from %s;Energy [keV];energy error [keV]", input_Filepath.Data()));
    graph->GetXaxis()->SetLimits(0.0, max_energy_calib*1.1);
    graph->GetYaxis()->SetRangeUser(0.0, 50);
    graph->SetMarkerStyle(20);
    graph->Draw("ap");

    TF1 *energy_error = new TF1("energy_error", "[0]*sqrt(x)", 0, 600);
    energy_error->SetParameter(0, 0.01*p0_res[iBoard][iCh]/(2.0*sqrt(2.0*log(2.0))));  // 初期値
    energy_error->SetLineColor(kBlue);
    energy_error->SetLineWidth(2);

    energy_error->Draw("same");
    canvas->Update();


    // 保存ファイル名を決定
    TString filename_figure = "energy_error.pdf";
    Int_t index = 1;
    while (gSystem->AccessPathName("./figure/" + filename_figure) == 0) {
        filename_figure = Form("energy_error_%d.pdf", index);
        index++;
    }
    canvas->SaveAs("./figure/" + filename_figure);
}
