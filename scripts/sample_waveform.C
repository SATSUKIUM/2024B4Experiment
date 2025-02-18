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
#include <TColor.h>


#include <fstream>
#include <filesystem>
#include <TSystem.h>

#include <iomanip>
#include <chrono>
#include <ctime> //時刻情報

#include <TLegend.h>

#include <fstream>

#include <TApplication.h>
#include <TChainElement.h>
#include <TObjArray.h>
#include <TFile.h>

void sample_waveform(TString filepath = "../data/PhysicsRun/Run_005.dat.root") {
    // ROOTファイルを開く
    TFile *file = TFile::Open(filepath);
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    // TTree を取得
    TTree *tree = (TTree*)file->Get("treeDRS4BoardEvent");
    if (!tree) {
        std::cerr << "Error: treeDRS4BoardEvent not found!" << std::endl;
        file->Close();
        return;
    }

    // 配列のポインタをセット
    Double_t waveform[2][4][1024];
    Double_t time[2][4][1024];
    tree->SetBranchAddress("waveform", waveform);
    tree->SetBranchAddress("time", time);

    // 9813番目のエントリを取得
    // tree->GetEntry(9813);
    tree->GetEntry(27201);

    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Waveform Plot", 800, 600);
    c1->SetGrid();

    // TGraph を作成（3つの異なる系列）
    Int_t colors[4] = {kAzure, kCyan+3, kOrange-1,kViolet+7};  // 異なる色を指定
    Int_t index = 0;
    TGraph *graphs[4];

    int channels[4][2] = {{0, 0}, {0, 3}, {1, 2},{0,2}};  // iBoard, iCh の組み合わせ

    for (int i = 0; i < 4; i++) {
        int iBoard = channels[i][0];
        int iCh = channels[i][1];

        graphs[i] = new TGraph(1024, time[iBoard][iCh], waveform[iBoard][iCh]);
        graphs[i]->GetYaxis()->SetRangeUser(-0.25, 0.05);
        graphs[i]->GetXaxis()->SetLimits(0, 400);
        graphs[i]->SetLineColor(colors[i]);
        graphs[i]->SetLineWidth(3);
        graphs[i]->SetTitle("Waveform;Time [ns];Amplitude");
        
        if (i == 0)
            graphs[i]->Draw("AL");
        else
            graphs[i]->Draw("L SAME");
    }

    // // 凡例を追加
    // auto legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    // legend->AddEntry(graphs[0], "S1(NaI)", "l");
    // legend->AddEntry(graphs[1], "S2(NaI)", "l");
    // legend->AddEntry(graphs[2], "A2 90°(GSO)", "l");
    // legend->AddEntry(graphs[3], "A1(NaI)", "l");
    // legend->Draw();

    c1->Update();
}