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

    TRandom3 randGen(0);


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

    Double_t x[l_theta], y[l_phi], y_center[abs_points], count_sum[abs_points], count_error[abs_points], count_measured[abs_points], count_measured_error[abs_points];

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
        count_measured[i] = randGen.Gaus(count_sum[i], sqrt(count_sum[i]));
        count_measured_error[i] = sqrt(count_measured[i]);
        count_error[i] = sqrt(count_sum[i]);
        cout << Form("phi = %.0f, Expected Counts = %.3f +/- %.3f, Measured Counts = %.3f +/- %.3f", y_center[i], count_sum[i], count_error[i], count_measured[i], count_measured_error[i]) << endl;
        // cout << Form("phi = %.0f, Counts = %.3f", y_center[i], count_sum[i]) << endl;

    }

    // cout << "l_theta = " << l_theta << endl;
    // cout << "l_phi = " << l_phi << endl;

    auto legend = new TLegend(0.7, 0.7, 0.9, 0.9);

    TGraphErrors* graph1 = new TGraphErrors(abs_points, y_center, count_sum, 0, count_error);
    // graph1->SetTitle(Form("Expected Count of Absorbers (%.0f days, #kappa = %.3f);#phi [degree];Counts", days, K));

    TGraphErrors* graph2 = new TGraphErrors(abs_points, y_center, count_measured, 0, count_measured_error);
    graph1->SetTitle(Form("Expected Counts and Measured Counts (%.0f days, #kappa = %.3f);#phi [degree];Counts", days, K));

    graph1->SetMarkerSize(0.6);
    graph1->SetMarkerStyle(8);
    graph1->GetXaxis()->SetLabelSize(0.04);
    graph1->GetYaxis()->SetLabelSize(0.04);
    graph1->GetXaxis()->SetTitleSize(0.05);
    graph1->GetYaxis()->SetTitleSize(0.05);
    graph1->GetXaxis()->SetTitleOffset(0.9);
    graph1->GetYaxis()->SetTitleOffset(0.9);
    graph1->GetXaxis()->SetRangeUser(-10, 190);
    graph1->SetMarkerColor(kRed);
    legend->AddEntry(graph1, "Expected");
    graph1->Draw("AP");

    graph2->SetMarkerSize(0.6);
    graph2->SetMarkerStyle(8);
    graph2->SetMarkerColor(kGreen);
    legend->AddEntry(graph2, "Measured");
    graph2->Draw("Psame");
    legend->Draw();

}

//　sim15th()は無視して

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
    return fit_par0 - fit_par1 * cos(2 * phi_prime * M_PI / 180);
}

Double_t Integral_over_phi_range(Double_t fit_par0, Double_t fit_par1, Double_t phi){
    
    Double_t A2_width = 2.0;
    Double_t r_S2_to_A2 = 25.0;

    // Double_t delta_phi_rad = 0.1 * M_PI / 180; //　0.1度に当たるラジアンがφの微少量とする
    Double_t delta_phi = 0.1;

    // Double_t phi_range_rad = 2 * atan((A2_width / 2) / r_S2_to_A2); //　およそ4.6度のラジアン
    Double_t phi_range = 2 * atan((A2_width / 2) / r_S2_to_A2) * 180 / M_PI;

    Double_t phi_min = phi - (phi_range / 2);

    Double_t l_phi_d = phi_range / delta_phi;

    Int_t l_phi = static_cast<Int_t>(l_phi_d); // 角度の範囲内にある0.1度刻みの点の個数

    Double_t sum = 0.0;

    for(Int_t i = 0; i < l_phi; i++){

        Double_t phi_tilde = phi_min + i * delta_phi;
        sum += Integrand(fit_par0, fit_par1, phi_tilde) * delta_phi;

    }

    return sum;
}

