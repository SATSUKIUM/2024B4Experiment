#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <iostream>
#include <vector>
#include <TStyle.h>
#include <TColor.h>
#include <algorithm>  // std::sortを使用するために追加
#include <regex>  // 数字抽出のために追加

// ファイル名から数字を抽出する関数
int extract_number(const TString& fileName) {
    std::string fileStr = fileName.Data();
    std::regex rgx("\\d+");
    std::smatch match;
    if (std::regex_search(fileStr, match, rgx)) {
        return std::stoi(match.str());
    }
    return 0;
}

// ファイル名に含まれる数字以外の部分を取り出す関数
std::string extract_non_numeric(const TString& fileName) {
    std::string fileStr = fileName.Data();
    std::string non_numeric;
    for (char c : fileStr) {
        if (!isdigit(c)) {
            non_numeric += c;
        }
    }
    return non_numeric;
}

void overlay_histograms2() {
    // キャンバスの作成
    gStyle->SetOptStat(0);
    TCanvas *c1 = new TCanvas("c1", "Overlay Histograms from Files", 800, 600);
    c1->SetGrid();

    // ディレクトリ内の全ての.rootファイルを検索
    TString dirPath = "../../1214/huruno2";
    TSystemDirectory dir("dir", dirPath);
    TList* files = dir.GetListOfFiles();

    if (!files || files->GetSize() == 0) {
        std::cerr << "指定されたディレクトリが存在しないか、空です。" << std::endl;
        return;
    }

    // ファイル名のリストを取得
    std::vector<TString> fileNames;
    TIter next(files);
    TObject* fileObj;
    while ((fileObj = next())) {
        TSystemFile* file = (TSystemFile*)fileObj;
        TString fileName = file->GetName();
        if (fileName.EndsWith(".root")) {
            fileNames.push_back(fileName);
        }
    }

    std::sort(fileNames.begin(), fileNames.end(), [](const TString& a, const TString& b) {
        int numA = extract_number(a);
        int numB = extract_number(b);
        bool isNumA = isdigit(a[0]);
        bool isNumB = isdigit(b[0]);
        if (isNumA && !isNumB) return true;
        if (!isNumA && isNumB) return false;
        if (isNumA && isNumB) {
            return numA < numB;
        } else {
            return a < b;
        }
    });

    // ヒストグラムを格納するベクター
    std::vector<TH1F*> histograms;

    // ファイルごとにヒストグラムを作成
    for (const auto& fileName : fileNames) {
        TString filePath = dirPath + "/" + fileName;

        // ファイルを開く
        TFile* f = TFile::Open(filePath);
        
        if (f && !f->IsZombie()) {
            // TTreeを取得
            TTree* tree = (TTree*)f->Get("treeDRS4BoardEvent");
            if (tree) {
                Int_t nentries = tree->GetEntriesFast();
                Double_t adcSum[1][4];
                tree->SetBranchAddress("adcSum", &adcSum);

                // ヒストグラム作成
                TH1F* hist = new TH1F(Form("hist_%s", fileName.Data()), "title", 500, 0, 300);

                for (size_t i = 0; i < nentries; ++i) {
                    tree->GetEntry(i);
                    if (adcSum[0][0] == 0) {
                        hist->Fill(-adcSum[0][1]);
                    } else {
                        hist->Fill(-adcSum[0][0]);
                    }
                }
                histograms.push_back(hist);
            }
        }
    }

    // 虹色のグラデーション（赤→橙→黄→緑→青→紫→ピンク）
    size_t numFiles = fileNames.size();
    std::vector<int> colors;
    double maxY = 0;
    for (auto hist : histograms) {
        if (hist->GetMaximum() > maxY) {
            maxY = hist->GetMaximum();
        }
    }

    // グラフ描画の色設定
    for (size_t i = 0; i < numFiles; ++i) {
        double t = (double)i / (numFiles - 1);

        int red = 0, green = 0, blue = 0;

        // 赤→橙→黄
        if (t < 0.25) {
            red = 255;
            green = (int)(255 * (t / 0.25));  // 0 → 1
        }
        // 黄→緑
        else if (t < 0.5) {
            red = (int)(255 * (1 - (t - 0.25) / 0.25));
            green = 255;
        }
        // 緑→青
        else if (t < 0.75) {
            green = (int)(255 * (1 - (t - 0.5) / 0.25));
            blue = (int)(255 * ((t - 0.5) / 0.25));
        }
        // 青→紫
        else if (t < 0.875) {
            blue = 255;
            red = (int)(255 * ((t - 0.75) / 0.125));  // 紫に向かって赤を増やす
        }
        // 紫→ピンク
        else {
            red = 255;
            green = (int)(255 * (1 - (t - 0.875) / 0.125));  // ピンクに向かって緑を減らす
            blue = 255;
        }

        int color = TColor::GetColor(red, green, blue);
        colors.push_back(color);
    }

    // 凡例の作成
    TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);  // 凡例の位置
    for (size_t i = 0; i < numFiles; ++i) {
        TH1F* hist = histograms[i];
        hist->SetLineColor(colors[i]);
        hist->SetLineWidth(2);
        if (i == 0) {
            hist->SetMaximum(maxY * 1.1);
            hist->Draw("");
            hist->SetTitle("22Na huruno2;ADC Sum[V];Counts");
        } else {
            hist->Draw("SAME");
        }

        legend->AddEntry(hist, fileNames[i], "l");
    }

    // 凡例の表示
    legend->Draw();

    // キャンバスの更新
    c1->Update();
}
