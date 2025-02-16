#include <vector>
#include <TH1F.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TRadom.h>

void vector_hist() {
    TCanvas* c1 = new TCanvas("c1", "Histograms", 800, 600);

    // ヒストグラムを格納するベクター
    std::vector<TH1F*> histograms;

    for (int i = 0; i < 5; ++i) {
        // ROOT グローバル領域にヒストグラム作成
        gROOT->cd();
        TH1F* hist = new TH1F(Form("hist_%d", i), Form("Histogram %d", i), 100, 0, 100);
        for (int j = 0; j < 1000; ++j) {
            hist->Fill(gRandom->Gaus(50 + i * 10, 10));
        }

        hist->SetLineColor(i + 1);
        hist->SetLineWidth(2);
        histograms.push_back(hist);
    }

    // 最初のヒストグラムを描画
    histograms[0]->Draw();

    // 残りのヒストグラムを重ねて描画
    for (size_t i = 1; i < histograms.size(); ++i) {
        histograms[i]->Draw("SAME");
    }

    c1->Update();
}