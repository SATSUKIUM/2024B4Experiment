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

#include <fstream>
#include <filesystem>
#include <TSystem.h>

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
    Int_t counter = 0;
    for (Int_t i = 0; i < 1024; i++)
    {
        if (fTime[iBoard][iCh][i] >= fPedestalTmin && fTime[iBoard][iCh][i] <= fPedestalTmax)
        {
            counter++;
            pedestalV += fWaveform[iBoard][iCh][i];
        }
    }
    // std::cout << pedestalV/counter << std::endl;
    return 0;
    // return pedestalV / counter;
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

// Double_t DRS4Ana::GetChargeIntegral(Int_t iCh, Double_t Vcut)
// {
//     if (fSignalPolarity == 1)
//     {
//         if (GetMaxVoltage(iCh) <= Vcut)
//         {
//             return -9999.9;
//         }
//     }
//     else
//     {
//         if (GetMinVoltage(iCh) >= Vcut)
//         {
//             return -9999.9;
//         }
//     }

//     Double_t pedestal = GetPedestal(iCh, Vcut);

//     Double_t charge = 0.0;
//     for (Int_t i = 0; i < 1024; i++)
//     {
//         if (fTime[0][iCh][i] >= fChargeIntegralTmin && fTime[0][iCh][i] <= fChargeIntegralTmax)
//         {
//             charge += fWaveform[0][iCh][i] - pedestal;
//         }
//     }
//     return charge;
// }

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
        if(jentry % 1000 == 0){
            // std::cout << fTriggerCell[0] << std::endl;
        }
    }
    // fH1ChargeIntegral->SetMinimum(0);
    // fH1ChargeIntegral->SetMaximum(600);
    fH1ChargeIntegral->Draw();
    TString filename_figure;
    filename_figure = Form("%s:ch%d_Charge_Integral_[%.1f,%.1f].pdf", fRootFile.Data(), iCh, fChargeIntegralTmin, fChargeIntegralTmax);
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

Double_t DRS4Ana::automated_peaksearch(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax, Int_t numPeaks)
{
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

        // Int_t iBoard = 0; //今はとりあえずiBoardをここで宣言したが、ゆくゆくはautomaeted_peaksearchの引数にiBoard入れておきたい。←しました。

        Double_t chargeIntegral = GetChargeIntegral(iBoard, iCh, Vcut, Tmax_for_fH1CI-10, Tmax_for_fH1CI+300); //電圧の和を取る時間の範囲を最後２つの変数に書いてる
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
    Double_t fitrange = 2.0;
    for(int i=0; i<foundPeaks; ++i){
        TF1* gaussian = new TF1(Form("gaussian_%d",i), "gaus", peakPositions[i]-fitrange, peakPositions[i]+fitrange); //要調整。特に範囲
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

    std::ofstream ofs(Form("./output/%s_data.txt",rootFile.Data()));
    auto mean_temp = means.begin();
    auto sigma_mean_temp = sigmas_mean.begin();
    auto sigma_gaus_temp = sigmas_gaus.begin();
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
    ofs << "fitrange : " << fitrange << std::endl; // ピーク中心からの範囲
    ofs.close();
    

    filename_figure = Form("./figure/%s:ch%d_automated_peaksearch.pdf", rootFile.Data(), iCh);
    c1->SaveAs(filename_figure);

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
    fH1TriggerRate = new TH1F("fH1TriggerRate", Form("%s:ch%d_Trigger_Rate", fRootFile.Data(), iCh), howLong_DAQ_spent, eventTime_begin, eventTime_end);
    fH1TriggerRate->SetXTitle("time [s]");
    fH1TriggerRate->SetYTitle("[counts]");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        fChain->GetEntry(jentry);
        fH1TriggerRate->Fill(fEventTimeInSec+fEventTimeInNanoSec*10e-9);
        counter++;
    }
    fH1TriggerRate->Draw();
    return(counter);
}

