#include <TH2.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TSpectrum.h>
#include <vector>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <TLine.h>
#include <TPad.h>

#include <fstream>
#include <filesystem>
#include <TSystem.h>

#include <iomanip>
#include <chrono>
#include <ctime> //時刻情報

#include <TLegend.h>

#include <fstream>

#include <TApplication.h>
#include <TFile.h>

void compare_pedestal(TString filename1, TString filename2){
    TFile *file1 = new TFile(filename1);
    TFile *file2 = new TFile(filename2);

    TTree *tree1 = (TTree*)file1->Get("treeDRS4BoardEvent");
    TTree *tree2 = (TTree*)file2->Get("treeDRS4BoardEvent");

    TCanvas *c1 = new TCanvas("name", "title", 1200, 800);
    
    tree1->Draw("pedestal[0][0]>>hist1(500,-0.02,0.02)");
    TH1F *hist1 = (TH1F*)gPad->GetPrimitive("hist1");
    hist1->SetLineColor(kRed);

    tree2->Draw("pedestal[0][0]>>hist2(500,-0.02,0.02)");
    TH1F *hist2 = (TH1F*)gPad->GetPrimitive("hist2");
    hist2->SetLineColor(kBlue);

    hist1->Draw();
    hist2->Draw("LSAME");


}