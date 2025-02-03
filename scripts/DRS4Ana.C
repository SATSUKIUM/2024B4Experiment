/*
DRS4Ana version 0.2
お求めやすい価格と手頃な量になって再登場
 */
/*======================================================================================================
 Name:           DRS4Ana.C
 Created by:     Akira Sato<sato@phys.sci.osaka-u.ac.jp>
 Date:           December 14th, 2022

 Purpose:        Example macro to analyze a root file created by binary2tree_sato3.C

How to use:

$ root
$ root[] .L DRS4Ana.C
$ root[] DRS4Ana a(<root file name>)
         ex) root[] DRS4Ana a("../data/test001.dat.root")
$ root[] a.PlotWaves()
$ root[] a.PlotChargeIntegral()

Please read the macro for the detail.
======================================================================================================*/

#define DRS4Ana_cxx
#include "DRS4Ana.h"
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

void DRS4Ana::PlotADCSum(Int_t iBoard, Int_t iCh)
{
    gStyle->SetOptStat(0);
    TCanvas *c_adcsum = new TCanvas("c_adcsum",
                                    Form("board%d,ch%d ADCsum", iBoard, iCh),
                                    800, 400);
    c_adcsum->Draw();

    if (fH1AdcSum != NULL)
    {
        delete fH1AdcSum;
    }
    fH1AdcSum = new TH1F("fH1AdcSum",
                         Form("board%d,ch%d ADCsum", iBoard, iCh),
                         1000, fADCsumXmin, fADCsumXmax);
    fH1AdcSum->SetXTitle("ADCsum");
    fH1AdcSum->SetYTitle("[count]");
    fChain->Draw(Form("-1.0*adcSum[%d][%d]>>fH1AdcSum", iBoard, iCh));

    // c_adcsum->Print(Form("%s_ch%d_adcSum.pdf", fRootFile.Data(), iCh));
}
TString DRS4Ana::Makedir_Date(){
    //YYYYMMDDのフォルダを作る関数。呼び出せば勝手にYYYYMMDDのフォルダができる。
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm);
    TString folderPath = TString::Format("./figure/%s", date);

    if(gSystem->AccessPathName(folderPath)){
        if(gSystem->mkdir(folderPath, true) != 0){
                std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
                return -1;
        }
    }
    return (folderPath);
}
Int_t DRS4Ana::IfFile_duplication(TString folderPath, TString &fileName){
    //例えば、"./figure/YYYYMMDD"というパスと、hoge.pdfを渡せば、そのディレクトリにhoge.pdfとhoge2.pdfが存在する場合に、渡した"hoge.pdf"を"hoge3.pdf"に変えてくれる関数
    Int_t index =1;
    while(gSystem->AccessPathName(folderPath + '/' + fileName) == 0){
        Int_t lastDotPos = fileName.Last('.');
        TString beforeDot = fileName(0, lastDotPos);
        TString afterDot = fileName(lastDotPos, fileName.Length());
        fileName = beforeDot + TString::Format("%d", index) + afterDot;
        index++;
        std::cout << Form("\tfilename : %s exists, rename...", fileName.Data()) << std::endl;
    }
    return index;
}

void DRS4Ana::Load_EnergycalbData(TString key, Double_t p0[2][4], Double_t p0e[2][4], Double_t p1[2][4], Double_t p1e[2][4]){
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
            std::cout << Form("\tiBoard : 0, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            std::cout << Form("\t\t%lf %lf %lf %lf", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        else if(line_index < 8){
            p0[1][line_index-4] = p0_buf;
            p0e[1][line_index-4] = p0e_buf;
            p1[1][line_index-4] = p1_buf;
            p1e[1][line_index-4] = p1e_buf;
            std::cout << Form("\tiBoard : 1, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            std::cout << Form("\t\t%lf %lf %lf %lf", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        
        line_index++;
        
    }
    ifs.close();
}

void DRS4Ana::PlotWave(Int_t iBoard, Int_t iCh, Int_t EventID)
{
    gStyle->SetOptStat(0);

    if (fH2Waveform != NULL)
    {
        delete fH2Waveform;
    }

    fH2Waveform = new TH2F("fH2Waveform",
                           Form("board%d,ch%d,Ev%d", iBoard, iCh, EventID),
                           10, fWaveformXmin, fWaveformXmax, 10, fWaveformYmin, fWaveformYmax);
    fH2Waveform->SetXTitle("Time [ns]");
    fH2Waveform->SetYTitle("Voltage [V]");
    fH2Waveform->Draw();
    fChain->Draw(Form("waveform[%d][%d]:%f*Iteration$", iBoard, iCh, fTimeBinWidthInNanoSec), "", "lsame", 1, EventID);
}

void DRS4Ana::PlotWaves(Int_t iBoard, Int_t iCh, Int_t EventID, Int_t nEvent)
{
    TCanvas *c_wave = new TCanvas("c_canvas", fRootFile.Data(), 800, 600);
    c_wave->Draw();

    for (Int_t i = 0; i < nEvent; i++)
    {
        PlotWave(iBoard, iCh, EventID + i);
        c_wave->WaitPrimitive();
    }
}

void DRS4Ana::SetWaveRangeX(Double_t min, Double_t max)
{
    fWaveformXmin = min;
    fWaveformXmax = max;
}

void DRS4Ana::SetWaveRangeY(Double_t min, Double_t max)
{
    fWaveformYmin = min;
    fWaveformYmax = max;
}

void DRS4Ana::SetPedestalTimeRange(Double_t min, Double_t max)
{
    fPedestalTmin = min;
    fPedestalTmax = max;
}

void DRS4Ana::SetChargeIntegralTimeRange(Double_t min, Double_t max)
{
    fChargeIntegralTmin = min;
    fChargeIntegralTmax = max;
}

Double_t DRS4Ana::GetMinVoltage(Int_t iBoard, Int_t iCh)
{
    Double_t minV = 100.0;
    for (Int_t i = 0; i < 1024; i++)
    {
        if (fWaveform[iBoard][iCh][i] < minV)
        {
            // printf("%d:%f\n",i,waveform[0][iCh][i]);
            minV = (Double_t)fWaveform[iBoard][iCh][i];
        }
    }
    return minV;
}

Double_t DRS4Ana::GetAbsMaxVoltage(Int_t iBoard, Int_t iCh)
{
    Double_t maxAbsV = 0.0;
    for (Int_t i = 0; i < 1024; i++)
    {
        if (fWaveform[iBoard][iCh][i] < -maxAbsV)
        {
            // printf("%d:%f\n",i,fWaveform[0][iCh][i]);
            maxAbsV = -(Double_t)fWaveform[iBoard][iCh][i];
        }
    }
    // cout << "Debug: GetMaxVoltage passed." << endl;
    return maxAbsV;
}

Double_t DRS4Ana::GetMaxVoltage(Int_t iBoard, Int_t iCh)
{
    Double_t maxV = -100.0;
    for (Int_t i = 0; i < 1024; i++)
    {
        if (fWaveform[iBoard][iCh][i] > maxV)
        {
            // printf("%d:%f\n",i,waveform[0][iCh][i]);
            maxV = (Double_t)fWaveform[iBoard][iCh][i];
        }
    }
    return maxV;
}

Double_t DRS4Ana::GetPedestal(Int_t iBoard, Int_t iCh, Double_t Vcut)
{
    if (fSignalPolarity == 1)
    {
        if (GetMaxVoltage(iBoard, iCh) <= Vcut)
        {
            return -9999.9;
        }
    }
    else
    {
        if (GetMinVoltage(iBoard, iCh) >= Vcut)
        {
            return -9999.9;
        }
    }

    Double_t pedestalV = 0.0;
    Double_t counter = 0;
    for (Int_t i = 0; i < 1024; i++)
    {
        if (fTime[iBoard][iCh][i] >= fPedestalTmin && fTime[iBoard][iCh][i] <= fPedestalTmax)
        {
            counter++;
            pedestalV += fWaveform[iBoard][iCh][i];
        }
    }
    // std::cout << pedestalV/counter << std::endl;
    // return 0;
    return pedestalV / counter;
}

Double_t DRS4Ana::GetPedestalMean(Int_t iBoard, Int_t iCh, Double_t Vcut)
{
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;
    Double_t pedMean = 0.0;
    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        Double_t ped = GetPedestal(iBoard, iCh, Vcut);
        if (ped > -9999.9)
        {
            counter++;
            pedMean += ped;
        }
    }
    return pedMean / counter;
}

Double_t DRS4Ana::PlotPedestalMean(Int_t iBoard, Int_t iCh, Double_t Vcut)
{
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;
    Double_t pedMean = 0.0;

    if (fH1Pedestal != NULL)
    {
        delete fH1Pedestal;
    }
    fH1Pedestal = new TH1F("fH1Pedestal", Form("%s:ch%d Pedestal", fRootFile.Data(), iCh),
                           1000, -0.01, 0.01);
    fH1Pedestal->SetXTitle("Voltage [V]");
    fH1Pedestal->SetYTitle("[counts]");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        Double_t ped = GetPedestal(iBoard, iCh, Vcut);
        if (ped > -9999.9)
        {
            counter++;
            pedMean += ped;
            fH1Pedestal->Fill(ped);
        }
    }
    fH1Pedestal->Draw();
    return pedMean / counter;
}

Double_t DRS4Ana::GetChargeIntegral(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t TcutMin = 0, Double_t TcutMax = 1000)
{
    if (fSignalPolarity == 1)
    {
        if (GetMaxVoltage(iBoard, iCh) <= Vcut)
        {
            return -9999.9;
        }
    }
    else
    {
        if (GetMinVoltage(iBoard, iCh) >= Vcut)
        {
            return -9999.9;
        }
    }

    Double_t pedestal = GetPedestal(iBoard, iCh, Vcut);

    Double_t charge = 0.0;
    for (Int_t i = 0; i < 1024; i++)
    {
        if (fTime[iBoard][iCh][i] >= TcutMin && fTime[iBoard][iCh][i] <= TcutMax)
        {
            charge += fWaveform[iBoard][iCh][i] - pedestal;
            // std::cout << fTriggerCell << std::endl;
            // std::cout << pedestal << " || " << fWaveform[iBoard][iCh][i] << std::endl;
            
        }
    }
    return charge;
}


Double_t DRS4Ana::PlotChargeIntegral(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax)
{
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    if (fH1ChargeIntegral != NULL)
    {
        delete fH1ChargeIntegral;
    }
    TCanvas *c1 = new TCanvas("c1", "Canvas", 800, 600);
    fH1ChargeIntegral = new TH1F("fH1ChargeIntegral", Form("%s:ch%d Charge Integral [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax),
                                 500, xmin, xmax);
    // fH1ChargeIntegral->SetXTitle("energy deposit [keV]");//for charge
    fH1ChargeIntegral->SetXTitle("voltage sum [V]");//for voltage sum
    fH1ChargeIntegral->SetYTitle("[counts]");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        Double_t chargeIntegral = GetChargeIntegral(iBoard, iCh, Vcut);
        if (chargeIntegral > -9999.9)
        {
            counter++;
            // fH1ChargeIntegral->Fill(chargeIntegral);//元のコード
            //PMTのパルスは負極性だからマイナスを付けた
            fH1ChargeIntegral->Fill(1.0*(-chargeIntegral)+0); //sum voltage
            // fH1ChargeIntegral->Fill(52.926*(-chargeIntegral)+1.1751); //for PMT good for HV -1700 V
            //fH1ChargeIntegral->Fill(52.926*(-chargeIntegral)+1.1751); //for PMT alpha for HV -1500 V
            // fH1ChargeIntegral->Fill(17.41*(-chargeIntegral)-26.05); //for PMT for sato_NaI for -1300 V
            // fH1ChargeIntegral->Fill(10.76*(-chargeIntegral)-198.1); //for PMT for huruno_PMT_1 HV -1150 V
        }
    }
    fH1ChargeIntegral->Draw();
    TString filename = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'+1));
    TString filename_figure = Form("./figure/%s:ch%d_Charge_Integral_[%.1f,%.1f].pdf", filename.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax);
    c1->SaveAs(filename_figure);
    
    
    filename_figure = Form("./figure/%s:ch%d_Charge_Integral_[%.1f,%.1f].png", filename.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax);
    c1->SaveAs(filename_figure);
    return (Double_t)counter;
}



Double_t DRS4Ana::PlotMaxVoltage(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax)
{
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    if (fH1MaxVoltage != NULL)
    {
        delete fH1MaxVoltage;
    }
    fH1MaxVoltage = new TH1F("fH1MaxVoltage", Form("%s:ch%d Max voltage [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax), 500, xmin, xmax);
    fH1MaxVoltage->SetXTitle("max voltage");
    fH1MaxVoltage->SetYTitle("[counts]");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        Double_t maxVoltage = GetAbsMaxVoltage(iBoard, iCh);
        counter++;
        fH1MaxVoltage->Fill(maxVoltage);
    }
    // fH1ChargeIntegral->SetMinimum(0);
    // fH1ChargeIntegral->SetMaximum(800);
    fH1MaxVoltage->Draw();

    return (Double_t)counter;
}
Double_t DRS4Ana::Output_chargeintegral(Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax)
{
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TString rootFile = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/')); //.rootファイルのフルパスからファイル名だけを抜き出した
    rootFile.ReplaceAll(".", "_dot_"); //.dat.rootのドットを"dot"に変えた
    TString filename_output;
    filename_output= Form("./text_output/%s_ch%d_Charge_Integral.txt", rootFile.Data(), iCh);
    std::ofstream ofs(filename_output);

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        Double_t chargeIntegral = GetChargeIntegral(iCh, Vcut);
        if (chargeIntegral > -9999.9)
        {
            counter++;
            ofs << -chargeIntegral << std::endl;
        }
    }
    ofs.close();
    return (Double_t)counter;
}

