#include <iostream>
#include <cmath>
#include <TRandom3.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TGraph2DErrors.h>
#include <TF1.h>
#include <TF2.h>
#include <TH1.h>
#include <TH2.h>
#include <TLegend.h>
#include <TMath.h>
#include <TGraph.h>
#include <TSpline.h>
#include <fstream>
#include <vector>
#include <TStyle.h>
#include <TH3D.h>

using namespace std;

Double_t gamma_factor(Double_t theta){

    Double_t inEnergy = 511e-3;
    Double_t scatEnergy = inEnergy / (2 - cos(theta * M_PI / 180));

    return (scatEnergy / inEnergy) + (inEnergy / scatEnergy);
}

Double_t GetRate(Double_t theta, Double_t phi, Double_t K){

    //　theta, phiは度

    // 入射gammaのエネルギー[MeV]
    Double_t inEnergy = 511e-3;

    // 定数たち
    Double_t K_Bell = pow(0.5, 0.5);
    // double K = 0.5;
    // double K = pow(0.5, 0.5);

    Double_t S1_r = 2.5;
    Double_t S1_hight = 5.0;

    Double_t S2_r = 2.5;
    Double_t S2_hight = 5.0;

    Double_t A1_r = 2.5;
    Double_t A1_hight = 5.0;

    Double_t A2_width = 2.0;
    Double_t A2_hight = 2.0;
    Double_t A2_length = 12.0;

    Double_t r_S1_to_A1 = 25.0;
    Double_t r_S2_to_A2 = 25.0;
    Double_t r_Na_to_S1 = 25.0;
    Double_t r_Na_to_S2 = 25.0; // 長さはcmで統一

    Double_t S1_size = pow(S1_r, 2) * M_PI;
    Double_t S2_size = pow(S2_r, 2) * M_PI;
    Double_t A1_size = pow(A1_r, 2) * M_PI;
    

    Double_t density_GSO = 6.71; // [g/cm^3]
    Double_t density_NaI = 3.67; // [g/cm^3]
    Double_t electron_GSO = 182.0; // Gd 64*2 + Si 14 + O 8*5
    Double_t electron_NaI = 64.0; // Na 11 + I 53
    Double_t molweight_GSO = 422.58;// Gd 157.25*2 + Si 28.08 + O 16*5 [g/mol]
    Double_t molweight_NaI = 149.9; // Na 23 + I 126.9 [g/mol]
    
    Double_t avogadro = 6.02e23; // [/mol]
    Double_t r_e = 2.8e-13; // [cm]

    Double_t intensity = 2.0e6; // [/s]
    Double_t time = 60*60*24*14; // [s]


    Double_t delta_theta = 0.1 * M_PI / 180; //　0.1度に当たるラジアンがθの微少量とする
    Double_t delta_phi = 0.1 * M_PI / 180; //　0.1度に当たるラジアンがφの微少量とする


    std::ifstream ifs("./cfg/GSO_pe.txt"); // 以下、あるエネルギーでの光電吸収に対する減衰係数を取得
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open file." << std::endl;
    }

    vector<Double_t> E_vals, absorb_vals;
    Double_t E, ab;

    while (ifs >> E >> ab) {
        E_vals.push_back(E);
        absorb_vals.push_back(ab);
    }
    ifs.close();

    Double_t E_val = inEnergy / (2 - cos(theta * M_PI / 180));
    Double_t absorb_val = 0.0;
    Double_t absorb_val_NaI = 1.006e-1;
    Int_t vec_size = E_vals.size();
    for(Int_t i=0; i<500; i++){
        if(E_val >= E_vals[i] && E_val <= E_vals[i+1]){
            if(E_val == E_vals[i]){
                absorb_val = absorb_vals[i];
            }
            else if(E_val == E_vals[i+1]){
                absorb_val = absorb_vals[i+1];
            }
            else{
                absorb_val = ((absorb_vals[i] * (E_val - E_vals[i+1])) + (absorb_vals[i+1] * (E_vals[i] - E_val))) / (E_vals[i] - E_vals[i+1]);
            }
        }
    }

    Double_t absorb_A1 = 1 - exp(-1.0 * density_NaI * absorb_val_NaI * A1_hight); // A1で吸収される確率
    Double_t absorb_A2 = 1 - exp(-1.0 * density_GSO * absorb_val * A2_hight / sin(theta * M_PI / 180)); // A2で吸収される確率
    
    Double_t comp_factor_S1 = S1_hight * density_NaI * electron_NaI * avogadro / molweight_NaI; // S1でコンプトン散乱する確率の定数部分
    Double_t comp_factor_S2 = S2_hight * density_NaI * electron_NaI * avogadro / molweight_NaI; // S2でコンプトン散乱する確率の定数部分
    Double_t comp_factor = comp_factor_S1 * comp_factor_S2 * pow(r_e, 4) / 64;

    Double_t solid_Na_to_S1 = S1_size / (4 * M_PI * pow(r_Na_to_S1, 2)); // 線源からS1を見た時の立体角(線源は点としている)、線源からS2にも同時に入るのでS2の立体角は考えない
    Double_t solid_S1_to_A1 = A1_size / (4 * M_PI * pow(r_S1_to_A1, 2)); // S1の光電面の中心からA1を見た時の立体角
    Double_t solid_S2_to_A2 = delta_phi * delta_theta / 4 * M_PI; // S2の光電面の中心からA2上の微小面積を見た時の立体角


    Double_t counts = time * intensity
                    * solid_Na_to_S1 * solid_S1_to_A1 
                    * comp_factor * solid_S2_to_A2 
                    * pow((2 - cos(theta * M_PI / 180)), -2) 
                    * ((gamma_factor(90) - 1) * (gamma_factor(theta) - pow(sin(theta * M_PI / 180), 2)) - (K * pow(sin(theta * M_PI / 180), 2) * cos(2 * phi * M_PI / 180))) 
                    * absorb_A1 * absorb_A2;
    
    Double_t rate = counts / time;

    return rate;
    // return counts/time;
    // return counts;
}