Double_t DRS4Ana::Overlay_PlotWaves(Int_t iCh = 0){
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
    gPad->SetLogz();
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


Double_t DRS4Ana::Plot_scatter_energy_btwn_PMTs(Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 800, 600);
    TGraphErrors *graph = new TGraphErrors(nentries);
    graph->Draw();
    graph->SetTitle(Form("scatter plot : energy between two PMTs;Board%d CH%d energy (keV);Board%d CH%d energy (keV)", x_iBoard, x_iCh, y_iBoard, y_iCh));
    graph->SetMarkerStyle(21);
    graph->SetMarkerSize(0.3);
    // graph->GetXaxis()->SetLimits(0, 1500); // X軸の範囲を設定
    // graph->GetYaxis()->SetRangeUser(0, 1500); // Y軸の範囲を設定
    
    gPad->SetGrid();

    Double_t x_p0, x_p1, y_p0, y_p1; //fitting parameter
    Double_t x_p0_e, x_p1_e, y_p0_e ,y_p1_e; //error of the fitting parameter
    x_p0 = 0.0;
    x_p1 = 1.0;
    y_p0 = 0.0;
    y_p1 = 1.0;
    // x_p0_e = 1.0; //no need to use
    x_p1_e = 1.0;
    // y_p0_e = 1.0; //no need to use
    y_p1_e = 1.0;

    if(x_iBoard == 0 && x_iCh == 0){
        printf("\n\t[Message]: template used\n");
        //for huruno_1 for HV 1350 V
        x_p0 = -52.05;
        x_p1 = 7.161;
        // x_p0_e;
        x_p1_e = 0.0006377;
    }
    if(y_iBoard == 0 && y_iCh == 2){
        printf("\n\t[Message]: template used\n");
        //for sato_NaI for HV 1300 V
        y_p0 = -39.94;
        y_p1 = 17.62;
        // x_p0_e;
        y_p1_e = 0.0009443;
    }
    
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    for(Int_t pt_index=0; pt_index<nentries; pt_index++){
        fChain->GetEntry(pt_index);

        x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, 0, 1020);
        y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, 0, 1020);//下で何回か使うので、そのたびにchargeIntの計算をしたくない。

        x_energy = x_p0 + x_p1*x_charge_buf;
        y_energy = y_p0 + y_p1*y_charge_buf;
        x_error = x_p1_e*x_charge_buf;
        y_error = y_p1_e*y_charge_buf;

        graph->SetPoint(pt_index, x_energy, y_energy);
        graph->SetPointError(pt_index, x_error, y_error);
        
        // canvas->Update();

        if(pt_index % 500 == 0){
            printf("\tPoint plot : %d\n", pt_index);
        }
        counter++;
        
    }
    graph->GetXaxis()->SetLimits(0, 1500); // X軸の範囲を設定
    graph->GetYaxis()->SetRangeUser(0, 1500); // Y軸の範囲を設定
    graph->Draw("AP");
    canvas->Update();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    printf("\n\tfigure saved as: %s\n", filename_figure.Data());
    // canvas->SaveAs(Form("../figure/%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/%s.pdf", filename_figure.Data()));
    
    return counter;
}

Double_t DRS4Ana::Plot_2Dhist_energy_btwn_PMTs(Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 800, 600);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    fH2Energy_PMTs = new TH2F("name", "title", 200, 0, 600, 200, 0, 600);
    fH2Energy_PMTs->SetTitle(Form("2D hist : energy between two PMTs;Board%d CH%d energy (keV);Board%d CH%d energy (keV)", x_iBoard, x_iCh, y_iBoard, y_iCh));
    fH2Energy_PMTs->Draw();

    
    
    gPad->SetGrid();
    gPad->SetLogz();
    // gStyle->SetPalette(kInvertedDarkBodyRadiator);

    Double_t x_p0, x_p1, y_p0, y_p1; //fitting parameter
    Double_t x_p0_e, x_p1_e, y_p0_e ,y_p1_e; //error of the fitting parameter
    x_p0 = 0.0;
    x_p1 = 1.0;
    y_p0 = 0.0;
    y_p1 = 1.0;
    // x_p0_e = 1.0; //no need to use
    x_p1_e = 1.0;
    // y_p0_e = 1.0; //no need to use
    y_p1_e = 1.0;

    if(x_iBoard == 0 && x_iCh == 0){
        printf("\n\t[Message]: template used\n");
        //for huruno_1 for HV 1350 V
        x_p0 = -52.05;
        x_p1 = 7.161;
        // x_p0_e;
        x_p1_e = 0.0006377;
    }
    if(y_iBoard == 0 && y_iCh == 2){
        printf("\n\t[Message]: template used\n");
        //for sato_NaI for HV 1300 V
        y_p0 = -39.94;
        y_p1 = 17.62;
        // x_p0_e;
        y_p1_e = 0.0009443;
    }
    
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, 0, 1020);
        y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, 0, 1020);

        x_energy = x_p0 + x_p1*x_charge_buf;
        y_energy = y_p0 + y_p1*y_charge_buf;

        fH2Energy_PMTs->Fill(x_energy, y_energy);


        if(Entry % 500 == 0){
            printf("\tPoint plot : %d\n", Entry);
        }
        counter++;
        
    }
    fH2Energy_PMTs->Draw();


    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kBlack);
    line->SetLineWidth(2);
    line->Draw("SAME");


    canvas->Update();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    printf("\n\tfigure saved as: %s\n", filename_figure.Data());
    // canvas->SaveAs(Form("../figure/%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/fH2Energy_PMTs_%s.png", filename_figure.Data()));
    canvas->SaveAs(Form("./figure/fH2Energy_PMTs_%s.pdf", filename_figure.Data()));
    
    return counter;
}