Double_t DRS4Ana::automated_peaksearch(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax, Int_t numPeaks, Double_t fitRange = 2.0)
{
    Int_t append_option = 1; //1 for not to overwrite the output.
    Int_t timecut_Option = 0;
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    if (fH1ChargeIntegral != NULL)
    {
        delete fH1ChargeIntegral;
    }

    Double_t Tmax_for_fH1CI = 0.0;
    if(timecut_Option == 1){
        Tmax_for_fH1CI = GetTriggerTiming(0, iCh, 0.1, -0.025) + 0;
        std::cout << "trigger timing || " << Tmax_for_fH1CI << std::endl;
    }

    TCanvas *c1 = new TCanvas("c1", "Canvas", 800, 600);
    fH1ChargeIntegral = new TH1F("fH1ChargeIntegral", Form("%s:ch%d Charge Integral [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax),
                                 500, xmin, xmax);
    // fH1ChargeIntegral->SetXTitle("energy deposit [keV]");//for charge
    fH1ChargeIntegral->SetXTitle("voltage sum [V]");//for voltage sum
    fH1ChargeIntegral->SetYTitle("[counts]");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);

        Double_t chargeIntegral;
        if(timecut_Option == 1){
            chargeIntegral = GetChargeIntegral(iBoard, iCh, Vcut, Tmax_for_fH1CI-10, Tmax_for_fH1CI+300); //電圧の和を取る時間の範囲を最後２つの変数に書いてる
            std::cout << "\tchargeIntegralTmin : " << Tmax_for_fH1CI-10 << std::endl << "\tchargeIntegralTmax : " << Tmax_for_fH1CI+300 << std::endl;
        }
        else{
            chargeIntegral = GetChargeIntegral(iBoard, iCh, Vcut, 0, 1024); //電圧の和を取る時間の範囲を最後２つの変数に書いてる
            //std::cout << "\tchargeIntegralTmin : " << fTime[iBoard][iCh][0] << std::endl << "\tchargeIntegralTmax : " << fTime[iBoard][iCh][1023] << std::endl;
        }
        
        if (chargeIntegral > -9999.9)
        {
            counter++;
            fH1ChargeIntegral->Fill(1.0*(-chargeIntegral)+0); //sum voltage
        }
    }
    gPad->SetGrid();
    fH1ChargeIntegral->Draw();


    TSpectrum *spectrum = new TSpectrum(numPeaks); //numPeaksは実際に見つけたいピークよりも多く設定しておくと良い
    spectrum->SetResolution(5); //
    Double_t spec_sigma = 5.0;
    Double_t spec_thr = 0.01;
    Int_t foundPeaks = spectrum->Search(fH1ChargeIntegral, spec_sigma, "", spec_thr); //要調整 .Search(a, b, c, d)のうち、bはどれくらいの太さ以上のピークを見つけたいか。cはオプション。dは最大のピークに対してどれくらいの大きさのピークまで探すかを指している。0.1だと最大のピークの10%の高さのピークまで探す。
    Double_t* peakPositions = spectrum->GetPositionX();

    std::vector<TF1*> fits; //"gaus"フィッティングを複数格納するベクトル
    std::vector<Double_t> means;
    std::vector<Double_t> sigmas_mean;
    std::vector<Double_t> sigmas_gaus;
    std::vector<TFitResultPtr> fitresults;
    for(int i=0; i<foundPeaks; ++i){
        TF1* gaussian = new TF1(Form("gaussian_%d",i), "gaus", peakPositions[i]-fitRange, peakPositions[i]+fitRange); //要調整。特に範囲
        gaussian->SetParameters(fH1ChargeIntegral->GetBinContent(fH1ChargeIntegral->FindBin(peakPositions[i]), peakPositions[i], 1.0));
        TFitResultPtr fit_result = fH1ChargeIntegral->Fit(gaussian, "RS+"); //オプションは好きに。TFitResultPtrはフィッティングの結果を保持する型。あとでフィッティングの可否判定に使う。
        std::cout << "debug" << std::endl;
        Int_t checking = fit_result->Status();
        if(checking != 0){}
        else{
            fits.push_back(gaussian);
            means.push_back(gaussian->GetParameter(1));
            // sigmas.push_back((gaussian->GetParameter(2))/sqrt(2*M_PI*(gaussian->GetParameter(0))*(gaussian->GetParameter(2))));//σ/√N
            sigmas_mean.push_back(gaussian->GetParError(1));//σ_mean
            sigmas_gaus.push_back(gaussian->GetParameter(2));//σ
        }
    }
    c1->Update();

    
    
    TString filename_figure;
    TString rootFile = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/')); //.rootファイルのフルパスからファイル名だけを抜き出した
    rootFile.ReplaceAll(".", "_dot_"); //.dat.rootのドットを"dot"に変えた

    std::ofstream ofs;
    if(append_option == 1){
        ofs.open("./output/automated_peaksearch_data.txt", std::ios::app);
    }
    else{
        ofs.open(Form("./output/%s_data.txt",rootFile.Data()));
    }
    
    auto mean_temp = means.begin();
    auto sigma_mean_temp = sigmas_mean.begin();
    auto sigma_gaus_temp = sigmas_gaus.begin();

    if(append_option == 1){
        ofs << std::endl << "=========================================" << std::endl << ".rootfile || filepath : " << fRootFile.Data() << std::endl;
        auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
        std::tm local_tm = *std::localtime(&now_c);

        ofs << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    }
    ofs << "means, sigmas of means, sigmas of gaussian" << std::endl << std::endl;
    while(mean_temp != means.end() && sigma_mean_temp != sigmas_mean.end() && sigma_gaus_temp != sigmas_gaus.end()){
        ofs << *mean_temp << " " << *sigma_mean_temp << " " << *sigma_gaus_temp << std::endl;
        ++mean_temp;
        ++sigma_mean_temp;
        ++sigma_gaus_temp;
    }
    ofs << std::endl << "numPeak : " << numPeaks << std::endl; // ピークの数
    ofs << "spec_sigma : " << spec_sigma << std::endl; // ピークの太さ
    ofs << "spec_thr : " << spec_thr << std::endl; // 最大ピークに対する高さの割合
    ofs << "fitrange : " << fitRange << std::endl; // ピーク中心からの範囲
    ofs.close();
    

    // 1. 日付を取得
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm); // "YYYYMMDD"形式で日付を取得

    // 2. フォルダパスを作成
    TString folderPath = TString::Format("./figure/%s", date);

    // 3. フォルダが存在しない場合は作成
    if (gSystem->AccessPathName(folderPath)) {
        if (gSystem->mkdir(folderPath, true) != 0) {
            std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
            return -1;
        }
    }

    filename_figure = Form("%s:ch%d_automated_peaksearch.pdf", rootFile.Data(), iCh);

    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName(folderPath + '/' + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("%s:ch%d_automated_peaksearch_%d.pdf", rootFile.Data(), iCh, index);
        index++;
    }

    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure.Data()));

    return (Double_t)counter;
}

void DRS4Ana::Output_EventTime(Int_t iCh)
{
    int counter = 1;
    Long64_t nentries = fChain->GetEntriesFast();

    for(Long64_t jentry = 0; jentry < nentries; jentry++){
        fChain->GetEntry(jentry);
        std::cout << "counter : " << counter << std::endl;
        std::cout << "in sec : " << fEventTimeInSec << std::endl;
        std::cout << "in nanosec : " << fEventTimeInNanoSec << std::endl << std::endl;
        counter++;
    }
}
Double_t DRS4Ana::PlotTriggerRate(Int_t iCh = 0){
    Long64_t nentries = fChain->GetEntriesFast();
    std::cout << "nentries: " << nentries << std::endl;

    Long64_t counter = 0;
    if(fH1TriggerRate != NULL){
        delete fH1TriggerRate;
    }

    //DAQの開始時刻と終了時刻の差をとる。
    fChain->GetEntry(0);
    Double32_t eventTime_begin = fEventTimeInSec + fEventTimeInNanoSec*10e-9; //time when started log
    Int_t eventTime_begin_InSec = fEventTimeInSec;
    fChain->GetEntry(nentries-1);
    Double32_t eventTime_end = fEventTimeInSec + fEventTimeInNanoSec*10e-9; //time when ended log
    Int_t eventTime_end_InSec = fEventTimeInSec;

    Int_t howLong_DAQ_spent = eventTime_end_InSec - eventTime_begin_InSec;
    std::cout << "how long DAQ spent: " << howLong_DAQ_spent << std::endl;
    // Double_t timeBin = howLong_DAQ_spent/10.0;


    // fH1TriggerRate = new TH1F("fH1TriggerRate", Form("%s:ch%d_Trigger_Rate", fRootFile.Data(), iCh), static_cast<Int_t>(timeBin), eventTime_begin, eventTime_end);

    //秒数を60で割って、60sあたりのトリガー数を入れたい
    fH1TriggerRate = new TH1F("fH1TriggerRate", Form("%s:ch%d_Trigger_Rate", fRootFile.Data(), iCh), howLong_DAQ_spent/60.0, 0, howLong_DAQ_spent);
    fH1TriggerRate->SetXTitle("time [s]");
    fH1TriggerRate->SetYTitle("[counts]/1min");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        fH1TriggerRate->Fill(-eventTime_begin_InSec+fEventTimeInSec+fEventTimeInNanoSec*10e-9);
        counter++;
    }
    fH1TriggerRate->Draw();
    return(counter);
}

Double_t DRS4Ana::Overlay_PlotWaves(Int_t iBoard=0, Int_t iCh=0){
    Long64_t nentries = fChain->GetEntriesFast();
    std::cout << "nentries: " << nentries << std::endl;

    gStyle->SetOptStat(0);
    
    if(fH2Overlay_Waves != NULL){
        delete fH2Overlay_Waves;
    }
    fH2Overlay_Waves = new TH2F();

    //ビンなどは適宜変える
    fChain->Draw(Form("waveform[%d][%d]:time[%d][%d]>>fH2Overlay_Waves(500, 0, %f, 500, -0.55, 0.05)",iBoard, iCh, iBoard, iCh, fTime[iBoard][iCh][1023]), "", "colz", nentries, 0); 
    //Draw(expression, selection, option, nentries, nfirstentry)

    TH2F* hist = (TH2F*)gROOT->FindObject("fH2Overlay_Waves");
    if(hist){
        hist->SetXTitle("Time (ns)");
        hist->GetXaxis()->SetRange(fTime[iBoard][iCh][1023]/1024.0, fTime[iBoard][iCh][1023]);
        hist->SetYTitle("Waveform (V)");
        hist->SetTitle(Form("fH2Overlay_Waves:%s", fRootFile.Data()));
    }
    gPad->SetLogz();
    gStyle->SetOptStat(0);
    Long64_t counter = 0;
    return counter;
}

void DRS4Ana::DEBUG_timebin(Int_t iBoard = 0, Int_t iCh = 0){
    std::cout << "iCh : " << iCh << std::endl;
    // Int_t nentries = fChain->GetEntriesFast();
    Int_t nentries = 5;

    for(Int_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);
        std::cout << "fTime[0][iCh][0] : " << fTime[iBoard][iCh][0] << std::endl;
        std::cout << "fTime[0][iCh][1] : " << fTime[iBoard][iCh][1] << std::endl;
        std::cout << "difference of these times in nanosec: " << fTime[iBoard][iCh][1] -fTime[iBoard][iCh][0] << std::endl;
        Double_t average_TimeBin = 0;
        Double_t max_TimeBin = 0;
        Int_t iCell_maxTimeBin, iCell_minTimeBin;
        Double_t min_TimeBin = 2;
        Double_t timeBin_buf;
        for(Int_t iCell=0; iCell<1023; iCell++){
            timeBin_buf = fTime[iBoard][iCh][(iCell+1) % 1024] - fTime[iBoard][iCh][iCell];
            average_TimeBin += timeBin_buf;
            if(timeBin_buf > max_TimeBin){
                max_TimeBin = timeBin_buf;
                iCell_maxTimeBin = iCell;
            }
            if(timeBin_buf < min_TimeBin){
                min_TimeBin = timeBin_buf;
                iCell_minTimeBin = iCell;
            }
        }
        average_TimeBin = average_TimeBin/1024;
        std::cout << "max_TimeBin : " << max_TimeBin << " || cell : " << iCell_maxTimeBin << std::endl;
        std::cout << "min_TimeBin : " << min_TimeBin << " || cell : " << iCell_minTimeBin << std::endl;
        std::cout << "distane_btwn_max_TimeBin_and_min_TimeBin : " << iCell_maxTimeBin - iCell_minTimeBin << std::endl;
        std::cout << "fTriggerCell : " << fTriggerCell[iBoard] << std::endl;
        std::cout << "average_TimeBin : " << average_TimeBin << " || end_TimeBin/1024 : " << fTime[iBoard][iCh][1023]/1024.0 << std::endl <<std::endl;
    }
}
void DRS4Ana::Plot_wave_two_boards(Int_t iCh_master = 0, Int_t iCh_slave = 0, Int_t EventID = 0, Int_t canvas_index){
    gStyle->SetOptStat(0);
    gPad->SetGrid();

    if(canvas_index == 1){
        if(fH2Waveform0 != NULL){
            delete fH2Waveform0;
            fH2Waveform0 = new TH2F("fH2Waveform", Form("waveform: board #%d || EventID %d",canvas_index-1, EventID), 10, 0, 1024, 10 ,-0.55, 0.05);
            fH2Waveform0->SetXTitle("Time [ns]");
            fH2Waveform0->SetYTitle("Voltage [V]");
            fH2Waveform0->Draw();
    
            fChain->Draw(Form("waveform[0][%d]:%f*Iteration$", iCh_master, fTime[0][iCh_master][1023]/1024.0), "", "same", 1, EventID); 
        }
    }
    if (canvas_index == 2)
    {
        if(fH2Waveform1 != NULL){
            delete fH2Waveform1;
            fH2Waveform1 = new TH2F("fH2Waveform", Form("waveform: board #%d || EventID %d",canvas_index-1, EventID), 10, 0, 1024, 10 ,-0.55, 0.05);
            fH2Waveform1->SetXTitle("Time [ns]");
            fH2Waveform1->SetYTitle("Voltage [V]");
            fH2Waveform1->Draw();
    
            fChain->Draw(Form("waveform[0][%d]:%f*Iteration$", iCh_slave, fTime[0][iCh_master][1023]/1024.0), "", "same", 1, EventID);
        }
    }
}