void PlotRateIntegral(Double_t days = 1.0, Double_t K = 0.78){


    Double_t A2_length = 12.0;
    Double_t A2_width = 2.0;
    Double_t r_S2_to_A2 = 25.0;

    Double_t delta_theta = 0.1 * M_PI / 180; //　0.1度に当たるラジアンがθの微少量とする
    Double_t delta_phi = 0.1 * M_PI / 180; //　0.1度に当たるラジアンがφの微少量とする

    Double_t theta_min_rad = atan(r_S2_to_A2 / A2_length); // およそ64度のラジアン
    Double_t phi_range_rad = 2 * atan((A2_width / 2) / r_S2_to_A2); //　およそ4.6度のラジアン

    Double_t l_theta_d = (M_PI / 2 - theta_min_rad) / delta_theta;
    Double_t l_phi_d = phi_range_rad / delta_phi;

    Int_t l_theta = static_cast<Int_t>(l_theta_d);
    Int_t l_phi = static_cast<Int_t>(l_phi_d); // 角度の範囲内にある0.1度刻みの点の個数

    Int_t abs_points = 5; // 吸収体の個数
    // int abs_points = 181;

    Double_t x[l_theta], y[l_phi], count_sum[abs_points], y_center[abs_points], count_error[abs_points];

    Double_t x_min = theta_min_rad * 180 / M_PI; // 度
    Double_t x_max = 90; // 度
    Double_t y_min; // 度
    Double_t y_max; // 度

    // double time = 60 * 60 * 24 * 7 * 1;
    // double counts= 0.0;

    // cout << "time = " << time/60/60/24 << "days" << endl;

    for(Int_t i = 0; i < abs_points; i++){

        count_sum[i] = 0.0;
        
        y_center[i] = i * (180 / (abs_points - 1));
        y_min = y_center[i] - ((phi_range_rad / 2) * 180 / M_PI);
        y_max = y_center[i] + ((phi_range_rad / 2) * 180 / M_PI);


        for(Int_t m = 0; m < l_theta; m++){

            x[m] = x_min + m * delta_theta * 180 / M_PI;

            for(Int_t n = 0; n < l_phi; n++){

                y[n] = y_min + n * delta_phi * 180 / M_PI;
                count_sum[i] += GetRate(x[m], y[n], K) * days * 24 * 60 * 60;
                
            }
        }

        count_error[i] = sqrt(count_sum[i]);
        cout << Form("phi = %.0f, Counts = %.3f", y_center[i], count_sum[i]) << endl;

    }

    cout << "l_theta = " << l_theta << endl;
    cout << "l_phi = " << l_phi << endl;


    // TGraph* graph = new TGraph(abs_points, y_center, count_sum);
    TGraphErrors* graph = new TGraphErrors(abs_points, y_center, count_sum, 0, count_error);
    graph->SetTitle(Form("Expected Count of Absorbers (%.0f days, #kappa = %.3f);#phi [degree];Counts", days, K));
    graph->SetMarkerSize(0.6);
    graph->SetMarkerStyle(8);
    graph->GetXaxis()->SetLabelSize(0.04);
    graph->GetYaxis()->SetLabelSize(0.04);
    graph->GetXaxis()->SetTitleSize(0.05);
    graph->GetYaxis()->SetTitleSize(0.05);
    graph->GetXaxis()->SetTitleOffset(0.9);
    graph->GetYaxis()->SetTitleOffset(0.9);
    graph->GetXaxis()->SetRangeUser(-10, 190);

    graph->Draw("AP");
    

}

//　以降は無視して

