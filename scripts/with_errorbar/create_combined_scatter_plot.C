#include "TGraph.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TText.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <TGraphErrors.h>

void create_combined_scatter_plot() {
    // ファイル名をリスト化
    std::vector<std::string> filenames = {
        "./output/22Na_0-1cm_dot_sdat_dot_sroot_maen_sigma.txt", "./output/22Na_1-2cm_dot_sdat_dot_sroot_maen_sigma.txt", 
        "./output/22Na_2-3cm_dot_sdat_dot_sroot_maen_sigma.txt", "./output/22Na_3-4cm_dot_sdat_dot_sroot_maen_sigma.txt", 
        "./output/22Na_4-5cm_dot_sdat_dot_sroot_maen_sigma.txt", "./output/22Na_5-6cm_dot_sdat_dot_sroot_maen_sigma.txt", 
        "./output/22Na_6-7cm_dot_sdat_dot_sroot_maen_sigma.txt", "./output/22Na_7-8cm_dot_sdat_dot_sroot_maen_sigma.txt", 
        "./output/22Na_8-9cm_dot_sdat_dot_sroot_maen_sigma.txt", "./output/22Na_9-10cm_dot_sdat_dot_sroot_maen_sigma.txt", 
        "./output/22Na_10-11cm_dot_sdat_dot_sroot_maen_sigma.txt", "./output/22Na_11-12cm_dot_sdat_dot_sroot_maen_sigma.txt"
    };

    // 散布図のデータを格納するベクター
    std::vector<double> x_vals, y_vals, y_errors ,x_errors;

    // 各ファイルに対して処理を実行
    for (size_t i = 0; i < filenames.size(); ++i) {
        // ファイルを開く
        std::ifstream file(filenames[i]);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filenames[i] << std::endl;
            continue;
        }

        // 横軸をファイル名から抽出
        std::string filename = filenames[i];
        size_t pos_start = filename.find('_') + 1;
        size_t pos_end = filename.find("cm", pos_start);
        std::string range = filename.substr(pos_start, pos_end - pos_start);
        std::stringstream range_ss(range);
        double x_left, x_right;
        char dash;
        range_ss >> x_left >> dash >> x_right;

        // 縦軸のデータを格納するベクター
        double y_val, y_error;
        // double scaling_factor = 24.1701;
        // double scaling_factor = 9.6779;
        // double scaling_factor = 3.52209;
        double scaling_factor = 1.0;

        // ファイル内容を読み込み、縦軸のデータをベクターに追加
        while (file >> y_val >> y_error) {
            x_vals.push_back(x_right-0.5);  // 横軸は範囲の右側の値を使用
            y_val = y_val/scaling_factor;
            y_error = y_error/scaling_factor;
            y_vals.push_back(y_val);    // 縦軸はファイルの数値を使用
            y_errors.push_back(y_error);
            x_errors.push_back(0);
        }
    }

    // すべてのファイルのデータを1つのグラフにまとめる
    TGraphErrors* graph = new TGraphErrors(x_vals.size(), &x_vals[0], &y_vals[0], &x_errors[0], &y_errors[0]);
    graph->SetMarkerSize(5.5);  // 点のサイズを1.5に設定
    graph->SetTitle("compare mean where gamma hits");
    // graph->GetXaxis()->SetTitle("distance from pmt window (cm)");
    // graph->GetYaxis()->SetTitle("mean of gaussian prop charge of pulse [V]");

    // キャンバスに描画
    TCanvas* canvas = new TCanvas("canvas_combined", "Combined Scatter Plot", 800, 600);
    graph->Draw("AP");

    // キャンバスを保存
    canvas->SaveAs("combined_scatter_plot.png");
}