void DRS4Ana::Plot_waves_two_boards(Int_t event_num_initial = 0, Int_t iCh_master = 0, Int_t iCh_slave = 0){
    Int_t nentries = fChain->GetEntriesFast();

    TCanvas *c1 = new TCanvas("c1", "Waveform : master and slave board", 700, 500);
    c1->Divide(2,1);
    c1->Draw();
    fH2Waveform0 = new TH2F;
    fH2Waveform1 = new TH2F;

    for(Int_t i=event_num_initial; i<nentries; i++){
        
        for(Int_t canvas_index=1; canvas_index<=2; canvas_index++){
            c1->cd(canvas_index);
            Plot_wave_two_boards(iCh_master, iCh_slave, i, canvas_index);
            c1->Update();
        }
        
        c1->WaitPrimitive(); 
    }
}

Double_t DRS4Ana::Overlay_PlotWaves_discri(Int_t iCh = 0, Double_t threshold = 0.10){
    Long64_t nentries = fChain->GetEntriesFast();
    std::cout << "nentries: " << nentries << std::endl;

    gStyle->SetOptStat(0);
    
    if(fH2Overlay_Waves != NULL){
        delete fH2Overlay_Waves;
    }
    fH2Overlay_Waves = new TH2F();

    //ビンなどは適宜変える
    fChain->Draw(Form("waveform[0][0]:%f*Iteration$>>fH2Overlay_Waves(300, 0, %f, 300, -0.55, 0.05)",fTimeBinWidthInNanoSec, fWaveformXmax), "", "colz", nentries, 0); //Draw(expression, selection, option, nentries, nfirstentry)

    TH2F* hist = (TH2F*)gROOT->FindObject("fH2Overlay_Waves");
    if(hist){
        hist->SetXTitle("Time (ns)");
        hist->SetYTitle("Waveform (V)");
        hist->SetTitle(Form("fH2Overlay_Waves:%s", fRootFile.Data()));
    }
    for(Int_t xBin = 1; xBin <= hist->GetNbinsX(); ++xBin){
        for(Int_t yBin = 1; yBin <= hist->GetNbinsY(); ++yBin){
            Double_t binContent = hist->GetBinContent(xBin, yBin);
            if(binContent <= nentries*threshold){
                hist->SetBinContent(xBin, yBin, 0);
            }
        }
    }
    gStyle->SetPalette(kRainBow);
    gPad->SetLogz();
    Long64_t counter = 0;
    return counter;
}
Double_t DRS4Ana::GetTriggerTiming(Int_t iBoard = 0, Int_t iCh = 0, Double_t threshold = 0.10, Double_t trigger_voltage = -0.025){
    Long64_t nentries = fChain->GetEntriesFast();
    // std::cout << "nentries: " << nentries << std::endl;
    
    if(fH2Filtered_Overlay_Waves != NULL){
        delete fH2Filtered_Overlay_Waves;
    }
    Int_t binsX,binsY;
    fH2Filtered_Overlay_Waves = new TH2F(Form("fH2Overlay_Waves:%s", fRootFile.Data()), "title", 200, fWaveformXmin, fWaveformXmax, 200, fWaveformYmin, fWaveformYmax);

    for(Int_t i=0; i<nentries; i++){
        for(Int_t iCell=0; iCell<1024; iCell++){
            fH2Filtered_Overlay_Waves->Fill(fTime[iBoard][iCh][iCell], fWaveform[iBoard][iCh][iCell]);
        }
    }

    Double_t binContent;
    Int_t flag_search_done = 0;
    for(Int_t xBin = 1; xBin <= fH2Filtered_Overlay_Waves->GetNbinsX(); ++xBin){
        for(Int_t yBin = fH2Filtered_Overlay_Waves->GetNbinsY(); yBin > 0; yBin += -1){
            if(fH2Filtered_Overlay_Waves->GetBinContent(xBin, yBin) > nentries*threshold){
                if(fH2Filtered_Overlay_Waves->GetYaxis()->GetBinCenter(yBin) < trigger_voltage){
                    flag_search_done = 1;
                    Double_t v_return = fH2Filtered_Overlay_Waves->GetXaxis()->GetBinCenter(xBin);
                    return(v_return);
                    break;
                }
            }
        }
    }
    if(flag_search_done != 1){
        return(-1.0); //trigger time was not found in the loop
    }
    else{
        return(0);
    }
}

Double_t DRS4Ana::Output_MaxVoltage(Int_t how_many_boards = 1, Int_t iCh = 0){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;
    Double_t avg_MaxVoltage = 0;
    Double_t avg_MinVoltage = 0;
    Double_t MaxVoltage_temp;
    Double_t MinVoltage_temp;
    Double_t buf_Cell;
    TString name_fRootFile = gSystem->BaseName(fRootFile);
    TString cout_buf[how_many_boards];

    for(Int_t iBoard=0; iBoard<how_many_boards; iBoard++){
        avg_MaxVoltage = 0;
        avg_MinVoltage = 0;
        MaxVoltage_temp = -0.5;
        MinVoltage_temp = 0.5;

        for(Long64_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);

        for(Int_t iCell=0; iCell<1024; iCell++){
            buf_Cell = fWaveform[iBoard][iCh][iCell];
            if(buf_Cell<MinVoltage_temp){
                MinVoltage_temp = buf_Cell;
            }
            if(buf_Cell>MaxVoltage_temp){
                MaxVoltage_temp = buf_Cell;
            }
        }
        avg_MinVoltage += MinVoltage_temp;
        avg_MaxVoltage += MaxVoltage_temp;
        }
        avg_MinVoltage = avg_MinVoltage/nentries;
        avg_MaxVoltage = avg_MaxVoltage/nentries;

        std::cout<< Form("iBoard : %d || MaxVoltage(%s): ", iBoard, name_fRootFile.Data()) << avg_MaxVoltage << std::endl;
        std::cout<< Form("iBoard : %d || MinVoltage(%s): ", iBoard, name_fRootFile.Data()) << avg_MinVoltage << std::endl;
        counter++;
        cout_buf[iBoard] = Form("%f %f %f",avg_MaxVoltage, avg_MinVoltage, avg_MaxVoltage-avg_MinVoltage);
    }
    for(int i=0; i<how_many_boards; i++){
        std::cout << cout_buf[i] << " ";
    }
    std::cout << std::endl;
    return counter;
}

Double_t DRS4Ana::Plot_2Dhist_energy_btwn_PMTs(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 600);
    canvas->Divide(3,1);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    TH1D *fH1EnergySpectra[2];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), 500, 0, 600);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), 500, 0, 600);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per 1.2 keV", x_iBoard, x_iCh, key_Crystal_x.Data()));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per 1.2 keV", y_iBoard, y_iCh, key_Crystal_y.Data()));

    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e);

    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    if(key_Crystal_x == "NaI"){
        adcSum_timerange_x = 600;
    }
    else if(key_Crystal_x == "GSO"){
        adcSum_timerange_x = 180;
    }
    else{
        printf("\t\nx axis || type of crystal is invalid\n");
    }
    if(key_Crystal_y == "NaI"){
        adcSum_timerange_y = 600;
    }
    else if(key_Crystal_y == "GSO"){
        adcSum_timerange_y = 180;
    }
    else{
        printf("\t\ny axis || type of crystal is invalid\n");
    }
    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        if(100 < DiscriTime_y && DiscriTime_y < 1400){
            x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
            y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

            x_energy = p0[x_iBoard][x_iCh] + p1[x_iBoard][x_iCh]*x_charge_buf;
            y_energy = p0[y_iBoard][y_iCh] + p1[y_iBoard][y_iCh]*y_charge_buf;

            fH2Energy_PMTs->Fill(x_energy, y_energy);
            fH1EnergySpectra[0]->Fill(x_energy);
            fH1EnergySpectra[1]->Fill(y_energy);
        }

        if(Entry % 500 == 0){
            printf("\tPoint plot : %d\n", Entry);
        }
        counter++;
    }
    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();
    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    fH1EnergySpectra[0]->Draw();
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    fH1EnergySpectra[1]->Draw();

    canvas->cd(1);
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kBlack);
    line->SetLineWidth(2);
    line->Draw("SAME");

    canvas->Update();

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    filename_figure += "_fH2Energy_PMTs.pdf";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure.Data()));
    
    return counter;
}

Double_t DRS4Ana::PlotEnergy(TString key = "0120", TString key_Crystal = "NaI", Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 20, Double_t xmin = 0, Double_t xmax = 600){
    /*
        エネルギー較正の式はかならずファイルから読み込むようにします。
        ファイルの形式は上の行から
        iBoard 0 iCh 0
        iB 0 iC 1
        iB 0 iC 2
        iB 0 iC 3
        iB 1 iC 0
        iB 1 iC 1
        iB 1 iC 2
        iB 1 iC 3
        とします。それぞれの行には4つ要素をスペース区切りで書きます。
        エネルギー較正の式をp0+p1*xとすると、行の要素は
        p0 Δp0 p1 Δp1 とします。
        9行目より後は読み込まれないようにしてあるので、メモ用紙にでも使ってください。
    */
   Int_t flag_SlaveOnly = 0;
    std::cout << Form("\n\tnumOfBoards : %d", fNumOfBoards) << std::endl;
    if(fNumOfBoards == 1){
        std::cout << Form("Board info\n\tmaster board : %d\n", fSerialNumber[0]) << std::endl;
        if(fSerialNumber[0] == 32814){
            flag_SlaveOnly = 1;
        }
    }
    else if(fNumOfBoards == 2){
        std::cout << Form("Boards info\n\tmaster board : %d\n\tslave board : %d", fSerialNumber[0], fSerialNumber[1]) << std::endl;
    }


    std::cout << "iBoard:" << " " << iBoard << std::endl;
    std::cout << "iCh:" << " " <<iCh << std::endl;
    std::cout << "Vcut:" << " " <<Vcut << std::endl;
    std::cout << "key_Crystal:" << " " <<key_Crystal << std::endl;
    std::cout << "xmin:" << " " <<xmin << std::endl;
    std::cout << "xmax:" << " " <<xmax << std::endl;


    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;
    Double_t timeCut_begin, timeCut_end;

    TCanvas *c1 = new TCanvas("c1", Form("%d:ch%d Plot Energy", iBoard, iCh), 1600, 1200);
    c1->Draw();
    //gStyle->SetOptStat(0);
    gPad->SetGrid();

    if (fH1ChargeIntegral != NULL)
    {
        delete fH1ChargeIntegral;
    }

    Int_t histDiv = 200;
    fH1ChargeIntegral = new TH1F("fH1ChargeIntegral", Form("%s || Board %d, CH %d", fRootFile.Data(), iBoard, iCh), histDiv, xmin, xmax);
    fH1ChargeIntegral->SetXTitle("Energy [keV]");
    fH1ChargeIntegral->SetYTitle(Form("counts per %f keV", (xmax-xmin)/histDiv));

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e);

    // for(Int_t ib=0; ib<2; ib++){
    //     for(Int_t ic=0; ic<4; ic++){
    //         printf("\t%f %f %f %f\n", p0[ib][ic], p0e[ib][ic], p1[ib][ic], p1e[ib][ic]);
    //     }
    // }
   
    Double_t discriTime;
    Double_t adcSum_timerange;
    if(key_Crystal == "NaI"){
        adcSum_timerange = 600;
    }
    else if(key_Crystal == "GSO"){
        adcSum_timerange = 180;
    }
    else{
        std::cout << "key is invalid" << std::endl;
    }
    Double_t energy_buf;

    for (Long64_t jentry = 0; jentry < nentries; jentry++){
        fChain->GetEntry(jentry);
        discriTime = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
        Double_t chargeIntegral = GetChargeIntegral(iBoard, iCh, Vcut, discriTime - 50, discriTime + adcSum_timerange);

        if (chargeIntegral > -9999.9)
        {
            energy_buf = p0[iBoard+flag_SlaveOnly][iCh] + p1[iBoard+flag_SlaveOnly][iCh]*(-chargeIntegral);
            counter++;
            fH1ChargeIntegral->Fill(energy_buf);
        }
    }

    
    fH1ChargeIntegral->Draw();

     TF1* gaussian = new TF1("gaussian", "gaus", 400, 600);
        // gauss1->SetParameters(
        //     gaussian_plus_linear->GetParameter(0), // 振幅
        //     gaussian_plus_linear->GetParameter(1), // 中心
        //     gaussian_plus_linear->GetParameter(2)  // 幅
        // );
        //gauss1->SetLineColor(kOrange+7);
        //gauss1->SetLineStyle(1);
        //gauss->Draw("LSAME");
        fH1ChargeIntegral -> Fit(gaussian, "R");
        gaussian -> Draw("same");

        c1->Update();
        gStyle->SetOptFit(1);

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    filename_figure += "_energy_spectrum.pdf";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure.Data()));

    return (Double_t)counter;
}

