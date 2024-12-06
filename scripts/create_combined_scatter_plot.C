#include <TGraph.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TText.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <TGraphErrors.h>

void create_combined_scatter_plot() {
    // ファイル名をリスト化
    std::vector<std::string> filenames = {
        "./output/20241128/inverted-RC7493/TEXTFILES/22Na_0-1_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", "./output/20241128/inverted-RC7493/TEXTFILES/22Na_1-2_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", 
        "./output/20241128/inverted-RC7493/TEXTFILES/22Na_2-3_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", "./output/20241128/inverted-RC7493/TEXTFILES/22Na_3-4_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", 
        "./output/20241128/inverted-RC7493/TEXTFILES/22Na_4-5_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", "./output/20241128/inverted-RC7493/TEXTFILES/22Na_5-6_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", 
        "./output/20241128/inverted-RC7493/TEXTFILES/22Na_6-7_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", "./output/20241128/inverted-RC7493/TEXTFILES/22Na_7-8_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", 
        "./output/20241128/inverted-RC7493/TEXTFILES/22Na_8-9_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", "./output/20241128/inverted-RC7493/TEXTFILES/22Na_9-10_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", 
        "./output/20241128/inverted-RC7493/TEXTFILES/22Na_10-11_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt", "./output/20241128/inverted-RC7493/TEXTFILES/22Na_11-12_cm_HV_1700_GSO_24_dot_dat_dot_root_mean_sigma.txt"
        // "./output/20241128/RC7493/a_0-1_cm_test.txt"       
    };

    // 散布図のデータを格納するベクター
    std::vector<double> x_vals, y_vals, sigma_vals;

    // 各ファイルに対して処理を実行
    for (size_t i = 0; i < filenames.size(); ++i) {
        // std::cout << filenames.size() << std::endl;

        // ファイルを開く
        std::ifstream file(filenames[i]);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filenames[i] << std::endl;
            continue;
        }

        // 横軸をファイル名から抽出
        std::string filename = filenames[i];
        size_t pos_start = filename.find('_') + 1; //　アンダーバーがそれまでに使われているとうまくいかない。数字をどう変えればいいのか？
        size_t pos_end = filename.find("cm", pos_start);
        std::string range = filename.substr(pos_start, pos_end - pos_start);
        std::stringstream range_ss(range);
        double x_left, x_right;
        char dash;
        range_ss >> x_left >> dash >> x_right;

        // 縦軸のデータを格納するベクター
        double y_val;
        double sigma;

        // ファイル内容を読み込み、縦軸のデータをベクターに追加
        while (file >> y_val >> sigma) {
            // x_vals.push_back(x_right-0.5);  // 横軸は範囲の右側の値を使用
            // y_vals.push_back(y_val/24.1701);    // 縦軸はファイルの数値を使用

            x_vals.push_back(x_right-0.5);
            y_vals.push_back(y_val);
            sigma_vals.push_back(sigma);
        }
        // std::cout << x_right << std::endl;
        // std::cout << sigma << std::endl;
    }

    // すべてのファイルのデータを1つのグラフにまとめる
    TGraph* graph = new TGraph(x_vals.size(), &x_vals[0], &y_vals[0]);
    // TGraphErrors* graph = new TGraphErrors(x_vals.size(), &x_vals[0], 0, &y_vals[0], &sigma_vals[0]);  //　誤差棒付けたかったけどsigmaが各ファイルの最後の値になって無理でした
    // TGraph* graph = new TGraph(x_vals.size(), &x_vals[0], &sigma_vals[0]);

    graph->SetMarkerSize(5.5);  // 点のサイズを1.5に設定
    // graph->SetTitle("compare mean where gamma hits");
    // graph->GetXaxis()->SetTitle("distance from pmt window [cm]");
    // graph->GetYaxis()->SetTitle("mean of gaussian prop charge of pulse [V]");
    // graph->SetTitle("compare mean where gamma hits (RC7493, GSO#24);distance from pmt window [cm];mean of gaussian prop charge of pulse [V]");
    graph->SetTitle("compare mean where gamma hits (RC7493, inverted GSO#24);distance from pmt window [cm];mean of gaussian prop charge of pulse [V]");

    // キャンバスに描画
    TCanvas* canvas = new TCanvas("canvas_combined", "Combined Scatter Plot", 800, 600);
    graph->Draw("AP");

    // キャンバスを保存
    // canvas->SaveAs("combined_scatter_plot_RC7493_inverted_GSO#24.png");
}