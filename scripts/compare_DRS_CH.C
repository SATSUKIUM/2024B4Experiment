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

#include <fstream>
#include <filesystem>
#include <iostream>
#include <TSystem.h>

#include <TSystemDirectory.h>
#include <TFile.h>
#include <TLegend.h>
#include <TLatex.h>

Double_t extract_Vpp(const TString filePath){
    TString fileName = filePath(filePath.Last('/')+1, filePath.Length()-filePath.Last('/'));

    Int_t firstUnderscorePos = fileName.Index('_');
    Int_t seconUnderscorePos = fileName.Index('_', firstUnderscorePos+1);
    Int_t thirdUnderscorePos = fileName.Index('_', seconUnderscorePos+1);
    Int_t fourthUnderscorePos = fileName.Index('_', thirdUnderscorePos+1);

    TString VppStr = fileName(thirdUnderscorePos+1, fourthUnderscorePos);
    Double_t VppNum = VppStr.Atoi();

    return (VppNum);

}

Double_t compare_DRS_CH(TString DIR_PATH = "../data/20241215/master/CH1_2", Int_t iCh_1 = 0, Int_t iCh_2 = 1) {
    TCanvas *c1 = new TCanvas("c1", "compare measured voltage between CHs of DRS4 Board", 800, 600);
    c1->SetGrid();
    gStyle->SetOptStat(0);

    // グラフ1 (iCh_1)
    TGraph *graph1 = new TGraph;
    graph1->SetMarkerStyle(20);
    graph1->SetMarkerColor(kBlue);
    graph1->SetTitle(";Function generator Vpp [mV];measured Vmax [mV]");
    graph1->Draw("AP");

    // グラフ2 (iCh_2)
    TGraph *graph2 = new TGraph;
    graph2->SetMarkerStyle(21); // 違いをつけるために別のマーカー
    graph2->SetMarkerColor(kRed);

    // 凡例を作成
    TLegend *legend = new TLegend(0.8, 0.1, 0.9, 0.3); // 右下に配置
    legend->SetBorderSize(1);
    legend->SetFillColor(0);
    legend->SetTextSize(0.03); // 文字サイズを設定（0.04は適宜調整）

    // .rootファイルを探す
    TSystemDirectory Dir("dir", DIR_PATH);
    TList *Filelist = Dir.GetListOfFiles();

    std::vector<TString> fileNames;
    TIter next(Filelist);
    TObject *fileObj;

    Int_t graph_index = 0;

    while ((fileObj = next())) {
        TSystemFile *file = (TSystemFile *)fileObj;
        TString fileName = file->GetName();
        if (fileName.EndsWith(".root")) {
            fileNames.push_back(fileName);
            std::cout << fileName << std::endl;
        } else {
            continue;
        }

        Double_t VppSet = extract_Vpp(fileName);
        TString filePath = DIR_PATH + "/" + fileName;
        TFile *f = TFile::Open(filePath);
        if (f && !f->IsZombie()) {
            TTree *tree = (TTree *)f->Get("treeDRS4BoardEvent");
            TTree *tree_info = (TTree *)f->Get("treeDRS4BoardInfo");

            Int_t numOfBoards;
            tree_info->SetBranchAddress("numOfBoards", &numOfBoards);
            tree_info->GetEntry(0);

            Double_t waveform[numOfBoards][4][1024];
            tree->SetBranchAddress("waveform", waveform);

            Int_t nentries = tree->GetEntriesFast();
            Double_t avgMaxVoltage1 = 0;
            Double_t avgMinVoltage1 = 0;
            Double_t avgMaxVoltage2 = 0;
            Double_t avgMinVoltage2 = 0;

            for (Int_t eventID = 0; eventID < nentries; eventID++) {
                tree->GetEntry(eventID);

                Double_t maxVoltage1 = -0.5, minVoltage1 = 0.5;
                Double_t maxVoltage2 = -0.5, minVoltage2 = 0.5;

                for (Int_t iCell = 0; iCell < 1024; iCell++) {
                    // iCh_1
                    Double_t voltage1 = waveform[0][iCh_1][iCell];
                    if (voltage1 > maxVoltage1) maxVoltage1 = voltage1;
                    if (voltage1 < minVoltage1) minVoltage1 = voltage1;

                    // iCh_2
                    Double_t voltage2 = waveform[0][iCh_2][iCell];
                    if (voltage2 > maxVoltage2) maxVoltage2 = voltage2;
                    if (voltage2 < minVoltage2) minVoltage2 = voltage2;
                }
                avgMaxVoltage1 += maxVoltage1;
                avgMinVoltage1 += minVoltage1;
                avgMaxVoltage2 += maxVoltage2;
                avgMinVoltage2 += minVoltage2;
            }

            avgMaxVoltage1 /= nentries;
            avgMinVoltage1 /= nentries;
            avgMaxVoltage2 /= nentries;
            avgMinVoltage2 /= nentries;

            // iCh_1のプロット
            graph1->SetPoint(graph_index, VppSet, (avgMaxVoltage1 - avgMinVoltage1) * 1000);

            // iCh_2のプロット
            graph2->SetPoint(graph_index, VppSet, (avgMaxVoltage2 - avgMinVoltage2) * 1000);

            // 凡例の更新
            if (graph_index == 0) { // 1度だけ追加
                legend->AddEntry(graph1, Form("CH%d", iCh_1 + 1), "p");
                legend->AddEntry(graph2, Form("CH%d", iCh_2 + 1), "p");
            }

            graph_index++;
        }
    }

    // タイトルにディレクトリ名を設定
    graph1->SetTitle(Form("Measured Vmax vs Vpp - Directory: %s", DIR_PATH.Data()));
    graph1->GetXaxis()->SetTitle("Function generator Vpp [mV]");
    graph1->GetYaxis()->SetTitle("Measured Vpp [mV]");
    // x軸の範囲を設定 (ゼロスタート)
    graph1->GetXaxis()->SetLimits(0, graph1->GetXaxis()->GetXmax());
    // y軸の範囲を設定 (ゼロスタート)
    graph1->GetYaxis()->SetRangeUser(0, graph1->GetYaxis()->GetXmax());

    // グラフ1の色を設定
    Int_t graphColor = kRed;  // 例として赤に設定
    graph1->SetLineColor(graphColor);  // グラフ1の線の色
    graph1->SetMarkerColor(graphColor);  // グラフ1のマーカーの色
    Int_t graphColor2 = kBlue;  // 例として青に設定
    graph2->SetLineColor(graphColor2);
    graph2->SetMarkerColor(graphColor2);

    // グラフ1に直線フィッティング
    TF1 *fit1 = new TF1("fit1", "pol1", 0, graph1->GetXaxis()->GetXmax());

    // 直線の色をグラフの色に合わせる
    fit1->SetLineColor(graphColor);

    fit1->SetNpx(1000);
    graph1->Fit(fit1, "R");  // "R"は範囲を指定してフィッティング
    

    // グラフ2に直線フィッティング
    TF1 *fit2 = new TF1("fit2", "pol1", 0, graph2->GetXaxis()->GetXmax());

    fit2->SetLineColor(graphColor2);
    
    fit2->SetNpx(1000);
    graph2->Fit(fit2, "R");  // "R"は範囲を指定してフィッティング
    


    
    

    

    
    

    

    graph1->Draw("AP"); // 軸を描画
    graph2->Draw("P SAME"); // 同じキャンバスに追加
    legend->Draw(); // 凡例を描画

    // // フィッティングパラメータを取得
    // Double_t slope = fit1->GetParameter(1);  // 勾配（直線の傾き）
    // Double_t intercept = fit1->GetParameter(0);  // 切片（y切片）

    // // フィッティングパラメータを左上に表示
    // TLatex *latex1 = new TLatex();
    // latex1->SetTextSize(0.04);  // 文字サイズを設定
    // latex1->SetTextAlign(12);  // 左上に配置
    // latex1->DrawLatex(1, 2, Form("y = %.2f x + %.2f", slope, intercept));

    // // 同様にグラフ2にもフィッティングパラメータを表示
    // Double_t slope2 = fit2->GetParameter(1);
    // Double_t intercept2 = fit2->GetParameter(0);

    // TLatex *latex2 = new TLatex();
    // latex2->SetTextSize(0.04);
    // latex2->SetTextAlign(12);
    // latex2->DrawLatex(1, 3, Form("y = %.2f x + %.2f", slope2, intercept2));

    // // キャンバスの余白を調整して位置変更を反映
    // c1->SetMargin(0.15, 0.05, 0.1, 0.1);  // 左右上下の余白を調整


    c1->Update();
    c1->SaveAs(Form("%s/CH_%d_and_%d.pdf",DIR_PATH.Data(), iCh_1+1, iCh_2+1));
    c1->SaveAs(Form("%s/CH_%d_and_%d.png",DIR_PATH.Data(), iCh_1+1, iCh_2+1));

    return graph_index;
}