Double_t DRS4Ana::PlotSumEnergy(TString key = "0120", TString key_Crystal1 = "NaI", Int_t iBoard1, Int_t iCh1, TString key_Crystal2 = "NaI", Int_t iBoard2, Int_t iCh2, Double_t Vcut, Double_t xmin, Double_t xmax)
{
    gStyle->SetOptStat(1); // 統計ボックス表示の有無 1が表示 0が非表示

    // 前のキャンバスが存在する場合、削除する
    TCanvas* existingCanvas = (TCanvas*)gROOT->FindObject("c1");
    if (existingCanvas)
    {
        existingCanvas->Close(); // キャンバスを閉じる
        delete existingCanvas;  // メモリ解放
        existingCanvas = nullptr;
    }

    TCanvas *c1 = new TCanvas("c1", Form("%s:Board%dCh%d+Board%dCh%d SumEnergy", fRootFile.Data(), iBoard1, iCh1+1, iBoard2, iCh2+1), 800, 600);

    if (fH1Energy_PMTs != NULL)
    {
        delete fH1Energy_PMTs;
    }

    Int_t histDiv = 200;

    fH1Energy_PMTs = new TH1F("fH1Energy_PMTs", Form("%s:Board%dCh%d+Board%dCh%d SumEnergy", fRootFile.Data(), iBoard1+1, iCh1+1, iBoard2+1, iCh2+1), histDiv, xmin, xmax);
    fH1Energy_PMTs->SetXTitle("Sum of Energy [keV]");
    fH1Energy_PMTs->SetYTitle(Form("counts per %f keV", (xmax-xmin)/histDiv));

    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    //　エネルギーへの変換に必要なパラメータを取得
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e);

    Double_t discriTime1, discriTime2;
    Double_t adcSum_timerange1, adcSum_timerange2;

    //　結晶に応じた積分範囲を指定
    if(key_Crystal1 == "NaI"){
        adcSum_timerange1 = 600;
    }
    else if(key_Crystal1 == "GSO"){
        adcSum_timerange1 = 180;
    }
    else{
        std::cout << "key1 is invalid" << std::endl;
    }

    if(key_Crystal2 == "NaI"){
        adcSum_timerange2 = 600;
    }
    else if(key_Crystal2 == "GSO"){
        adcSum_timerange2 = 180;
    }
    else{
        std::cout << "key2 is invalid" << std::endl;
    }

    Double_t energy_buf1, energy_buf2;

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);

        discriTime1 = fTime[iBoard1][iCh1][fDiscriCell[iBoard1][iCh1]];
        discriTime2 = fTime[iBoard2][iCh2][fDiscriCell[iBoard2][iCh2]];

        // 各チャンネルの Charge Integral を取得
        Double_t chargeIntegral1 = GetChargeIntegral(iBoard1 , iCh1, Vcut, discriTime1 - 50, discriTime1 + adcSum_timerange1);
        Double_t chargeIntegral2 = GetChargeIntegral(iBoard2 , iCh2, Vcut, discriTime2 - 50, discriTime2 + adcSum_timerange2);

        // Charge Integralが有効な場合のみ足し合わせる
        if (chargeIntegral1 > -9999.9 && chargeIntegral2 > -9999.9)
        {
            

            // チャンネルに応じたエネルギーへ変換
            energy_buf1 = p0[iBoard1][iCh1] + p1[iBoard1][iCh1]*(-chargeIntegral1);
            energy_buf2 = p0[iBoard2][iCh2] + p1[iBoard2][iCh2]*(-chargeIntegral2);

            if (energy_buf1 < 450.0 && energy_buf2 > 100.0){
                Double_t sumEnergy = energy_buf1 + energy_buf2;
                fH1Energy_PMTs->Fill(sumEnergy);

                counter++;
            }
            
        }
    }

    fH1Energy_PMTs->Draw();

    

    // ピークに対するフィッティング
    // TF1 *fitFunc1 = new TF1("fitFunc1", "gaus", 500, 520); // 第2ピークに対する範囲
    // fH1SumChargeIntegral->Fit(fitFunc1, "R");
 

    // fH1SumChargeIntegral->Draw();
    // fitFunc1->Draw("same");

    TString name;
    name = Form("Energy_Board%dch%d+Board%dch%d.pdf",iBoard1+1, iCh1+1, iBoard2+1, iCh2+1);
    c1->SaveAs(name);


    return counter;
}

Double_t DRS4Ana::PlotWavesWithThreshold(Int_t iBoard, Int_t iCh)
{
    TCanvas *c_wave = new TCanvas("c_canvas", fRootFile.Data(), 800, 600);
    c_wave->Draw();

    Long64_t nentries = fChain->GetEntriesFast();  // イベントの総数を取得

    // 全てのイベントで最大電圧を確認
    for (Long64_t i = 0; i < nentries; i++)
    {
        // 最大電圧を取得
        Double_t maxVoltage = GetMaxVoltage(iBoard, iCh);
        std::cout << "maxVoltage: " << maxVoltage << std::endl;

        // 最大電圧が-10mVより大きい場合にのみ波形を描く
         if (maxVoltage < -0.010)  // -0.01V (即ち-10mV) より大きい場合
    {

       //イベントIDに基づいて波形を描画
            
        PlotWave(iBoard, iCh, i);  // イベントIDはiを使用
        
        TObject* obj = gPad->GetListOfPrimitives()->Last();
            if (obj) {
                // TGraphやTH1Fの場合のみ色を設定
                if (TGraph* graph = dynamic_cast<TGraph*>(obj)) {
                    graph->SetLineColor(kBlack + i);  // イベントごとに色を変更
                } else if (TH1F* hist = dynamic_cast<TH1F*>(obj)) {
                    hist->SetLineColor(kBlack + i);  // イベントごとに色を変更
                }
            }
        //c_wave->WaitPrimitive();
    }
    }

    // 最後に描画を更新
    c_wave->Update();
    Long64_t counter = 0;
    return counter;
    
}

Double_t DRS4Ana::automated_peaksearch_SCA_mode(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax, Int_t numPeaks, Double_t fitRange = 2.0)
{
    Int_t append_option = 1; //1 for not to overwrite the output.
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    if (fH1MaxVoltage != NULL)
    {
        delete fH1MaxVoltage;
    }

    TCanvas *c1 = new TCanvas("c1", "Canvas", 800, 600);
    fH1MaxVoltage = new TH1F("fH1MaxVoltage", Form("%s:ch%d SCA spectrum [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax),500, xmin, xmax);

    gPad->SetGrid();
    fH1MaxVoltage->SetXTitle("Max voltage [V]");//for voltage sum
    fH1MaxVoltage->SetYTitle("[counts]/bin");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        Double_t pulseHight = 0.0;
        pulseHight = GetAbsMaxVoltage(iBoard, iCh);
        if( pulseHight > 0){
            counter++;
            fH1MaxVoltage->Fill(pulseHight);
        }
    }
    
    fH1MaxVoltage->Draw();


    TSpectrum *spectrum = new TSpectrum(numPeaks); //numPeaksは実際に見つけたいピークよりも多く設定しておくと良い
    spectrum->SetResolution(5); //
    Double_t spec_sigma = 0.25;
    Double_t spec_thr = 0.01;
    Int_t foundPeaks = spectrum->Search(fH1MaxVoltage, spec_sigma, "", spec_thr); //要調整 .Search(a, b, c, d)のうち、bはどれくらいの太さ以上のピークを見つけたいか。cはオプション。dは最大のピークに対してどれくらいの大きさのピークまで探すかを指している。0.1だと最大のピークの10%の高さのピークまで探す。
    Double_t* peakPositions = spectrum->GetPositionX();

    std::vector<TF1*> fits; //"gaus"フィッティングを複数格納するベクトル
    std::vector<Double_t> means;
    std::vector<Double_t> sigmas_mean;
    std::vector<Double_t> sigmas_gaus;
    std::vector<TFitResultPtr> fitresults;

    for(int i=0; i<foundPeaks; ++i){
        TF1* gaussian = new TF1(Form("gaussian_%d",i), "gaus", peakPositions[i]-fitRange, peakPositions[i]+fitRange); //要調整。特に範囲
        gaussian->SetParameters(fH1MaxVoltage->GetBinContent(fH1MaxVoltage->FindBin(peakPositions[i]), peakPositions[i], 1.0));
        TFitResultPtr fit_result = fH1MaxVoltage->Fit(gaussian, "RS+"); //オプションは好きに。TFitResultPtrはフィッティングの結果を保持する型。あとでフィッティングの可否判定に使う。
        std::cout << "debug" << std::endl;
        Int_t checking = fit_result->Status();
        if(checking != 0){}
        else{
            fits.push_back(gaussian);
            means.push_back(gaussian->GetParameter(1));
            // sigmas.push_back((gaussian->GetParameter(2))/sqrt(2*M_PI*(gaussian->GetParameter(0))*(gaussian->GetParameter(2))));//σ/√N
            sigmas_mean.push_back(gaussian->GetParError(1));//σ_mean
            sigmas_gaus.push_back(gaussian->GetParameter(2));//σ
        }
    }


    c1->Update();

    
    
    TString filename_figure;
    TString rootFile = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/')); //.rootファイルのフルパスからファイル名だけを抜き出した
    rootFile.ReplaceAll(".", "_dot_"); //.dat.rootのドットを"dot"に変えた

    std::ofstream ofs;
    if(append_option == 1){
        ofs.open("./output/SCA_peaksearch_data.txt", std::ios::app);
    }
    else{
        ofs.open(Form("./output/%s_data.txt",rootFile.Data()));
    }
    
    auto mean_temp = means.begin();
    auto sigma_mean_temp = sigmas_mean.begin();
    auto sigma_gaus_temp = sigmas_gaus.begin();

    if(append_option == 1){
        ofs << std::endl << "=========================================" << std::endl << ".rootfile || filepath : " << fRootFile.Data() << std::endl;
        auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
        std::tm local_tm = *std::localtime(&now_c);

        ofs << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    }
    ofs << "means, sigmas of means, sigmas of gaussian" << std::endl << std::endl;
    while(mean_temp != means.end() && sigma_mean_temp != sigmas_mean.end() && sigma_gaus_temp != sigmas_gaus.end()){
        ofs << *mean_temp << " " << *sigma_mean_temp << " " << *sigma_gaus_temp << std::endl;
        ++mean_temp;
        ++sigma_mean_temp;
        ++sigma_gaus_temp;
    }
    ofs << std::endl << "numPeak : " << numPeaks << std::endl; // ピークの数
    ofs << "spec_sigma : " << spec_sigma << std::endl; // ピークの太さ
    ofs << "spec_thr : " << spec_thr << std::endl; // 最大ピークに対する高さの割合
    ofs << "fitrange : " << fitRange << std::endl; // ピーク中心からの範囲
    ofs.close();
    

    // 1. 日付を取得
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm); // "YYYYMMDD"形式で日付を取得

    // 2. フォルダパスを作成
    TString folderPath = TString::Format("./figure/%s", date);

    // 3. フォルダが存在しない場合は作成
    if (gSystem->AccessPathName(folderPath)) {
        if (gSystem->mkdir(folderPath, true) != 0) {
            std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
            return -1;
        }
    }

    filename_figure = Form("%s:ch%d_SCA_peaksearch.pdf", rootFile.Data(), iCh);

    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName(folderPath + '/' + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("%s:ch%d_SCA_peaksearch_%d.pdf", rootFile.Data(), iCh, index);
        index++;
    }

    
    c1->SaveAs(folderPath + '/' + filename_figure);

    return (Double_t)counter;
}



Double_t DRS4Ana::time_divided_spectrum(Int_t divOfTime = 10){
    Long64_t nentries = fChain->GetEntriesFast();
    // Long64_t nentries = 10000;
    Long64_t counter = 0;
    Int_t numOfBoards = 1;

    TCanvas *canvas = new TCanvas("canvas", "title", 1600, 1200);
    canvas->Divide(2,numOfBoards*2);
    if(divOfTime>1){
        gStyle->SetPalette(kCool);
    }
    TH1D* fH1EnergySpectra[2][4][divOfTime];
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            for(Int_t iDiv=0; iDiv<divOfTime; iDiv++){
                fH1EnergySpectra[iBoard][iCh][iDiv] = new TH1D(Form("fH1EnergySpectra || iB : %d, iC : %d, iDiv : %d", iBoard, iCh, iDiv), Form("iB : %d, iC : %d, iDiv : %d", iBoard, iCh, iDiv), 100, 0, 600);
            }
            canvas->cd(iBoard*4+iCh+1);
            gPad->SetGrid();
        }
    }
    gPad->SetGrid();
    gStyle->SetOptStat(0);


    Double_t p0[2][4], p1[2][4];
    p0[0][0] = -19.46;
    p1[0][0] = 5.487;
    p0[0][1] = -42.98;
    p1[0][1] = 6.078;
    p0[0][2] = -24.38;
    p1[0][2] = 6.737;
    p0[0][3] = -10.61;
    p1[0][3] = 12.05;//ため息が出る汚さ

    Double_t p0_buf, p1_buf;
    Double_t chargeInt_buf;
    Int_t colorIndex_key, colorIndex;
    TLegend* legend[2][4];
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            legend[iBoard][iCh] = new TLegend(0.7, 0.5, 0.9, 0.9);
        }
    }

    for(Int_t iDiv=0; iDiv<divOfTime; iDiv++){
        for(Int_t Entry = iDiv*(nentries/divOfTime); Entry<(iDiv+1)*(nentries/divOfTime); Entry++){
            fChain->GetEntry(Entry);
            counter++;
            if(counter % 1000 == 0){
                std::cout << "\tcounter : " << counter << std::endl;
            }

            for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
                for(Int_t iCh=0; iCh<4; iCh++){
                    p0_buf = p0[iBoard][iCh];
                    p1_buf = p1[iBoard][iCh];

                    canvas->cd(iBoard*4+iCh+1);

                    if(iBoard == 0 && iCh == 3){
                        // chargeInt_buf = GetChargeIntegral(iBoard, iCh, 20, 300, 800);
                        chargeInt_buf = GetChargeIntegral(iBoard, iCh, 20, 200, 450);
                    }
                    else{
                        chargeInt_buf = GetChargeIntegral(iBoard, iCh, 20, 0, 1023);
                    }
                    fH1EnergySpectra[iBoard][iCh][iDiv]->Fill(p0_buf+(-chargeInt_buf)*p1_buf);

                }
            }
        }
        for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                colorIndex = 255*iDiv/divOfTime;
                colorIndex_key = TColor::GetColorPalette(colorIndex);
                fH1EnergySpectra[iBoard][iCh][iDiv]->SetLineColor(colorIndex_key);
            }
        }
    }

    for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            for(Int_t iDiv=0; iDiv<divOfTime; iDiv++){
                canvas->cd(iBoard*4+iCh+1);

                if(iDiv == 0){
                    fH1EnergySpectra[iBoard][iCh][iDiv]->Draw();
                }
                else{
                    fH1EnergySpectra[iBoard][iCh][iDiv]->Draw("SAME");
                }
                legend[iBoard][iCh]->SetTextSize(0.03);
                legend[iBoard][iCh]->SetBorderSize(1);
                // 凡例にエントリを追加
                TString legendLabel = Form("Time Div %d", iDiv + 1);
                legend[iBoard][iCh]->AddEntry(fH1EnergySpectra[iBoard][iCh][iDiv], legendLabel, "l");
                std::cout << Form("\tDraw : iBoard %d, iCh %d, iDiv %d", iBoard, iCh, iDiv) << std::endl;        
            }
        }
    }
    if(divOfTime > 1){
        for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            canvas->cd(iBoard*4+iCh + 1);
            legend[iBoard][iCh]->Draw();
        }
        }
    }
    canvas->Update();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    printf("\n\tfigure saved as: %s\n", filename_figure.Data());
    // canvas->SaveAs(Form("../figure/%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/timeDiv_%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/timeDiv_%s.pdf", filename_figure.Data()));
    
    return counter;
}

