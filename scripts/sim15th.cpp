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

double_t func1(double theta, double phi, double K){

    // 入射gammaのエネルギー[MeV]と波数[/m]
    double inEnergy = 511e-3;
    double inWavenum = inEnergy * 1.6e-19 / 6.6e-34 / 3e9;
    double scatEnergy = inEnergy / (2 - cos(theta * M_PI / 180));

    // 因子g
    double_t g = (scatEnergy / inEnergy) + (inEnergy / scatEnergy);

    // 定数たち
    double K_Bell = pow(0.5, 0.5);
    // double K = 0.5;
    // double K = pow(0.5, 0.5);

    double width = 2.0;
    double length = 12.0;
    double r_scat_to_detect = 25.0;

    double detect_size = 2.5 * 2.5 * M_PI;
    double scat_size = 2.5 * 2.5 * M_PI;
    double scat_width = 5.0;
    double r_source_to_scat = 25.0;

    double degree = atan(r_scat_to_detect/length);

    double density_GSO = 6.71;
    double density_NaI = 3.67;
    double electron_GSO = 182.0;
    double electron_NaI = 64.0;
    double molweight_GSO = 422.58;
    double molweight_NaI = 149.9;
    double intensity = 2.0e6;

    double avogadro = 6.02e23;
    double r_e = 2.8e-13;

    double time = 60*60*24*14;

    double Energy_resolution = 0.1;
    double_t sigmaE = Energy_resolution * scatEnergy / (2 * sqrt(2 * log(2)));
    double_t d_theta = pow((2 - cos(theta * M_PI / 180)), 2) * sigmaE / (sqrt(1 - pow(cos(theta * M_PI / 180), 2)) * inEnergy);
    double_t d_phi = 2 * asin(width * sin(theta * M_PI / 180) / (2 * r_scat_to_detect));


    std::ifstream ifs("../data/sim/GSO_pe.txt");
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open file." << std::endl;
    }

    vector<double> E_vals, absorb_vals;
    double E, ab;

    while (ifs >> E >> ab) {
        E_vals.push_back(E);
        absorb_vals.push_back(ab);
    }
    ifs.close();

    double_t E_val = inEnergy / (2 - cos(theta * M_PI / 180));
    double_t absorb_val = 0.0;
    double_t absorb_val_NaI = 1.006e-1;
    int vec_size = E_vals.size();
    for(int i=0; i<500; i++){
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

    double_t photoelec_ratio_GSO = 1 - exp(-1.0 * density_GSO * absorb_val * width / sin(theta * M_PI / 180));
    double_t photoelec_ratio_NaI = 1 - exp(-1.0 * density_NaI * absorb_val_NaI * scat_width / sin(M_PI/2));
    double_t comp_ratiofactor = scat_width * density_NaI * electron_NaI * avogadro / molweight_NaI;

    double_t solid_source_to_scat = scat_size / (4 * M_PI * r_source_to_scat * r_source_to_scat);
    double_t solid_scat_to_detectA = detect_size / pow(r_scat_to_detect, 2);
    double_t solid_scat_to_detectB = d_phi * d_theta;

    double_t A = (g - pow(sin(theta * M_PI / 180), 2)) * (g - pow(sin(M_PI/2), 2)) * pow(r_e, 4) / (pow(2 - cos(theta * M_PI / 180), 2) * pow(2 - cos(M_PI/2), 2) * 16);
    double_t B = pow(sin(theta * M_PI / 180), 2) * pow(sin(M_PI/2), 2) * K * pow(r_e, 4)/ (pow(2 - cos(theta * M_PI / 180), 2) * pow(2 - cos(M_PI/2), 2) * 16);

    double_t counts = time * intensity * solid_source_to_scat * (A - (B * cos(2 * phi * M_PI / 180))) * comp_ratiofactor * comp_ratiofactor * solid_scat_to_detectA * solid_scat_to_detectB * photoelec_ratio_NaI * photoelec_ratio_GSO;
    double_t rate = counts / time;

    // cout << "A = " << time * intensity * solid_source_to_scat * A * comp_ratiofactor * comp_ratiofactor * solid_scat_to_detectA * solid_scat_to_detectB * photoelec_ratio_NaI * photoelec_ratio_GSO << endl;
    // cout << "B = " << time * intensity * solid_source_to_scat * B * comp_ratiofactor * comp_ratiofactor * solid_scat_to_detectA * solid_scat_to_detectB * photoelec_ratio_NaI * photoelec_ratio_GSO << endl;
    
    return rate;
    // return counts/time;
    // return counts;
}

void Integral(){
    double length = 12.0;
    double width = 2.0;
    double r_scat_to_detect = 25.0;
    double degree_x = atan(r_scat_to_detect/length);
    double degree_y = 2 * atan(width / (2 * r_scat_to_detect));

    int l_x = 10;
    int l_y = 10;
    int abs_points = 5;
    double x[l_x], y[l_y], count_sum[abs_points], y_center[abs_points];
    double x_min = degree_x * 180 / M_PI;
    double x_max = 90;
    // double x_min = M_PI / 2;
    // double x_max = M_PI - degree_x;
    double y_min;
    double y_max;
    // double y_center;

    double time = 60 * 60 * 24 * 14;
    // double counts= 0.0;
    cout << "time = " << time << endl;

    for(int i = 0; i < abs_points; i++){
        // y_center = i * M_PI / 4;
        // y_min = y_center - degree_y / 2;
        // y_max = y_center + degree_y / 2;
        count_sum[i] = 0.0;
        
        y_center[i] = i * 45;
        y_min = i * 45 - (degree_y * 180 / M_PI / 2);
        y_max = y_min + (degree_y * 180 / M_PI);
        double delta_x = (x_max - x_min) / l_x;
        double delta_y = (y_max - y_min) / l_y;
        double delta_x_rad = delta_x * M_PI / 180;
        double delta_y_rad = delta_y * M_PI / 180;
        // if(i == 4){
        //     y_min = i * M_PI / 4 - degree_y;
        //     y_max = i * M_PI / 4;
        // }
        for(int m = 0; m < l_x; m++){
            x[m] = x_min + m * delta_x;
            for(int n = 0; n < l_y; n++){
                y[n] = y_min + n * delta_y;
                count_sum[i] += func1(x[m], y[n], 0.78) * delta_x_rad * delta_y_rad * time;
                
            }
        }
        // counts += count_sum[i];
        cout << "phi_center = " << y_center[i] << " ; count = " << count_sum[i] << endl;
        // cout << y_max - y_min << endl;
        // cout << x_max - x_min << endl;
    }
    TGraph* graph = new TGraph(abs_points, y_center, count_sum);
    graph->SetTitle(Form("Expected Count of Absorbers (%fdays);#phi;Counts", time/60/60/24));
    graph->SetMarkerSize(0.6);
    graph->SetMarkerStyle(8);
    graph->GetXaxis()->SetLabelSize(0.04);
    graph->GetYaxis()->SetLabelSize(0.04);
    graph->GetXaxis()->SetTitleSize(0.05);
    graph->GetYaxis()->SetTitleSize(0.05);
    graph->GetXaxis()->SetTitleOffset(0.9);
    graph->GetYaxis()->SetTitleOffset(0.9);

    graph->Draw("AP");
    

}


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