Double_t Integrand_divisor(Double_t theta_prime){

    Double_t inEnergy = 511e-3;
    Double_t density_GSO = 6.71; // [g/cm^3]
    Double_t A2_hight = 2.0;


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

    Double_t E_val = inEnergy / (2 - cos(theta_prime * M_PI / 180));
    Double_t absorb_val = 0.0;
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

    return pow(2 - cos(theta_prime * M_PI / 180), -2) * pow(sin(theta_prime * M_PI / 180), 2) * (1 - exp(-1.0 * density_GSO * absorb_val * A2_hight / sin(theta_prime * M_PI / 180)));

}

Double_t Integrand_dividend(Double_t theta_prime){

    Double_t inEnergy = 511e-3;
    Double_t density_GSO = 6.71; // [g/cm^3]
    Double_t A2_hight = 2.0;


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

    Double_t E_val = inEnergy / (2 - cos(theta_prime * M_PI / 180));
    Double_t absorb_val = 0.0;
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

    return (gamma_factor(90) - 1) * (gamma_factor(theta_prime) - pow(sin(theta_prime * M_PI / 180), 2)) * pow(2 - cos(theta_prime * M_PI / 180), -2) * (1 - exp(-1.0 * density_GSO * absorb_val * A2_hight / sin(theta_prime * M_PI / 180)));
}

Double_t GetKappa(TString input_Folder = "./cfg/sim16_kappa/001.txt"){

    gStyle->SetOptFit(1111);

    TString input_Filepath = input_Folder;
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

    TH2D* hh = new TH2D("h", "h", 10, -10, 190, 10, 0, 0.1); // 範囲変えて
    hh->SetStats(0);
    hh->SetTitle("The number of valid events;#phi[degree];Counts");
    hh->Draw();


    TF1* fitfunc = new TF1("fitfunc", "Integral_over_phi_range([0], [1], x)", 0, 180);
    fitfunc->SetParameters(100.0, 10.0);
    graph->Fit(fitfunc);
    graph->SetMarkerSize(0.6);
    graph->SetMarkerStyle(8);
    graph->Draw("Psame");

    Double_t sum_divisor = 0.0;
    Double_t sum_dividend = 0.0;

    Double_t A2_length = 12.0;
    Double_t r_S2_to_A2 = 25.0;

    // Double_t delta_theta_rad = 0.1 * M_PI / 180; //　0.1度に当たるラジアンがθの微少量とする
    Double_t delta_theta = 0.1;

    // Double_t theta_min_rad = atan(r_S2_to_A2 / A2_length); // およそ64度のラジアン
    Double_t theta_min = atan(r_S2_to_A2 / A2_length) * 180 / M_PI;

    Double_t l_theta_d = (90 - theta_min) / delta_theta;

    Int_t l_theta = static_cast<Int_t>(l_theta_d);

    for(int i = 0; i < l_theta; i++){

        Double_t theta_tilde = theta_min + i * delta_theta;
        sum_dividend += Integrand_dividend(theta_tilde) * delta_theta;
        sum_divisor += Integrand_divisor(theta_tilde) * delta_theta;

    }

    Double_t p0 = fitfunc->GetParameter(0);
    Double_t p1 = fitfunc->GetParameter(1);
    Double_t p0e = fitfunc->GetParError(0);
    Double_t p1e = fitfunc->GetParError(1);

    Double_t kappa = (p1 / p0) * (sum_dividend / sum_divisor);
    Double_t kappa_error = (sum_dividend / (sum_divisor * p0)) * sqrt(pow(p1 * p0e / p0 , 2) + pow(p1e, 2));

    cout << "kappa = " << kappa << endl;
    cout << "kappa_error = " << kappa_error << endl;

    // cout << "p0 = " << p0 << endl;
    // cout << "p0e = " << p0e << endl;
    // cout << "p1 = " << p1 << endl;
    // cout << "p1e = " << p1e << endl;

    // cout << "sum_dividend = " << sum_dividend << endl;
    // cout << "sum_divisor = " << sum_divisor << endl;
    // cout << "l_theta = " << l_theta << endl;
    // cout << "l_theta_d = " << l_theta_d << endl;


    return 0;
}