Double_t DRS4Ana::time_divided_adcSum(Int_t divOfTime = 10){
    Long64_t nentries = fChain->GetEntriesFast();
    // Long64_t nentries = 10000;
    Long64_t counter = 0;
    Int_t numOfBoards = 1;

    TCanvas *canvas = new TCanvas("canvas", "title", 1600, 1200);
    canvas->Divide(2,numOfBoards*2);
    if(divOfTime>1){
        gStyle->SetPalette(kCool);
    }
    TH1D* fH1EnergySpectra[2][4][divOfTime];
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            for(Int_t iDiv=0; iDiv<divOfTime; iDiv++){
                fH1EnergySpectra[iBoard][iCh][iDiv] = new TH1D(Form("fH1EnergySpectra || iB : %d, iC : %d, iDiv : %d", iBoard, iCh, iDiv), Form("iB : %d, iC : %d, iDiv : %d", iBoard, iCh, iDiv), 400, 0, 250);
            }
            canvas->cd(iBoard*4+iCh+1);
            gPad->SetGrid();
        }
    }
    gPad->SetGrid();
    gStyle->SetOptStat(0);

    Double_t chargeInt_buf;
    Int_t colorIndex_key, colorIndex;
    TLegend* legend[2][4];
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            legend[iBoard][iCh] = new TLegend(0.7, 0.5, 0.9, 0.9);
        }
    }

    for(Int_t iDiv=0; iDiv<divOfTime; iDiv++){
        for(Int_t Entry = iDiv*(nentries/divOfTime); Entry<(iDiv+1)*(nentries/divOfTime); Entry++){
            fChain->GetEntry(Entry);
            counter++;
            if(counter % 1000 == 0){
                std::cout << "\tcounter : " << counter << std::endl;
            }

            for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
                for(Int_t iCh=0; iCh<4; iCh++){
                    canvas->cd(iBoard*4+iCh+1);
                    chargeInt_buf = GetChargeIntegral(iBoard, iCh, 20, 0, 1023);
                    fH1EnergySpectra[iBoard][iCh][iDiv]->Fill(-chargeInt_buf);

                }
            }
        }
        for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                colorIndex = 255*iDiv/divOfTime;
                colorIndex_key = TColor::GetColorPalette(colorIndex);
                fH1EnergySpectra[iBoard][iCh][iDiv]->SetLineColor(colorIndex_key);
            }
        }
    }

    for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            for(Int_t iDiv=0; iDiv<divOfTime; iDiv++){
                canvas->cd(iBoard*4+iCh+1);

                if(iDiv == 0){
                    fH1EnergySpectra[iBoard][iCh][iDiv]->Draw();
                }
                else{
                    fH1EnergySpectra[iBoard][iCh][iDiv]->Draw("SAME");
                }
                legend[iBoard][iCh]->SetTextSize(0.03);
                legend[iBoard][iCh]->SetBorderSize(1);
                // 凡例にエントリを追加
                TString legendLabel = Form("Time Div %d", iDiv + 1);
                legend[iBoard][iCh]->AddEntry(fH1EnergySpectra[iBoard][iCh][iDiv], legendLabel, "l");
                std::cout << Form("\tDraw : iBoard %d, iCh %d, iDiv %d", iBoard, iCh, iDiv) << std::endl;        
            }
        }
    }
    if(divOfTime > 1){
        for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            canvas->cd(iBoard*4+iCh + 1);
            legend[iBoard][iCh]->Draw();
        }
        }
    }
    canvas->Update();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    printf("\n\tfigure saved as: %s\n", filename_figure.Data());
    // canvas->SaveAs(Form("../figure/%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/timeDiv_%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/timeDiv_%s.pdf", filename_figure.Data()));
    
    return counter;
}
Double_t DRS4Ana::Print_discriCell(Int_t iBoard = 0, Int_t iCh = 0){
    Long64_t nentries = fChain->GetEntriesFast();
    // Long64_t nentries = 10000;
    Long64_t counter = 0;
    if(fH1TriggerTime != NULL){
        delete fH1TriggerTime;
    }
    TCanvas *c1 = new TCanvas("c1", "Canvas", 800, 600);
    Int_t div = 512;
    fH1TriggerTime = new TH1F("fH1TriggerTime", Form("%s: (iBoard %d, iCh %d Trigger Time", fRootFile.Data(), iBoard, iCh), div, 0, 1023);
    fH1TriggerTime->SetTitle(Form(";trigger cell( nearly equal to trigger time [ns]);count per %.2f", 1024.0/div));

    Int_t discriCell;
    for(Int_t eventID=0; eventID<nentries; eventID++){
        fChain->GetEntry(eventID);
        discriCell = fDiscriCell[iBoard][iCh];
        
        if(counter % 10000 == 0){
            printf("\ttrigger : %d (%.1f [ns])\n", discriCell, fTime[iBoard][iCh][discriCell]);
        }
        fH1TriggerTime->Fill(discriCell);
        counter++;
    }
    fH1TriggerTime->Draw();
    return (Double_t)counter;
}

Double_t DRS4Ana::NaI_peaksearch(Int_t iBoard = 0, Int_t iCh = 0, Double_t adcMin = 0, Double_t adcMax = 150.0, Int_t numPeaks = 10, Double_t fitRange = 2.0, Double_t spec_sigma = 5.0)
{
    Int_t append_Option = 1; //1 for not to overwrite the output.
    Int_t timecut_Option = 1; //1 to restrict the time range for better energy resolution
    Double_t adcTimeRange = 600.0;

    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    Double_t timeCut_begin, timeCut_end;

    if(timecut_Option == 1){
        fChain->GetEntry(0);
        timeCut_begin = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]]-50;
        timeCut_end = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] + adcTimeRange;
    }
    else{
        timeCut_begin = fTime[iBoard][iCh][0]; //時間カットなし
        timeCut_end = fTime[iBoard][iCh][1023]; 
    }

    if (fH1ChargeIntegral != NULL)
    {
        delete fH1ChargeIntegral;
    }

    std::cout << "================================================================" << std::endl << "NaI peaksearch" << std::endl << "\tiBoard : " << iBoard << std::endl << "\tiCh : " << iCh << std::endl << "\tadcMin : " << adcMin << std::endl << "\tadcMax : " << adcMax << std::endl << std::endl;
    std::cout << "\tFit information\n" << "\t\ttimeCut_begin = " << timeCut_begin << " (first event)\n" << "\t\ttimeCut_end = " << timeCut_end << " (first event)\n" << "\t\tfitRange = " << fitRange << "\n\t\tspec_sigma = " << spec_sigma << std::endl;
    std::cout << "================================================================" << std::endl;

    //canvasの宣言など...
    TCanvas *c1 = new TCanvas("c1", "Canvas", 1600, 1200);
    fH1ChargeIntegral = new TH1F("fH1ChargeIntegral", Form("%s:ch%d Charge Integral(for NaI) [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax), 500, adcMin, adcMax);
    fH1ChargeIntegral->SetXTitle("Voltage sum [V]");
    fH1ChargeIntegral->SetYTitle(Form("[counts] per %.2f V", (adcMax-adcMin)/500));
    gPad->SetGrid();

    //chargeIntegralの計算
    Double_t chargeIntegral;
    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        timeCut_begin = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] - 50;//トリガー時刻から-50 ns遡ってsum
        timeCut_end = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] + adcTimeRange;//トリガー時刻から+adcTimeRange nsまでsum
        if(timecut_Option==0){
            timeCut_begin = fTime[iBoard][iCh][0];
            timeCut_end = fTime[iBoard][iCh][1023];
        }

        chargeIntegral = GetChargeIntegral(iBoard, iCh, 20, timeCut_begin, timeCut_end);
        
        if (chargeIntegral > -9999.9)
        {
            counter++;
            fH1ChargeIntegral->Fill(-chargeIntegral);
        }
    }
    fH1ChargeIntegral->Draw();

    //まずはpeaksearchを自動で行う
    TSpectrum *spectrum = new TSpectrum(numPeaks); //numPeaksは実際に見つけたいピークよりも多く設定しておくと良い
    spectrum->SetResolution(5);

    Double_t spec_thr = 0.005;

    Int_t foundPeaks = spectrum->Search(fH1ChargeIntegral, spec_sigma, "", spec_thr); //要調整 .Search(a, b, c, d)のうち、bはどれくらいの太さ以上のピークを見つけたいか。cはオプション。dは最大のピークに対してどれくらいの大きさのピークまで探すかを指している。0.1だと最大のピークの10%の高さのピークまで探す。
    Double_t* peakPositions = spectrum->GetPositionX();

    //peaksearchの結果に応じてフィッティングを行い、パラメータを最適化する
    std::vector<TF1*> fits; //"gaus"フィッティングを複数格納するベクトル
    std::vector<Double_t> means;
    std::vector<Double_t> sigmas_mean;
    std::vector<Double_t> sigmas_gaus;
    std::vector<Double_t> intercept;
    std::vector<Double_t> slope;

    std::vector<Double_t> chi2_ndof_vec; 
    std::vector<Double_t> prob_vec;

    std::vector<TFitResultPtr> fitresults;


    for(int i=0; i<foundPeaks; ++i){
        TF1* gaussian_plus_linear = new TF1(
            Form("gaussian_plus_linear_%d",i), "gaus+pol1(3)", 
            peakPositions[i]-fitRange, peakPositions[i]+fitRange
        ); //要調整。特に範囲


        // 初期パラメータの設定
     gaussian_plus_linear->SetParameters(
         fH1ChargeIntegral->GetBinContent(fH1ChargeIntegral->FindBin(peakPositions[i])), // ガウスの振幅 [0]
         peakPositions[i],                                                       // ガウスの中心 [1]
         1.0,                                                                    // ガウスの幅 [2]
         50.0,                                                                    // 一次関数の切片 [3]
         -5.0                                                                    // 一次関数の傾き [4]
     );
        
        
        TFitResultPtr fit_result = fH1ChargeIntegral->Fit(gaussian_plus_linear, "RS+"); //オプションは好きに。TFitResultPtrはフィッティングの結果を保持する型。あとでフィッティングの可否判定に使う。
        Int_t checking = fit_result->Status();


    double chi2 = gaussian_plus_linear -> GetChisquare();  // χ²
    int ndof = gaussian_plus_linear -> GetNDF();           // 自由度
    double chi2_ndof = (ndof > 0) ? chi2 / ndof : 0; // 0除算回避
    double prob = TMath::Prob(chi2, ndof);

    chi2_ndof_vec.push_back(chi2_ndof);
    prob_vec.push_back(prob);


        if(checking != 0){}
        else{
             fits.push_back(gaussian_plus_linear);
             means.push_back(gaussian_plus_linear->GetParameter(1));           // ガウス中心値
             sigmas_mean.push_back(gaussian_plus_linear->GetParError(1));      // ガウス中心値の誤差
             sigmas_gaus.push_back(gaussian_plus_linear->GetParameter(2));     // ガウス幅
             intercept.push_back(gaussian_plus_linear->GetParameter(3));       // 切片
             slope.push_back(gaussian_plus_linear->GetParameter(4));           // 傾き
            }
    
    // 各成分を個別にプロットする
        TF1* gauss1 = new TF1("gauss1", "gaus", peakPositions[i] - fitRange, 
        peakPositions[i] + fitRange);
        gauss1->SetParameters(
            gaussian_plus_linear->GetParameter(0), // 振幅
            gaussian_plus_linear->GetParameter(1), // 中心
            gaussian_plus_linear->GetParameter(2)  // 幅
        );
        gauss1->SetLineColor(kOrange+7);
        gauss1->SetLineStyle(1);
        gauss1->Draw("LSAME");

        TF1* linear = new TF1("linear", "pol1", peakPositions[i] - fitRange, 
        peakPositions[i] + fitRange);
        linear->SetParameters(
            gaussian_plus_linear->GetParameter(3), // 切片
            gaussian_plus_linear->GetParameter(4)  // 傾き
        );
        linear->SetLineColor(kGreen+1);
        linear->SetLineStyle(1);
        linear->Draw("LSAME");
    
    }
    c1->Update();

    //結果の図やフィッティングパラメータを保存する。フィッティングパラメータは"./output/GSO_peaksearch_data.txt"に追記して保存する。図は"./figure/"にYYYYMMDDというフォルダを作ってその中に保存する。
    TString filename_figure;
    TString rootFile = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/')); //.rootファイルのフルパスからファイル名だけを抜き出した
    rootFile.ReplaceAll(".", "_"); //.dat.rootのドットを"_"に変えた

    std::ofstream ofs;
    if(append_Option == 1){
        ofs.open("./output/NaI_peaksearch_data.txt", std::ios::app);
    }
    else{
        ofs.open(Form("./output/%s_data.txt",rootFile.Data()));
    }
    
    auto mean_temp = means.begin();
    auto sigma_mean_temp = sigmas_mean.begin();
    auto sigma_gaus_temp = sigmas_gaus.begin();
    auto intercept_temp = intercept.begin();
    auto slope_temp = slope.begin();

    auto chi2_ndof_temp = chi2_ndof_vec.begin();
    auto prob_temp = prob_vec.begin();

    if(append_Option == 1){
        ofs << std::endl << "================================================================" << std::endl << ".rootfile || filepath : " << fRootFile.Data() << std::endl;
        auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
        std::tm local_tm = *std::localtime(&now_c);

        ofs << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    }
    ofs << "means, sigmas of means, sigmas of gaussian, intercept ,slope, chi2_ndof, prob" << std::endl << std::endl;
    while(mean_temp != means.end() && sigma_mean_temp != sigmas_mean.end() && sigma_gaus_temp != sigmas_gaus.end() && intercept_temp != intercept.end() && slope_temp != slope.end()){
        ofs << *mean_temp << " " << *sigma_mean_temp << " " << *sigma_gaus_temp << " " << *intercept_temp << " " << *slope_temp << " " << *chi2_ndof_temp << " " << *prob_temp << std::endl;
        ++mean_temp; //peak[1]
        ++sigma_mean_temp; //sigma_m 
        ++sigma_gaus_temp; //sigma[2]
        ++intercept_temp; //切片[3]
        ++slope_temp; //傾き[4]
        ++chi2_ndof_temp; 
        ++prob_temp;
    }
    ofs << std::endl << "numPeak : " << numPeaks << std::endl; // ピークの数
    ofs << "spec_sigma : " << spec_sigma << std::endl; // ピークの太さ
    ofs << "spec_thr : " << spec_thr << std::endl; // 最大ピークに対する高さの割合
    ofs << "fitrange : " << fitRange << std::endl; // ピーク中心からの範囲

    ofs.close();
    

    //図を保存するフォルダのための日付
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm); // "YYYYMMDD"形式で日付を取得
    //YYYYMMDDフォルダのパス
    TString folderPath = TString::Format("./figure/%s", date);
    //フォルダが存在しない場合は作成
    if (gSystem->AccessPathName(folderPath)) {
        if (gSystem->mkdir(folderPath, true) != 0) {
            std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
            return -1;
        }
    }

    filename_figure = Form("%s:ch%d_NaI_peaksearch.pdf", rootFile.Data(), iCh);

    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName(folderPath + '/' + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("%s:ch%d_NaI_peaksearch_%d.pdf", rootFile.Data(), iCh, index);
        index++;
    }

    c1->SaveAs(folderPath + '/' + filename_figure);

    return (Double_t)counter;

}


