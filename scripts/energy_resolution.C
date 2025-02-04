

// Eを較正後になおすためのデータ読み込み
void Load_EnergycalbData(TString key, Double_t p0[2][4], Double_t p0e[2][4], Double_t p1[2][4], Double_t p1e[2][4]){
    Double_t p0_buf, p1_buf, p0e_buf, p1e_buf;
    TString calb_data_filepath = Form("./cfg/%s/data.txt", key.Data());
    std::ifstream ifs(calb_data_filepath);
    Int_t line_index = 0;
    while(ifs >> p0_buf >> p0e_buf >> p1_buf >> p1e_buf){
        if(line_index == 8){
            break;
        }
        if(line_index < 4){
            p0[0][line_index] = p0_buf;
            p0e[0][line_index] = p0e_buf;
            p1[0][line_index] = p1_buf;
            p1e[0][line_index] = p1e_buf;
            std::cout << Form("\tiBoard : 0, iCh : %d || energy calibration data loaded.\n", line_index % 4) << std::endl;
            std::cout << Form("\t\t%lf %lf %lf %lf", p0_buf, p1_buf, p0e_buf, p1e_buf) << std::endl;
        }
        //else if((line_index-4) % 4 == line_index){
        else if(line_index < 8){
            p0[1][line_index-4] = p0_buf;
            p0e[1][line_index-4] = p0e_buf;
            p1[1][line_index-4] = p1_buf;
            p1e[1][line_index-4] = p1e_buf;
            std::cout << Form("\tiBoard : 1, iCh : %d || energy calibration data loaded.\n", line_index % 4) << std::endl;
            std::cout << Form("\t\t%lf %lf %lf %lf", p0_buf, p1_buf, p0e_buf, p1e_buf) << std::endl;
        }
        line_index++;
    }
    ifs.close();
}


// energy resolution plot
void energy_resolution(TString input_Folder = "./output/", TString key = "0204", Int_t iBoard = 0, Int_t iCh= 0){

    TString input_Filepath = Form("%s_calib.txt", input_Folder.Data());
    std::ifstream ifs(input_Filepath);
    double energy, ch, sigma_ch, sigma_gaus, sigma_gaus_energy;

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e);

    TCanvas* canvas = new TCanvas();
    TGraph* graph = new TGraph;
    int index_data = 0;
    Double_t R; // energy resolution
    Double_t max_R = 0;
    Double_t energy_calib;
    Double_t max_energy_calib = 0;

    while(ifs >> energy >> ch >> sigma_ch >> sigma_gaus){
        std::cout << "Energy: " << energy << ", Ch: " << ch << ", Sigma_ch: " << sigma_ch << ", Sigma_gaus: " << sigma_gaus << std::endl;
        
        sigma_gaus_energy = sigma_gaus * p1[iBoard][iCh]; // energy error in keV
        energy_calib = p1[iBoard][iCh] * ch + p0[iBoard][iCh];
        //energy_calib = p1_buf * ch + p0_buf;
        R = sigma_gaus_energy * 2 * sqrt(2 * log(2)) / energy_calib;
        
        graph->SetPoint(index_data, energy_calib, R * 100);

        std::cout << p0[iBoard][iCh] << " " << p1[iBoard][iCh] << std::endl;
        std::cout << R*100 << " " << energy_calib << std::endl;

        if(energy_calib > max_energy_calib){
            max_energy_calib = energy_calib;
        }
        if(R > max_R){
            max_R = R;
        }
        index_data++;
    }
    ifs.close();

    graph->SetTitle(Form("energy calibration from %s;Energy [keV];energy resolution [%%]", input_Filepath.Data()));
    
    
    graph->GetXaxis()->SetLimits(0.0, max_energy_calib*1.1);
    graph->GetYaxis()->SetRangeUser(0.0, max_R * 200);

    graph->SetMarkerStyle(20);
    graph->Draw("ap");

    // 1/√Eでフィッティング
    TF1 *fitFunc = new TF1("fitFunc", "[0]/sqrt(x)", 0, 1300);
    fitFunc->SetParameter(0, 300);  // 初期値
    graph->Fit(fitFunc);
    //std::cout << "Fitting parameter [0]/sqrt(x) : " << fitFunc->GetParameter(0) << std::endl;

    //graph->GetXaxis()->SetLimits(0, 1300);
    fitFunc->Draw("same");
    gStyle->SetOptFit();
    canvas->Update();

    // data.txtにフィットパラメータを追加
    // TString data_filepath = Form("./cfg/%s/data.txt", key.Data());
    // std::ifstream ifs_data(data_filepath);
    // std::ofstream ofs_data;
    // std::vector<std::string> lines;
    // std::string line;

    // data.txt の内容を読み込んで行ごとに保存
    // while (std::getline(ifs_data, line)) {
    //     lines.push_back(line);
    // }
    // ifs_data.close();

    // // iBoard と iCh に対応する行を更新
    // int line_to_update = iBoard * 4 + iCh; // iBoard と iCh の位置を計算
    // if (line_to_update < lines.size()) {
    //     // 既存の行にフィットパラメータを追加
    //     lines[line_to_update] += Form(" %f", fitFunc->GetParameter(0)); // フィットパラメータを追加
    // } else {
    //     std::cerr << "Error: Line to update is out of bounds." << std::endl;
    // }

    // // data.txt に更新内容を書き込む
    // ofs_data.open(data_filepath, std::ios::trunc); // 上書きモードで開く
    // for (const auto& l : lines) {
    //     //ofs_data << l << "\n";
    // }
    // ofs_data.close();

    // std::cout << "Fit parameter added to data.txt at line: " << line_to_update << std::endl;

    // 保存ファイル名を決定
    TString filename_figure = "energy_res.pdf";
    Int_t index = 1;
    while (gSystem->AccessPathName("./figure/" + filename_figure) == 0) {
        filename_figure = Form("energy_res_%d.pdf", index);
        index++;
    }
    canvas->SaveAs("./figure/" + filename_figure);
}
