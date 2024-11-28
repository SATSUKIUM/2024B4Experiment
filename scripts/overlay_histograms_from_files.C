#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <TStyle.h>
#include <filesystem>
#include <TString.h>
#include <iostream>
#include <algorithm>
#include <sstream>
// #include <utility> //pairなどを使うため
    
double extractDistance(const TString& filepath){
    TString filename = filepath(filepath.Last('/')+1, filepath.Length()-filepath.Last('/')); //ファイルパスからファイル名だけ抜き出した

    //以下では"22Na_-2_-1cm_dot_..."のような文字列から数字だけ抜き出す
    Int_t firstUnderscorePos = filename.Index('_');
    Int_t secondUnderscorePos = filename.Index('_',firstUnderscorePos+1);
    int centiMeterPos = filename.Index("cm",secondUnderscorePos+1);

    TString firstNumberStr = filename(firstUnderscorePos+1, secondUnderscorePos);
    TString secondNumberStr = filename(secondUnderscorePos+1, centiMeterPos);
    int firstNum = firstNumberStr.Atoi();
    int secondNum = secondNumberStr.Atoi();

    return (firstNum+secondNum)/2;
}
double extractDistance_ver2(const TString& filepath){
    TString filename = filepath(filepath.Last('/')+1, filepath.Length()-filepath.Last('/')); //ファイルパスからファイル名だけ抜き出した

    //以下では"22Na_-2_-1cm_dot_..."のような文字列から数字だけ抜き出す
    Int_t firstUnderscorePos = filename.Index('_');
    Int_t secondUnderscorePos = filename.Index('_',firstUnderscorePos+1);
    Int_t thirdUnderscorePos = filename.Index('_', secondUnderscorePos+1);

    TString firstNumberStr = filename(firstUnderscorePos+1, secondUnderscorePos);
    TString secondNumberStr = filename(secondUnderscorePos+1, thirdUnderscorePos);
    std::cout<< firstNumberStr << " || " << secondNumberStr << std::endl;
    int firstNum = firstNumberStr.Atoi();
    int secondNum = secondNumberStr.Atoi();

    return (firstNum+secondNum)/2;
}
void overlay_histograms_from_files(std::string folderpath = "./text_output/") {
    // キャンバスの作成
    TCanvas *c1 = new TCanvas("c1", "compare light yield of GSO scintillator by where gamma hit", 3200, 2400);

    // 凡例の作成
    TLegend *legend = new TLegend(0.7, 0.4, 0.9, 0.7); // 凡例の位置を設定
    legend->SetTextSize(0.02);


    std::vector<TH1F*> hists; //histのポインタを格納するベクトル
    std::vector<std::string> filepaths;
    int numFiles = 0;
    for(const auto& entry : std::filesystem::directory_iterator(folderpath)){
        filepaths.push_back(entry.path());
        numFiles++;
    }
    //filepathsの中身をdistanceの小さい順にsort
    std::sort(filepaths.begin(), filepaths.end(), [](const std::string& a, const std::string& b){
        return extractDistance(a) < extractDistance(b);
    });

    // データをテキストファイルから読み込んでヒストグラムにフィルする関数
    auto fillHistogramFromFile = [](TH1F* hist, std::string filename,TLegend* legend) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Failed to open file " << filename << std::endl;
            return;
        }
        double value;
        int count = 0;
        while (file >> value) {
            hist->Fill(value);
            count++;
        }
        file.close();
        if (count == 0) {
            std::cerr << "Warning: No data read from " << filename << std::endl;
        } else {
            std::cout << "Filled " << count << " entries from " << filename << std::endl;
        }
        // 凡例にヒストグラムを追加
        legend->AddEntry(hist, filename.c_str(), "lp");  // "l"は線の凡例を意味する
    };

    // 各ファイルからデータを読み込んでヒストグラムにフィル
    for(int i=0; i<numFiles; i++){
        TH1F *hist = new TH1F(Form("hist%d",i), "compare light yield of GSO scintillator by where gamma hits", 500, 0, 150);
        fillHistogramFromFile(hist, filepaths[i], legend);
        hists.push_back(hist);
    }

    // ヒストグラムのスタイル設定
    gStyle->SetPalette(kCool); // 波長順にカラーパレットを設定
    int j=0; //パレットのインデックス
    Int_t colorIndex = TColor::GetColorPalette(j);
    for(int i=0; i<numFiles; i++){
        hists[i]->SetLineColor(colorIndex);
        hists[i]->SetLineWidth(2);
        j+=255/numFiles;
        colorIndex = TColor::GetColorPalette(j);
    }
    
    // 最初のヒストグラムを描画
    hists[0]->SetXTitle("voltage sum prop charge of a pulse [V]");
    hists[0]->SetAxisRange(0,6000,"Y");
    hists[0]->SetAxisRange(0,90,"X");
    hists[0]->Draw();
    
    legend->Draw();

    // 残りのヒストグラムを重ねて描画
    for(int i=1; i<numFiles; i++){
        hists[i]->Draw("SAME");
    }

    // キャンバスを更新して表示
    c1->Update();
    c1->SaveAs("histgrams.png");
}