Double_t DRS4Ana::GSO_peaksearch(Int_t iBoard = 0, Int_t iCh = 0, Double_t adcMin = 0, Double_t adcMax = 150.0, Int_t numPeaks = 10, Double_t fitRange = 2.0, Double_t spec_sigma = 5.0)
{
    Int_t append_Option = 1; //1 for not to overwrite the output.
    Int_t timecut_Option = 1; //1 to restrict the time range for better energy resolution
    Double_t adcTimeRange = 180.0;

    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    Double_t timeCut_begin, timeCut_end;

    if(timecut_Option == 1){
        timeCut_begin = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] - 50; //50 ns before trig
        timeCut_end = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] + adcTimeRange; //adcTimeRange ns after trig
    }

    if (fH1ChargeIntegral != NULL)
    {
        delete fH1ChargeIntegral;
    }

    std::cout << "================================================================" << std::endl << "GSO peaksearch" << std::endl << "\tiBoard : " << iBoard << std::endl << "\tiCh : " << iCh << std::endl << "\tadcMin : " << adcMin << std::endl << "\tadcMax : " << adcMax << std::endl << std::endl;
    std::cout << "\tFit information" << std::endl << "\t\ttimeCut_begin = " << timeCut_begin << std::endl << "\t\ttimeCut_end = " << timeCut_end << std::endl << "\t\tfitRange = " << fitRange << std::endl;
    std::cout << "================================================================" << std::endl;

    //canvasの宣言など...
    TCanvas *c1 = new TCanvas("c1", "Canvas", 1600, 1200);
    fH1ChargeIntegral = new TH1F("fH1ChargeIntegral", Form("%s:ch%d Charge Integral(for GSO) [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax), 500, adcMin, adcMax);
    fH1ChargeIntegral->SetXTitle("Voltage sum [V]");
    fH1ChargeIntegral->SetYTitle(Form("[counts / %.2f V]", (adcMax-adcMin)/500));
    gPad->SetGrid();

    //chargeIntegralの計算
    Double_t chargeIntegral;
    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        timeCut_begin = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] - 50; //50 ns before trig
        timeCut_end = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] + adcTimeRange; //adcTimeRange ns after trig
    
        chargeIntegral = GetChargeIntegral(iBoard, iCh, 20, timeCut_begin, timeCut_end);
        

        if (chargeIntegral > -9999.9)
        {
            counter++;
            fH1ChargeIntegral->Fill(-chargeIntegral);
        }
    }
    fH1ChargeIntegral->Draw();

    //まずはpeaksearchを自動で行う
    TSpectrum *spectrum = new TSpectrum(numPeaks); //numPeaksは実際に見つけたいピークよりも多く設定しておくと良い
    spectrum->SetResolution(5);

    Double_t spec_thr = 0.001;

    Int_t foundPeaks = spectrum->Search(fH1ChargeIntegral, spec_sigma, "", spec_thr);
    //要調整 .Search(a, b, c, d)のうち、bはどれくらいの太さ以上のピークを見つけたいか。cはオプション。dは最大のピークに対してどれくらいの大きさのピークまで探すかを指している。0.1だと最大のピークの10%の高さのピークまで探す。
    Double_t* peakPositions = spectrum->GetPositionX();

    //peaksearchの結果に応じてフィッティングを行い、パラメータを最適化する
    std::vector<TF1*> fits; //"gaus"フィッティングを複数格納するベクトル
    std::vector<Double_t> means;
    std::vector<Double_t> sigmas_mean;
    std::vector<Double_t> sigmas_gaus;
    std::vector<Double_t> intercept;
    std::vector<Double_t> slope;
    
    std::vector<Double_t> chi2_ndof_vec; 
    std::vector<Double_t> prob_vec;


    std::vector<TFitResultPtr> fitresults;

   



for (int i = 0; i < foundPeaks; ++i) {
    // ガウス関数 + 一次関数の定義
    TF1* gaussian_plus_linear = new TF1(
        Form("gaussian_plus_linear_%d", i),
        //"[0] * exp(-0.5 * ((x - [1])/[2])**2) + [3] + [4]*x", 
        "gaus+pol1(3)", 
        peakPositions[i] - fitRange, 
        peakPositions[i] + fitRange
    );
    

    // 初期パラメータの設定
     gaussian_plus_linear->SetParameters(
         fH1ChargeIntegral->GetBinContent(fH1ChargeIntegral->FindBin(peakPositions[i])), // ガウスの振幅 [0]
         peakPositions[i],                                                       // ガウスの中心 [1]
         1.0,                                                                    // ガウスの幅 [2]
         50.0,                                                                    // 一次関数の切片 [3]
         -5.0                                                                    // 一次関数の傾き [4]
     );

    double chi2 = gaussian_plus_linear -> GetChisquare();  // χ²
    int ndof = gaussian_plus_linear -> GetNDF();           // 自由度
    double chi2_ndof = (ndof > 0) ? chi2 / ndof : 0; // 0除算回避
    double prob = TMath::Prob(chi2, ndof);

    chi2_ndof_vec.push_back(chi2_ndof);
    prob_vec.push_back(prob);

    // フィッティング
    TFitResultPtr fit_result = fH1ChargeIntegral->Fit(gaussian_plus_linear, "RS+"); // オプション "RS+" を使用
    std::cout << "debug" << std::endl;
    Int_t checking = fit_result->Status();

    if (checking != 0) {
        // フィッティングが失敗した場合の処理（必要に応じて記述）
        std::cout << "no fit" << std::endl;
    } else {
        // フィッティング成功時の処理
        fits.push_back(gaussian_plus_linear);
        means.push_back(gaussian_plus_linear->GetParameter(1));           // ガウス中心値
        sigmas_mean.push_back(gaussian_plus_linear->GetParError(1));      // ガウス中心値の誤差
        sigmas_gaus.push_back(gaussian_plus_linear->GetParameter(2));     // ガウス幅
        intercept.push_back(gaussian_plus_linear->GetParameter(3));       // 切片
        slope.push_back(gaussian_plus_linear->GetParameter(4));           // 傾き
    }



    // 各成分を個別にプロットする
        TF1* gauss1 = new TF1("gauss1", "gaus", peakPositions[i] - fitRange, 
        peakPositions[i] + fitRange);
        gauss1->SetParameters(
            gaussian_plus_linear->GetParameter(0), // 振幅
            gaussian_plus_linear->GetParameter(1), // 中心
            gaussian_plus_linear->GetParameter(2)  // 幅
        );
        gauss1->SetLineColor(kOrange+7);
        gauss1->SetLineStyle(1);
        gauss1->Draw("LSAME");

        TF1* linear = new TF1("linear", "pol1", peakPositions[i] - fitRange, 
        peakPositions[i] + fitRange);
        linear->SetParameters(
            gaussian_plus_linear->GetParameter(3), // 切片
            gaussian_plus_linear->GetParameter(4)  // 傾き
        );
        linear->SetLineColor(kGreen+1);
        linear->SetLineStyle(1);
        linear->Draw("LSAME");

}


    c1->Update();

    //結果の図やフィッティングパラメータを保存する。フィッティングパラメータは"./output/GSO_peaksearch_data.txt"に追記して保存する。図は"./figure/"にYYYYMMDDというフォルダを作ってその中に保存する。
    TString filename_figure;
    TString rootFile = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/')); //.rootファイルのフルパスからファイル名だけを抜き出した
    rootFile.ReplaceAll(".", "_"); //.dat.rootのドットを"_"に変えた

    std::ofstream ofs;
    if(append_Option == 1){
        ofs.open("./output/GSO_peaksearch_data.txt", std::ios::app);
    }
    else{
        ofs.open(Form("./output/%s_data.txt",rootFile.Data()));
    }
    
    auto mean_temp = means.begin();
    auto sigma_mean_temp = sigmas_mean.begin();
    auto sigma_gaus_temp = sigmas_gaus.begin();
    auto intercept_temp = intercept.begin();
    auto slope_temp = slope.begin();

    auto chi2_ndof_temp = chi2_ndof_vec.begin();
    auto prob_temp = prob_vec.begin();



    if(append_Option == 1){
        ofs << std::endl << "================================================================" << std::endl << ".rootfile || filepath : " << fRootFile.Data() << std::endl;
        auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
        std::tm local_tm = *std::localtime(&now_c);

        ofs << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    }
    ofs << "means, sigmas of means, sigmas of gaussian, intercept ,slope, chi2_ndof, prob" << std::endl << std::endl;
    while(mean_temp != means.end() && sigma_mean_temp != sigmas_mean.end() && sigma_gaus_temp != sigmas_gaus.end() && intercept_temp != intercept.end() && slope_temp != slope.end()){
        ofs << *mean_temp << " " << *sigma_mean_temp << " " << *sigma_gaus_temp << " " << *intercept_temp << " " << " " << *slope_temp << " " << *chi2_ndof_temp << " " << *prob_temp << std::endl;
        ++mean_temp; //peak[1]
        ++sigma_mean_temp; //sigma_m 
        ++sigma_gaus_temp; //sigma[2]
        ++intercept_temp; //切片[3]
        ++slope_temp; //傾き[4]
        ++chi2_ndof_temp; 
        ++prob_temp;
    }
    ofs << std::endl << "numPeak : " << numPeaks << std::endl; // ピークの数
    ofs << "spec_sigma : " << spec_sigma << std::endl; // ピークの太さ
    ofs << "spec_thr : " << spec_thr << std::endl; // 最大ピークに対する高さの割合
    ofs << "fitrange : " << fitRange << std::endl; // ピーク中心からの範囲
    ofs << "spec sigma : " << spec_sigma << std::endl; //ピークサーチの幅
    ofs.close();
    

    //図を保存するフォルダのための日付
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm); // "YYYYMMDD"形式で日付を取得
    //YYYYMMDDフォルダのパス
    TString folderPath = TString::Format("./figure/%s", date);
    //フォルダが存在しない場合は作成
    if (gSystem->AccessPathName(folderPath)) {
        if (gSystem->mkdir(folderPath, true) != 0) {
            std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
            return -1;
        }
    }

    filename_figure = Form("%s:ch%d_GSO_peaksearch.pdf", rootFile.Data(), iCh);

    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName(folderPath + '/' + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("%s:ch%d_GSO_peaksearch_%d.pdf", rootFile.Data(), iCh, index);
        index++;
    }

    c1->SaveAs(folderPath + '/' + filename_figure);

    return (Double_t)counter;
}