void sim15th() {

    double length = 12.0;
    double width = 2.0;
    double r_scat_to_detect = 25.0;
    double degree_x = atan(r_scat_to_detect/length) * 180 / M_PI;
    double degree_y = atan(width / r_scat_to_detect);
    TCanvas* c1 = new TCanvas("c1", "c1", 800, 600);
    // TH3D* h1 = new TH3D("h1", "h1", 10, degree_x, 90, 10, 0, 180, 10, 0, 0.01);
    // TF2* func = new TF2("f", "func1(x, y)", degree, M_PI/2 + degree, 0, M_PI);
    // TF2* func = new TF2("f", "func1(x, y)", M_PI/2, M_PI/2 + degree, 0, M_PI);    
    // TF2* func = new TF2("f", "func1(x, y)", 0, M_PI, 0, M_PI);

    // TF2* func = new TF2("f", "func1(x, y)", degree_x, 90, 0, 180);

    // func->SetTitle("R with kappa_initial = 1.0;Theta[degree];Phi[degree];R[/day]");
    // func->GetXaxis()->SetLabelSize(0.04);
    // func->GetYaxis()->SetLabelSize(0.04);
    // func->GetZaxis()->SetLabelSize(0.04);
    // func->GetXaxis()->SetTitleSize(0.05);
    // func->GetYaxis()->SetTitleSize(0.05);
    // func->GetZaxis()->SetTitleSize(0.05);
    // func->GetYaxis()->SetNdivisions(9);
    // func->GetXaxis()->SetTitleOffset(1.2);
    // func->Draw("surf2");

    auto legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    auto hh = new TH2D("h", "", 10, 0, 180, 10, 40, 120);
    hh->SetStats(0);
    hh->SetTitleSize(0.05);
    hh->GetXaxis()->SetLabelSize(0.04);
    hh->GetYaxis()->SetLabelSize(0.04);
    hh->GetXaxis()->SetTitleSize(0.05);
    hh->GetYaxis()->SetTitleSize(0.05);
    hh->SetTitle("#phi-dependence of R with #theta = 90[degree];#phi[degree];R[/day]");
    hh->GetXaxis()->SetTitleOffset(0.9);
    hh->GetYaxis()->SetTitleOffset(0.9);
    hh->Draw();

    TF1* func_a = new TF1("f", "func1([0], x, [1])", 0, 180);
    func_a->SetParameters(90, 1.0);
    
    func_a->SetLineColor(kRed);
    legend->AddEntry(func_a, "#kappa = 1.0");
    func_a->Draw("same");

    TF1* func_b = new TF1("f", "func1([0], x, [1])", 0, 180);
    func_b->SetParameters(90, 0.5);
    func_b->SetLineColor(kGreen);
    legend->AddEntry(func_b, "#kappa = 0.5");
    func_b->Draw("same");
    legend->Draw();

    // TF1* func = new TF1("f", "func1(x, [0])", degree, (M_PI/2 + degree));
    // TF1* func = new TF1("f", "func1([0], x)", 0, M_PI);
    // func->SetParameters(M_PI/4);
    // func->SetTitle(";theta;Counts");
    // func->SetTitle(";phi;Counts");    
    // func->Draw();
    
}

Double_t Integrand(Double_t fit_par0, Double_t fit_par1, Double_t phi_prime){
    return fit_par0 - fit_par1 * cos(phi_prime * M_PI / 180);
}

Double_t Integral_over_phi_range(Double_t fit_par0, Double_t fit_par1, Double_t phi){
    
    Double_t A2_width = 2.0;
    Double_t r_S2_to_A2 = 25.0;

    Double_t delta_phi = 0.1 * M_PI / 180; //　0.1度に当たるラジアンがφの微少量とする

    Double_t phi_range_rad = 2 * atan((A2_width / 2) / r_S2_to_A2); //　およそ4.6度のラジアン
    Double_t phi_range = phi_range_rad * 180 / M_PI;

    Double_t phi_min = phi - phi_range / 2;

    Double_t l_phi_d = phi_range_rad / delta_phi;

    Int_t l_phi = static_cast<Int_t>(l_phi_d); // 角度の範囲内にある0.1度刻みの点の個数

    Double_t sum = 0.0;

    for(Int_t i = 0; i < l_phi; i++){

        Double_t phi_tilde = phi_min + i * l_phi;
        sum += Integrand(fit_par0, fit_par1, phi_tilde) * delta_phi;

    }

    return sum;
}

Double_t GetKappa(TString input_Folder = "./cfg/"){

    TString input_Filepath = Form("%scounts_data.txt",input_Folder.Data());
    std::ifstream ifs(input_Filepath);

    TGraphErrors* graph = new TGraphErrors();

    Int_t index_data = 0;
    Double_t counts;

    while(ifs >> counts){
        graph->SetPoint(index_data, index_data * 45, counts);
        graph->SetPointError(index_data, 0, sqrt(counts));
        index_data++;
    }
    ifs.close();

    TF1* fitfunc = new TF1("fitfunc", "Integral_over_phi_range([0], [1], x)", 0, 180);
    fitfunc->SetParameters(10.0, 10.0);
    graph->Fit(fitfunc);
    graph->Draw("ap");

    return 0;
}