Double_t DRS4Ana::peak_divided(Int_t iBoard = 0, Int_t iCh = 0, Double_t adcMin = 0.0, Double_t adcMax = 150.0, Double_t fitXmin = 0.0, Double_t fitXmax = 0.0, Double_t adcTimeRange = 180.0)
{
    Int_t append_Option = 1; //1 for not to overwrite the output.
    Int_t timecut_Option = 1; //1 to restrict the time range for better energy resolution

    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    Double_t timeCut_begin, timeCut_end;

    if(timecut_Option == 1){
        timeCut_begin = fDiscriCell[iBoard][iCh] - 50; //50 ns before trig
        timeCut_end = fDiscriCell[iBoard][iCh] + adcTimeRange; //adcTimeRange ns after trig
    }

    if (fH1ChargeIntegral != NULL)
    {
        delete fH1ChargeIntegral;
    }

    std::cout << "================================================================" << std::endl << "GSO peaksearch" << std::endl << "\tiBoard : " << iBoard << std::endl << "\tiCh : " << iCh << std::endl << "\tadcMin : " << adcMin << std::endl << "\tadcMax : " << adcMax << std::endl << std::endl;
    std::cout << "\tFit information" << std::endl << "\t\ttimeCut_begin = " << timeCut_begin << std::endl << "\t\ttimeCut_end = " << timeCut_end << std::endl;
    std::cout << "================================================================" << std::endl;

    //canvasの宣言など...
    TCanvas *c1 = new TCanvas("c1", "Canvas", 1600, 1200);
    fH1ChargeIntegral = new TH1F("fH1ChargeIntegral", Form("%s:ch%d Charge Integral(for GSO) [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax), 500, adcMin, adcMax);
    fH1ChargeIntegral->SetXTitle("voltage sum [V]");
    fH1ChargeIntegral->SetYTitle(Form("[counts / %.2f V]", (adcMax-adcMin)/500));
    gPad->SetGrid();

    //chargeIntegralの計算
    Double_t chargeIntegral;
    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        timeCut_begin = fDiscriCell[iBoard][iCh] - 50; //50 ns before trig
        timeCut_end = fDiscriCell[iBoard][iCh] + adcTimeRange; //adcTimeRange ns after trig
        chargeIntegral = GetChargeIntegral(iBoard, iCh, 20, timeCut_begin, timeCut_end);
        
        if (chargeIntegral > -9999.9)
        {
            counter++;
            fH1ChargeIntegral->Fill(-chargeIntegral);
        }
    }
    fH1ChargeIntegral->Draw();

    // //まずはpeaksearchを自動で行う
    // TSpectrum *spectrum = new TSpectrum(numPeaks); //numPeaksは実際に見つけたいピークよりも多く設定しておくと良い
    // spectrum->SetResolution(5);
    // Double_t spec_sigma = 6.0; //分解能みたいな 小さいほど鋭いピークになる
    // Double_t spec_thr = 0.001;
    // Int_t foundPeaks = spectrum->Search(fH1ChargeIntegral, spec_sigma, "", spec_thr);
    // //要調整 .Search(a, b, c, d)のうち、bはどれくらいの太さ以上のピークを見つけたいか。cはオプション。dは最大のピークに対してどれくらいの大きさのピークまで探すかを指している。0.1だと最大のピークの10%の高さのピークまで探す。
    // Double_t* peakPositions = spectrum->GetPositionX();

    //peaksearchの結果に応じてフィッティングを行い、パラメータを最適化する
    std::vector<TF1*> fits; //"gaus"フィッティングを複数格納するベクトル
    std::vector<Double_t> means1;
    std::vector<Double_t> sigmas_mean1;
    std::vector<Double_t> sigmas_gaus1;
    std::vector<Double_t> means2;
    std::vector<Double_t> sigmas_mean2;
    std::vector<Double_t> sigmas_gaus2;
    std::vector<Double_t> intercept;
    std::vector<Double_t> slope;
    std::vector<TFitResultPtr> fitresults;
    

    // ガウス関数 + 一次関数の定義
    TF1* gaussian_plus_linear = new TF1(
        Form("gaussian_plus_linear"),
        //"[0] * exp(-0.5 * ((x - [1])/[2])**2) + [3] + [4]*x", 
        "gaus+gaus(3)+pol1(6)", 
        fitXmin, 
        fitXmax
    );
    

    // 初期パラメータの設定
     gaussian_plus_linear->SetParameters(
         300.0,     // ガウスの振幅 [0]
         3.5,     // ガウスの中心 [1]
         0.01,     // ガウスの幅 [2]
         200.0,   // ガウスの振幅 [3]
         4.0,     // ガウスの中心 [4]
         0.1,      // ガウスの幅 [5]
         100.0,    // 一次関数の切片 [6]
         -10.0     // 一次関数の傾き [7]
     );

    // フィッティング
    TFitResultPtr fit_result = fH1ChargeIntegral->Fit(gaussian_plus_linear, "RS+"); // オプション "RS+" を使用
    std::cout << "debug" << std::endl;
    Int_t checking = fit_result->Status();

    if (checking != 0) {
        // フィッティングが失敗した場合の処理（必要に応じて記述）
        std::cout << "no fit" << std::endl;
    } else {
        // フィッティング成功時の処理
        fits.push_back(gaussian_plus_linear);
        means1.push_back(gaussian_plus_linear->GetParameter(1));           // ガウス中心値
        sigmas_mean1.push_back(gaussian_plus_linear->GetParError(1));      // ガウス中心値の誤差
        sigmas_gaus1.push_back(gaussian_plus_linear->GetParameter(2));     // ガウス幅
        
        means2.push_back(gaussian_plus_linear->GetParameter(4));           // ガウス中心値
        sigmas_mean2.push_back(gaussian_plus_linear->GetParError(4));      // ガウス中心値の誤差
        sigmas_gaus2.push_back(gaussian_plus_linear->GetParameter(5));     // ガウス幅
        
        intercept.push_back(gaussian_plus_linear->GetParameter(6));       // 切片
        slope.push_back(gaussian_plus_linear->GetParameter(7));           // 傾き
    }

// 各成分を個別にプロットする
        TF1* gauss1 = new TF1("gauss1", "gaus", fitXmin, fitXmax);
        gauss1->SetParameters(
            gaussian_plus_linear->GetParameter(0), // 振幅
            gaussian_plus_linear->GetParameter(1), // 中心
            gaussian_plus_linear->GetParameter(2)  // 幅
        );
        gauss1->SetLineColor(kOrange+7);
        gauss1->SetLineStyle(1);
        gauss1->Draw("LSAME");

        TF1* gauss2 = new TF1("gauss2", "gaus", fitXmin, fitXmax);
        gauss2->SetParameters(
            gaussian_plus_linear->GetParameter(3), // 振幅
            gaussian_plus_linear->GetParameter(4), // 中心
            gaussian_plus_linear->GetParameter(5)  // 幅
        );
        gauss2->SetLineColor(kGreen+2);
        gauss2->SetLineStyle(1);
        gauss2->Draw("LSAME");

        TF1* linear = new TF1("linear", "pol1", fitXmin, fitXmax);
        linear->SetParameters(
            gaussian_plus_linear->GetParameter(6), // 切片
            gaussian_plus_linear->GetParameter(7)  // 傾き
        );
        linear->SetLineColor(kRed);
        linear->SetLineStyle(1);
        linear->Draw("LSAME");

    c1->Update();

    //結果の図やフィッティングパラメータを保存する。フィッティングパラメータは"./output/GSO_peaksearch_data.txt"に追記して保存する。図は"./figure/"にYYYYMMDDというフォルダを作ってその中に保存する。
    TString filename_figure;
    TString rootFile = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/')); //.rootファイルのフルパスからファイル名だけを抜き出した
    rootFile.ReplaceAll(".", "_"); //.dat.rootのドットを"_"に変えた

    std::ofstream ofs;
    if(append_Option == 1){
        ofs.open("./output/GSO_peak_divided.txt", std::ios::app);
    }
    else{
        ofs.open(Form("./output/%s_data.txt",rootFile.Data()));
    }
    
    auto mean_temp1 = means1.begin();
    auto sigma_mean_temp1 = sigmas_mean1.begin();
    auto sigma_gaus_temp1 = sigmas_gaus1.begin();

    auto mean_temp2 = means2.begin();
    auto sigma_mean_temp2 = sigmas_mean2.begin();
    auto sigma_gaus_temp2 = sigmas_gaus2.begin();
    auto intercept_temp = intercept.begin();
    auto slope_temp = slope.begin();



    if(append_Option == 1){
        ofs << std::endl << "================================================================" << std::endl << ".rootfile || filepath : " << fRootFile.Data() << std::endl;
        auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
        std::tm local_tm = *std::localtime(&now_c);

        ofs << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    }
    ofs << "means1, sigmas of means1, sigmas of gaussian1" << std::endl << "means2, sigmas of means2, sigmas of gaussian2" << std::endl <<"intercept ,slope" << std::endl << std::endl;
    while(mean_temp1 != means1.end() && sigma_mean_temp1 != sigmas_mean1.end() && sigma_gaus_temp1 != sigmas_gaus1.end() 
    && mean_temp2 != means2.end() && sigma_mean_temp2 != sigmas_mean2.end() && sigma_gaus_temp2 != sigmas_gaus2.end() 
    && intercept_temp != intercept.end() && slope_temp != slope.end()){
        
        ofs << *mean_temp1 << " " << *sigma_mean_temp1 << " " << *sigma_gaus_temp1 << std::endl
        << *mean_temp2 << " " << *sigma_mean_temp2 << " " << *sigma_gaus_temp2 << std::endl
        << *intercept_temp << " " << " " << *slope_temp <<  std::endl;
        ++mean_temp1; //peak[1]
        ++sigma_mean_temp1; //sigma_m 
        ++sigma_gaus_temp1; //sigma[2]
        ++mean_temp2; //peak[1]
        ++sigma_mean_temp2; //sigma_m 
        ++sigma_gaus_temp2; //sigma[2]
        ++intercept_temp; //切片[3]
        ++slope_temp; //傾き[4]
    }
    // ofs << std::endl << "numPeak : " << numPeaks << std::endl; // ピークの数
    // ofs << "spec_sigma : " << spec_sigma << std::endl; // ピークの太さ
    // ofs << "spec_thr : " << spec_thr << std::endl; // 最大ピークに対する高さの割合
    // ofs << "fitrange : " << fitRange << std::endl; // ピーク中心からの範囲
    // ofs.close();
    

    //図を保存するフォルダのための日付
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm); // "YYYYMMDD"形式で日付を取得
    //YYYYMMDDフォルダのパス
    TString folderPath = TString::Format("./figure/%s", date);
    //フォルダが存在しない場合は作成
    if (gSystem->AccessPathName(folderPath)) {
        if (gSystem->mkdir(folderPath, true) != 0) {
            std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
            return -1;
        }
    }

    filename_figure = Form("%s:ch%d_peak_divide.pdf", rootFile.Data(), iCh);

    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName(folderPath + '/' + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("%s:ch%d_peak_divided_%d.pdf", rootFile.Data(), iCh, index);
        index++;
    }

    c1->SaveAs(folderPath + '/' + filename_figure);

    return (Double_t)counter;
}


Double_t DRS4Ana::semi_automated_spectrum_fitting(TString key_crystal = "NaI", Int_t iBoard, Int_t iCh, Double_t adcMin = 0, Double_t adcMax = 100){
    Int_t append_Option = 1; //1 for not to overwrite the output.
    Int_t timecut_Option = 1; //1 to restrict the time range for better energy resolution
    Double_t adcTimeRange;
    if(key_crystal == "NaI"){
        adcTimeRange = 600.0;
    }
    else if(key_crystal == "GSO"){
        adcTimeRange = 180.0;
    }
    else{
        printf("\tkey invalid\n");
    }

    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    Double_t timeCut_begin, timeCut_end;

    if(timecut_Option == 1){
        fChain->GetEntry(0);
        timeCut_begin = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] - 50.0;
        timeCut_end = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] + adcTimeRange;
    }

    if (fH1ChargeIntegral != NULL)
    {
        delete fH1ChargeIntegral;
    }

    std::cout << "================================================================" << std::endl << "semi-auto peak fitting" << std::endl << "\tiBoard : " << iBoard << std::endl << "\tiCh : " << iCh << std::endl << "\tadcMin : " << adcMin << std::endl << "\tadcMax : " << adcMax << std::endl << std::endl;
    std::cout << "\tFit information\n" << "\t\ttimeCut_begin = " << timeCut_begin << " (first event)\n" << "\t\ttimeCut_end = " << timeCut_end << " (first event)\n"  << std::endl;
    std::cout << "================================================================" << std::endl;

    //canvasの宣言など...
    TCanvas *c1 = new TCanvas("c1", "Canvas", 1600, 1200);
    fH1ChargeIntegral = new TH1F("fH1ChargeIntegral", Form("%s:ch%d Charge Integral [%.1f,%.1f]", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax), 500, adcMin, adcMax);
    fH1ChargeIntegral->SetXTitle("Voltage sum [V]");
    fH1ChargeIntegral->SetYTitle(Form("[counts] per %.2f V", (adcMax-adcMin)/500));
    gPad->SetGrid();

    //chargeIntegralの計算
    Double_t chargeIntegral;
    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        timeCut_begin = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] - 50;//トリガー時刻から-50 ns遡ってsum
        timeCut_end = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]] + adcTimeRange;//トリガー時刻から+adcTimeRange nsまでsum
        chargeIntegral = GetChargeIntegral(iBoard, iCh, 20, timeCut_begin, timeCut_end);
        
        if (chargeIntegral > -9999.9)
        {
            counter++;
            fH1ChargeIntegral->Fill(-chargeIntegral);
        }
    }
    fH1ChargeIntegral->Draw();
    c1->Update();
    gPad->WaitPrimitive();  // ここでグラフが表示されたまま一時停止

    Int_t flag_std_input = 1;
    Int_t fitIndex;
    std::vector<TF1*> fits;
    std::vector<Double_t> means, sigmas_mean, sigmas_gauss, intercepts, slopes;
    while(flag_std_input == 1){
        Double_t fitLowerBound, fitUpperBound, peakHight, sigma_set;
        std::cout << "fitLowerBound = ";
        std::cin >> fitLowerBound;
        std::cout << std::endl;
        std::cout << "fitUpperBound = ";
        std::cin >>fitUpperBound;
        std::cout << std::endl;
        std::cout << "peakHight = ";
        std::cin >> peakHight;
        std::cout << std::endl;
        std::cout << "set sigma : ";
        std::cin >> sigma_set;
        if(fitLowerBound == 0 && fitUpperBound == 0){
            break;
        }
        Double_t peakPosition = (fitLowerBound + fitUpperBound)/2.0;
        Double_t fitRange = fitUpperBound - fitLowerBound;
        TF1* gaussian_plus_linear = new TF1(Form("gaussian_plus_linear_%d", fitIndex), "gaus+pol1(3)", fitLowerBound, fitUpperBound);
        /*
            [0]*exp(-0.5*((x-[1])/[2])**2) + [3] + [4]*x
        */
        gaussian_plus_linear->SetParLimits(0,0.1*peakHight,5*peakHight);
        gaussian_plus_linear->SetParLimits(1,fitLowerBound,fitUpperBound);
        gaussian_plus_linear->SetParLimits(2,0.1*sigma_set,5*sigma_set);
        gaussian_plus_linear->SetParLimits(4,-1e4,1);
        gaussian_plus_linear->SetParameters(peakHight, peakPosition, sigma_set, 1000.0, -0.01);
        TFitResultPtr fit_result = fH1ChargeIntegral->Fit(gaussian_plus_linear, "RS+"); //TFitResultPtrはフィッティングの結果を保持する型。あとでフィッティングの可否判定に使う。
        Int_t checking = fit_result->Status();
        if(checking != 0){}
        else{
            fits.push_back(gaussian_plus_linear);
            means.push_back(gaussian_plus_linear->GetParameter(1));
            // sigmas.push_back((gaussian->GetParameter(2))/sqrt(2*M_PI*(gaussian->GetParameter(0))*(gaussian->GetParameter(2))));//σ/√N
            sigmas_mean.push_back(gaussian_plus_linear->GetParError(1));//σ_mean
            sigmas_gauss.push_back(gaussian_plus_linear->GetParameter(2));//σ
            intercepts.push_back(gaussian_plus_linear->GetParameter(3));//切片
            slopes.push_back(gaussian_plus_linear->GetParameter(4));//傾き
        }
        
        //ガウシアン、直線、その和を個々でプロットする
        TF1* gauss = new TF1("gauss", "gaus", fitLowerBound, fitUpperBound);
        gauss->SetParameters(
            gaussian_plus_linear->GetParameter(0), gaussian_plus_linear->GetParameter(1), gaussian_plus_linear->GetParameter(2)
        );
        gauss->SetLineColor(kOrange-3);
        gauss->SetLineStyle(1);
        gauss->Draw("LSAME");
        
        TF1* linear = new TF1("linear", "pol1", fitLowerBound, fitUpperBound);
        linear->SetParameters(
            gaussian_plus_linear->GetParameter(3), // 切片
            gaussian_plus_linear->GetParameter(4)  // 傾き
        );
        linear->SetLineColor(kGreen);
        linear->SetLineStyle(1);
        linear->Draw("LSAME");
        c1->Update();

        fitIndex++;
        gPad->WaitPrimitive();  // ここでグラフが表示されたまま一時停止
    }

    c1->Update();

    //結果の図やフィッティングパラメータを保存する。フィッティングパラメータは"./output/GSO_peaksearch_data.txt"に追記して保存する。図は"./figure/"にYYYYMMDDというフォルダを作ってその中に保存する。
    TString filename_figure;
    TString rootFile = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/')); //.rootファイルのフルパスからファイル名だけを抜き出した
    rootFile.ReplaceAll(".", "_"); //.dat.rootのドットを"_"に変えた

    std::ofstream ofs;
    if(append_Option == 1){
        ofs.open("./output/semi-auto_fitting_data.txt", std::ios::app);
    }
    else{
        ofs.open(Form("./output/%s_data.txt",rootFile.Data()));
    }
    
    auto mean_temp = means.begin();
    auto sigma_mean_temp = sigmas_mean.begin();
    auto sigma_gaus_temp = sigmas_gauss.begin();

    if(append_Option == 1){
        ofs << std::endl << "================================================================" << std::endl << ".rootfile || filepath : " << fRootFile.Data() << std::endl;
        auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
        std::tm local_tm = *std::localtime(&now_c);

        ofs << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    }
    ofs << "means, sigmas of means, sigmas of gaussian" << std::endl << std::endl;
    while(mean_temp != means.end() && sigma_mean_temp != sigmas_mean.end() && sigma_gaus_temp != sigmas_gauss.end()){
        ofs << *mean_temp << " " << *sigma_mean_temp << " " << *sigma_gaus_temp << std::endl;
        ++mean_temp;
        ++sigma_mean_temp;
        ++sigma_gaus_temp;
    }
    ofs << std::endl << "numPeak : " << fitIndex << std::endl; // ピークの数
    ofs.close();
    

    //図を保存するフォルダのための日付
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char date[9];
    strftime(date, sizeof(date), "%Y%m%d", ltm); // "YYYYMMDD"形式で日付を取得
    //YYYYMMDDフォルダのパス
    TString folderPath = TString::Format("./figure/%s", date);
    //フォルダが存在しない場合は作成
    if (gSystem->AccessPathName(folderPath)) {
        if (gSystem->mkdir(folderPath, true) != 0) {
            std::cerr << "フォルダの作成に失敗しました: " << folderPath << std::endl;
            return -1;
        }
    }

    filename_figure = Form("%s:ch%d_semi_auto_fitting_%s.pdf", rootFile.Data(), iCh, key_crystal.Data());

    // 既にファイルが存在するか確認
    Int_t index = 1;
    while (gSystem->AccessPathName(folderPath + '/' + filename_figure) == 0) {
        // ファイルが存在する場合、ファイル名にインデックスを追加
        filename_figure = Form("%s:ch%d_semi_auto_fitting_%s_%d.pdf", rootFile.Data(), iCh, key_crystal.Data(),index);
        index++;
    }

    c1->SaveAs(folderPath + '/' + filename_figure);

    return (Double_t)counter;
}

Double_t DRS4Ana::Plot_waveform_8ch(){
    Double_t nentries = fChain->GetEntriesFast();
    Double_t counter = 0.0;

    TCanvas *c1 = new TCanvas("title", "name", 1200, 6000);
    c1->Divide(2,4);
    TH2D* hists[2][4];
    gPad->SetLogz();
    gPad->SetGrid();
    gStyle->SetOptStat(0);
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            hists[iBoard][iCh] = new TH2D(Form("title_ib%d_ic%d", iBoard, iCh), Form("name_ib%d_ic%d", iBoard, iCh), 500, 0, 1500, 500, -0.55, 0.05);
        }
    }
    Double_t discriTime;
    Int_t counters[2][4];
    for(Int_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);
        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                discriTime = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
                if(100 < discriTime && discriTime < 1400){
                    for(Int_t iCell=0; iCell<1024; iCell++){
                        hists[iBoard][iCh]->Fill(fTime[iBoard][iCh][iCell], fWaveform[iBoard][iCh][iCell]);
                        
                    }
                    counters[iBoard][iCh] += 1;
                }
            }
        }
        if(static_cast<Int_t>(counter) % 5000 == 0){
            printf("\tfilled points %d...\n", static_cast<Int_t>(counter));
        }
        counter++;
    }
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            c1->cd(iBoard*4+iCh+1);
            hists[iBoard][iCh]->Draw();
            gPad->SetLogz();
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    TString folderPath = Makedir_Date();
    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    filename_figure += "_allCH_waveforms.pdf";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure.Data()));

    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            printf("\tcounter[%d][%d] : %d\n", iBoard, iCh, counters[iBoard][iCh]);
        }
    }
    return (Double_t)counter;
}

Double_t DRS4Ana::Plot_TriggerTimeDist_8ch(){
    Double_t nentries = fChain->GetEntriesFast();
    Double_t counter = 0.0;

    TCanvas *c1 = new TCanvas("title", "name", 1200, 6000);
    c1->Divide(2,4);
    TH1D* hists[2][4];
    gPad->SetLogz();
    gPad->SetGrid();
    gStyle->SetOptStat(0);
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            hists[iBoard][iCh] = new TH1D(Form("title_ib%d_ic%d", iBoard, iCh), Form("name_ib%d_ic%d", iBoard, iCh), 256, fWaveformXmin, fWaveformXmax);
        }
    }
    for(Int_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);
        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                for(Int_t iCell=0; iCell<1024; iCell++){
                    hists[iBoard][iCh]->Fill(fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]]);
                }
            }
        }
        if(static_cast<Int_t>(counter) % 5000 == 0){
            printf("\tfilled points %d...", static_cast<Int_t>(counter));
        }
        counter++;
    }
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            c1->cd(iBoard*4+iCh+1);
            hists[iBoard][iCh]->Draw();
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    TString folderPath = Makedir_Date();
    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    filename_figure += "_allCH_triggertime.pdf";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure.Data()));

    return (Double_t)counter;
}


Double_t DRS4Ana::PlotSumEnergy_with_cutting(TString key = "0120", Int_t iBoard1, Int_t iCh1, Int_t iBoard2, Int_t iCh2, Double_t xmax)
{
    gStyle->SetOptStat(1); // 統計ボックス表示の有無 1が表示 0が非表示

    // 前のキャンバスが存在する場合、削除する
    TCanvas* existingCanvas = (TCanvas*)gROOT->FindObject("c1");
    if (existingCanvas)
    {
        existingCanvas->Close(); // キャンバスを閉じる
        delete existingCanvas;  // メモリ解放
        existingCanvas = nullptr;
    }

    TCanvas *c1 = new TCanvas("c1", Form("%s:Board%dCh%d+Board%dCh%d SumEnergy", fRootFile.Data(), iBoard1, iCh1+1, iBoard2, iCh2+1), 800, 600);

    if (fH1Energy_PMTs != NULL)
    {
        delete fH1Energy_PMTs;
    }

    Int_t histDiv = 100;
    Double_t Vcut = 20.0;
    Double_t xmin = 0.0;

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;

    fH1Energy_PMTs = new TH1F("fH1Energy_PMTs", Form("%s:Board%dCh%d+Board%dCh%d SumEnergy w/ cutting", fRootFile.Data(), iBoard1+1, iCh1+1, iBoard2+1, iCh2+1), histDiv, xmin, xmax);
    fH1Energy_PMTs->SetXTitle("Sum of Energy [keV]");
    fH1Energy_PMTs->SetYTitle(Form("counts per %.1f keV", (xmax-xmin)/histDiv));

    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    //　エネルギーへの変換に必要なパラメータを取得
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e);

    Double_t discriTime1, discriTime2, discriTime_S1, discriTime_A1;
    Double_t adcSum_timerange1, adcSum_timerange2;

    //　結晶に応じた積分範囲を指定
    if(iBoard1 == 0){
        if(iCh1 == 1){
            adcSum_timerange1 = 180; // master ch2 crystal is GSO
        }
        else if(iCh1 == 0 || iCh1 == 2 || iCh1 == 3){
            adcSum_timerange1 = 600; // master ch1,3,4 crystals are NaI
        }
        else{
            std::cout << "1st crystal is not found" << std::endl;
        }
    }
    else if(iBoard1 == 1){
        adcSum_timerange1 = 180; // slave all ch crystals are GSO
    }
    else{
        std::cout << "1st crystal is not found" << std::endl;
    }

    if(iBoard2 == 0){
        if(iCh2 == 1){
            adcSum_timerange2 = 180; // master ch2 crystal is GSO
        }
        else if(iCh2 == 0 || iCh2 == 2 || iCh2 == 3){
            adcSum_timerange2 = 600; // master ch1,3,4 crystals are NaI
        }
        else{
            std::cout << "2nd crystal is not found" << std::endl;
        }
    }
    else if(iBoard2 == 1){
        adcSum_timerange2 = 180; // slave all ch crystals are GSO
    }
    else{
        std::cout << "2nd crystal is not found" << std::endl;
    }

    Double_t energy_buf1, energy_buf2, energy_buf_S1, energy_buf_A1;
    Double_t lower_limit_buf1, lower_limit_buf2, lower_limit_buf_S1, lower_limit_buf_A1, lower_limit_buf_S1A1;
    Double_t upper_limit_buf1, upper_limit_buf2, upper_limit_buf_S1, upper_limit_buf_A1, upper_limit_buf_S1A1;
    Double_t lower_limit_discri, upper_limit_discri;

    upper_limit_buf1 = 450.0;
    upper_limit_buf_S1 = 300.0;
    upper_limit_discri = 200.0;
    upper_limit_buf_S1A1 = 600.0;

    lower_limit_buf2 = 100.0;
    lower_limit_buf_S1 = 200.0;
    lower_limit_discri = 100.0;
    
    // lower_limit_buf_S1A1 = 400.0;
    // upper_limit_buf_S1A1 = 600.0;

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);

        discriTime1 = fTime[iBoard1][iCh1][fDiscriCell[iBoard1][iCh1]];
        discriTime2 = fTime[iBoard2][iCh2][fDiscriCell[iBoard2][iCh2]];
        discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
        discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];

        // 各チャンネルの Charge Integral を取得
        Double_t chargeIntegral1 = GetChargeIntegral(iBoard1 , iCh1, Vcut, discriTime1 - 50, discriTime1 + adcSum_timerange1);
        Double_t chargeIntegral2 = GetChargeIntegral(iBoard2 , iCh2, Vcut, discriTime2 - 50, discriTime2 + adcSum_timerange2);
        Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, Vcut, discriTime_S1 - 50, discriTime_S1 + 600);
        Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, Vcut, discriTime_A1 - 50, discriTime_A1 + 600);

        // Charge Integralが有効な場合のみ足し合わせる
        if (chargeIntegral1 > -9999.9 && chargeIntegral2 > -9999.9)
        {
            // チャンネルに応じたエネルギーへ変換
            energy_buf1 = p0[iBoard1][iCh1] + p1[iBoard1][iCh1]*(-chargeIntegral1);
            energy_buf2 = p0[iBoard2][iCh2] + p1[iBoard2][iCh2]*(-chargeIntegral2);
            energy_buf_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
            energy_buf_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

            Double_t energy_buf_S1A1 = energy_buf_S1 + energy_buf_A1;

            if (energy_buf1 < upper_limit_buf1 && // kill over 511keV events

                energy_buf2 > lower_limit_buf2 && // kill dark

                energy_buf_S1 > lower_limit_buf_S1 && 
                energy_buf_S1 < upper_limit_buf_S1 &&

                energy_buf_S1A1 < upper_limit_buf_S1A1 &&

                discriTime1 > lower_limit_discri && 
                discriTime1 < upper_limit_discri && 

                discriTime2 > lower_limit_discri && 
                discriTime2 < upper_limit_discri && 

                discriTime_S1 > lower_limit_discri && 
                discriTime_S1 < upper_limit_discri && 

                discriTime_A1 > lower_limit_discri && 
                discriTime_A1 < upper_limit_discri)
            {   
                Double_t sumEnergy = energy_buf1 + energy_buf2;
                fH1Energy_PMTs->Fill(sumEnergy);

                counter++;
            }
            
        }
    }

    fH1Energy_PMTs->Draw();

    

    // ピークに対するフィッティング
    // TF1 *fitFunc1 = new TF1("fitFunc1", "gaus", 500, 520); // 第2ピークに対する範囲
    // fH1SumChargeIntegral->Fit(fitFunc1, "R");
 

    // fH1SumChargeIntegral->Draw();
    // fitFunc1->Draw("same");

    // TString name;
    // name = Form("EnergyHist_with_cutting_Board%dch%d+Board%dch%d.pdf",iBoard1+1, iCh1+1, iBoard2+1, iCh2+1);
    // c1->SaveAs(name);


    return nentries;
}
