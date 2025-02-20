/*
DRS4Ana version 0.3
TChainで複数のRunを解析できるようにした
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

TChain *globalChain_Event = new TChain("treeDRS4BoardEvent");
TChain *globalChain_Info = new TChain("treeDRS4BoardInfo");


std::vector<TString> fRootFile_pars;



#define DRS4Ana_cxx
#include "DRS4Ana.h"
TString DRS4Ana::fRootFile = "";
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

void addGlobalChain(const TString fRootFile_par){
    if(globalChain_Event && globalChain_Info){
        globalChain_Event->Add(fRootFile_par);
        globalChain_Info->Add(fRootFile_par);
        fRootFile_pars.push_back(fRootFile_par);
        DRS4Ana::fRootFile += "_" + fRootFile_par;
    }
}
void listChains(const TString key = "456"){
    TString key_path = "./cfg/chains/" + key + "/data.txt";
    std::ifstream ifs(key_path);
    TString fRootFile_par;
    while(ifs >> fRootFile_par){
        addGlobalChain(fRootFile_par);
        printf("%s add to globalChain\n", fRootFile_par.Data());
    }
    ifs.close();
    printf("globalChain : %d TTrees\n", globalChain_Event->GetNtrees());
    globalChain_Event->GetListOfFiles()->Print();
    for(Int_t it=0; it<fRootFile_pars.size(); it++){
        TString fRootFile_element = TString(fRootFile_pars[it]);
        DRS4Ana::fRootFile += fRootFile_element;
        DRS4Ana::fRootFile += "_";
    }
}

Double_t error_curve_upper(Double_t* x, Double_t* par){
    // std::cout << 511.0 - x[0] + 0.01*(par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))) + par[1] * sqrt(abs(x[0])) / (2.0*sqrt(2.0*log(2.0)))) << std::endl;
    return(511.0 - x[0] + 0.01*(par[2] * par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))) + par[3] * par[1] * sqrt(abs(x[0])) / (2.0*sqrt(2.0*log(2.0))))); //p0_res[0]はyのp0_res、p0_res[1]はxのp0_res
}
Double_t error_curve_lower(Double_t* x, Double_t* par){
    // std::cout << 511.0 - x[0] - 0.01*(par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))) + par[1] * sqrt(abs(x[0])) / (2.0*sqrt(2.0*log(2.0)))) << std::endl;
    return(511.0 - x[0] - 0.01*(par[2] * par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))) + par[3] * par[1] * sqrt(abs(x[0])) / (2.0*sqrt(2.0*log(2.0))))); //par[0]はyのp0_res、par[1]はxのp0_res
}

// Double_t error_curve_upper3(Double_t* x, Double_t* par){
//     // std::cout << 511.0 - x[0] + 0.01*(par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))) + par[1] * sqrt(abs(x[0])) / (2.0*sqrt(2.0*log(2.0)))) << std::endl;
//     return(511.0 - x[0] + par[1]*0.01*(par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))))); //p0_res[0]はyのp0_res、p0_res[1]はxのp0_res
// }
// Double_t error_curve_lower3(Double_t* x, Double_t* par){
//     // std::cout << 511.0 - x[0] - 0.01*(par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))) + par[1] * sqrt(abs(x[0])) / (2.0*sqrt(2.0*log(2.0)))) << std::endl;
//     return(511.0 - x[0] - par[1]*0.01*(par[0] * sqrt(abs(511.0 - x[0])) / (2.0*sqrt(2.0*log(2.0))))); //par[0]はyのp0_res、par[1]はxのp0_res
// }

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
    Int_t lastDotPos = fileName.Last('.');
    TString filename_buf_before_dot = fileName(0, lastDotPos);
    TString filename_buf_after_dot = fileName(lastDotPos, fileName.Length());
    TString filename_buf = fileName;
    while(gSystem->AccessPathName(folderPath + '/' + filename_buf) == 0){
        filename_buf = filename_buf_before_dot + TString::Format("%d", index) + filename_buf_after_dot;
        index++;
        std::cout << Form("\tfilename : %s exists, rename...", filename_buf.Data()) << std::endl;
    }
    fileName = filename_buf;
    return index;
}

void DRS4Ana::Load_EnergycalbData(TString key, Double_t p0[2][4], Double_t p0e[2][4], Double_t p1[2][4], Double_t p1e[2][4], Double_t p0_res[2][4], Double_t p0e_res[2][4]){
    Double_t p0_buf, p1_buf, p0e_buf, p1e_buf, p0_res_buf, p0e_res_buf;
    TString calb_data_filepath = Form("./cfg/%s/data.txt", key.Data());
    std::ifstream ifs(calb_data_filepath);
    Int_t line_index = 0;

    while(ifs >> p0_buf >> p0e_buf >> p1_buf >> p1e_buf){
        
        if(line_index < 4){
            p0[0][line_index] = p0_buf;
            p0e[0][line_index] = p0e_buf;
            p1[0][line_index] = p1_buf;
            p1e[0][line_index] = p1e_buf;
            std::cout << Form("\tiBoard : 0, iCh : %d, line_inedex %d \n", line_index % 4, line_index);
            //printf("\tline_index %d\n",line_index);
            //std::cout << Form("\tiBoard : 0, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            //std::cout << Form("\t\t%lf %lf %lf %lf\n", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        else if(line_index < 8){
            p0[1][line_index-4] = p0_buf;
            p0e[1][line_index-4] = p0e_buf;
            p1[1][line_index-4] = p1_buf;
            p1e[1][line_index-4] = p1e_buf;
            std::cout << Form("\tiBoard : 1, iCh : %d, line_index %d\n", line_index % 4, line_index);
            //printf("\tline_index %d\n",line_index);
            //std::cout << Form("\tiBoard : 1, iCh : %d || energy calibration data loaded.\n", line_index % 4);
            //std::cout << Form("\t\t%lf %lf %lf %lf\n", p0_buf, p1_buf, p0e_buf, p1e_buf);
        }
        
        line_index++;
        if(line_index == 8){
            break;
        }
    }

    while(ifs >> p0_res_buf >> p0e_res_buf){
        if(line_index == 16){
            break;
        }
        if(line_index < 12){
            p0_res[0][line_index-8] = p0_res_buf;
            p0e_res[0][line_index-8] = p0e_res_buf;
            std::cout << Form("\tiBoard : 0, iCh : %d, line_index %d \n", line_index % 4, line_index);
            //std::cout << Form("\tiBoard : 0, iCh : %d || energy resolution data loaded.\n", line_index % 4);
            //printf("\tline_index %d\n",line_index);
            //std::cout << Form("\t\t%lf %lf\n", p0_res_buf, p0e_res_buf);
        }
        else if(line_index < 16){
            p0_res[1][line_index-12] = p0_res_buf;
            p0e_res[1][line_index-12] = p0e_res_buf;
            std::cout << Form("\tiBoard : 1, iCh : %d, line_index %d \n", line_index % 4, line_index);
            //std::cout << Form("\tiBoard : 1, iCh : %d || energy resolution data loaded.\n", line_index % 4);
            //printf("\tline_index %d\n",line_index);
            //std::cout << Form("\t\t%lf %lf\n", p0_res_buf, p0e_res_buf);
        }
        line_index++;
    }

    // while (ifs) {
    //     if (line_index < 8) {  // calib data
    //        if (!(ifs >> p0_buf >> p0e_buf >> p1_buf >> p1e_buf)) break;
    //         int Board = line_index / 4;
    //         int ch = line_index % 4;
    //         p0[Board][ch] = p0_buf;
    //         p0e[Board][ch] = p0e_buf;
    //         p1[Board][ch] = p1_buf;
    //         p1e[Board][ch] = p1e_buf;
    //         std::cout << Form("iBoard: %d, iCh: %d, line_index %d\n", Board, ch, line_index);
    //         std::cout << Form("\t%lf %lf %lf %lf\n", p0_buf, p0e_buf, p1_buf, p1e_buf);
    //     } 
    //     else if (line_index < 16) {  // resolution data
    //        if (!(ifs >> p0_res_buf >> p0e_res_buf)) break;
    //         int Board = (line_index - 8) / 4;
    //         int ch = (line_index - 8) % 4;
    //         p0_res[Board][ch] = p0_res_buf;
    //         p0e_res[Board][ch] = p0e_res_buf;
    //         std::cout << Form("iBoard: %d, iCh: %d, line_index %d\n", Board, ch, line_index);
    //         std::cout << Form("\t%lf %lf\n", p0_res_buf, p0e_res_buf);
    //     }
    //     line_index++;
    // }
    ifs.close();

    for(Int_t iBoard=0; iBoard<2; iBoard++)
    {
        for(Int_t iCh=0; iCh<4; iCh++)
        {
            printf("\tp0 p0e p1 p1e : %f %f %f %f\n", p0[iBoard][iCh], p0e[iBoard][iCh], p1[iBoard][iCh], p1e[iBoard][iCh]);
        }
    }
    for(Int_t iBoard=0; iBoard<2; iBoard++)
    {
        for(Int_t iCh=0; iCh<4; iCh++)
        {
            printf("\tp0_res p0e_res : %f %f\n", p0_res[iBoard][iCh], p0e_res[iBoard][iCh]);
        }
    }
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

// Double_t DRS4Ana::GetPedestalMean(Int_t iBoard, Int_t iCh, Double_t Vcut)
// {
//     Long64_t nentries = fChain->GetEntriesFast();
//     Long64_t counter = 0;
//     Double_t pedMean = 0.0;
//     for (Long64_t jentry = 0; jentry < nentries; jentry++)
//     {
//         fChain->GetEntry(jentry);
//         Double_t ped = GetPedestal(iBoard, iCh, Vcut);
//         if (ped > -9999.9)
//         {
//             counter++;
//             pedMean += ped;
//         }
//     }
//     return pedMean / counter;
// }

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
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);
    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
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

Double_t DRS4Ana::Overlay_PlotWaves_discri(Int_t iBoard = 0, Int_t iCh = 0, Double_t threshold = 0.10){
    Long64_t nentries = fChain->GetEntriesFast();
    std::cout << "nentries: " << nentries << std::endl;

    gStyle->SetOptStat(0);
    
    if(fH2Overlay_Waves != NULL){
        delete fH2Overlay_Waves;
    }
    fH2Overlay_Waves = new TH2F();

    //ビンなどは適宜変える
    fChain->Draw(Form("waveform[%d][%d]:%f*Iteration$>>fH2Overlay_Waves(300, 0, %f, 300, -0.55, 0.05)",iBoard, iCh, fTimeBinWidthInNanoSec, fWaveformXmax), "", "colz", nentries, 0); //Draw(expression, selection, option, nentries, nfirstentry)

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

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);

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

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);

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

    TF1* gaussian_plus_linear = new TF1("gaussian_plus_linear", "gaus+pol1(3)", 440, 580);
    gaussian_plus_linear->SetParameters(7000, 500, 1.0, 50.0, -5.0);
    fH1ChargeIntegral -> Fit(gaussian_plus_linear, "R");
    gaussian_plus_linear -> Draw("same");

    // TF1* gauss1 = new TF1("gauss1", "gaus", 440, 580);
    // gauss1->SetParameters(
    //     gaussian_plus_linear->GetParameter(0), // 振幅
    //     gaussian_plus_linear->GetParameter(1), // 中心
    //     gaussian_plus_linear->GetParameter(2)  // 幅
    // );
    // gauss1->SetLineColor(kOrange+7);
    // gauss1->SetLineStyle(1);
    // gauss1->Draw("LSAME");

    TF1* linear = new TF1("linear", "pol1", 440, 580);
    linear->SetParameters(
        gaussian_plus_linear->GetParameter(3), // 切片
        gaussian_plus_linear->GetParameter(4)  // 傾き
    );
    linear->SetLineColor(kGreen+1);
    linear->SetLineStyle(1);
    linear->Draw("same");



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

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);

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

    // フィッティング
    TFitResultPtr fit_result = fH1ChargeIntegral->Fit(gaussian_plus_linear, "RS+"); // オプション "RS+" を使用
    std::cout << "debug" << std::endl;
    Int_t checking = fit_result->Status();



    double chi2 = gaussian_plus_linear -> GetChisquare();  // χ²
    int ndof = gaussian_plus_linear -> GetNDF();           // 自由度
    double chi2_ndof = (ndof > 0) ? chi2 / ndof : 0; // 0除算回避
    double prob = TMath::Prob(chi2, ndof);




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
        chi2_ndof_vec.push_back(chi2_ndof);
        prob_vec.push_back(prob);
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
        gaussian_plus_linear->SetParLimits(3,0,1e8);
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
        if(fDiscriCell[0][2] < 133)
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

    TCanvas *c1 = new TCanvas("title", "name", 2400, 12000);
    c1->Divide(2,4);
    TH1D* hists[2][4];
    TF1* fits[2][4];
    gPad->SetLogz();
    gPad->SetGrid();
    gStyle->SetOptStat(0);
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            hists[iBoard][iCh] = new TH1D(Form("title_ib%d_ic%d", iBoard, iCh), Form("name_ib%d_ic%d", iBoard, iCh), 512, fWaveformXmin, fWaveformXmax);
            fits[iBoard][iCh] = new TF1("fit%d %d", "gaus", 150, 220);
        }
    }
    Double_t trig_time_buf;
    for(Int_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);
        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                for(Int_t iCell=0; iCell<1024; iCell++){
                    trig_time_buf = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
                    if(trig_time_buf < 1400 && trig_time_buf > 10)
                    hists[iBoard][iCh]->Fill(trig_time_buf);
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
            hists[iBoard][iCh]->Fit("gaus", "", "", 100, 200);
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    TString folderPath = Makedir_Date();
    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + "_allCH_triggertime.pdf";
    TString filename_figure_png = filename_figure + "_allCH_triggertime.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));
    IfFile_duplication(folderPath, filename_figure_png);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    return (Double_t)counter;
}


Double_t DRS4Ana::PlotSumEnergy_with_cutting(TString key_energy_calib = "0120", Int_t cutting_option, Int_t iBoard1, Int_t iCh1, Int_t iBoard2, Int_t iCh2, Double_t xmax)
{
    // cutting_option == 0 カットなし
    // cutting_option == 1 トリガー時間カット
    // cutting_option == 2 エネルギーカット

    gStyle->SetOptStat(1); // 統計ボックス表示の有無 1が表示 0が非表示

    // 前のキャンバスが存在する場合、削除する
    TCanvas* existingCanvas = (TCanvas*)gROOT->FindObject("c1");
    if (existingCanvas)
    {
        existingCanvas->Close(); // キャンバスを閉じる
        delete existingCanvas;  // メモリ解放
        existingCanvas = nullptr;
    }

    TCanvas *c1 = new TCanvas("c1", Form("%s:Board%dCh%d+Board%dCh%d SumEnergy", fRootFile.Data(), iBoard1+1, iCh1+1, iBoard2+1, iCh2+1), 800, 600);

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
    // エネルギー分解能の1/sqrt(E)フィッティングパラメータ取得
    Double_t p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key_energy_calib, p0, p0e, p1, p1e, p0_res, p0e_res);

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

    upper_limit_buf1 = 350.0;
    upper_limit_buf_S1 = 300.0;
    upper_limit_discri = 200.0;
    upper_limit_buf_S1A1 = 600.0;

    lower_limit_buf1 = 100.0;
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
            Double_t sumEnergy = energy_buf1 + energy_buf2;

            if (cutting_option == 0){
                fH1Energy_PMTs->Fill(sumEnergy);
                counter++;
            }

            else if(cutting_option == 1){
                if (
                discriTime1 > lower_limit_discri && 
                discriTime1 < upper_limit_discri && 

                discriTime2 > lower_limit_discri && 
                discriTime2 < upper_limit_discri && 

                discriTime_S1 > lower_limit_discri && 
                discriTime_S1 < upper_limit_discri && 

                discriTime_A1 > lower_limit_discri && 
                discriTime_A1 < upper_limit_discri)
                {
                fH1Energy_PMTs->Fill(sumEnergy);
                counter++;
            }
            }
            else if(cutting_option == 2){
                if (
                energy_buf1 > lower_limit_buf1 && 
                energy_buf1 < upper_limit_buf1 &&

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
                fH1Energy_PMTs->Fill(sumEnergy);
                counter++;
            }
            }
            
            
        }
    }

    fH1Energy_PMTs->Draw();
    std::cout << counter << std::endl;

    

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


Double_t DRS4Ana::Plot_2Dhist_energy_with_cut(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 1200);
    canvas->Divide(2,2);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];





    Double_t distance_from_511_line, x_distance_btwn_2points, y_distance_btwn_2points;
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

        //DiscriTime_x = 400;
        //DiscriTime_y = 400;
        //std::cout << DiscriTime_x << " " <<  DiscriTime_y << std::endl;


        if(100 < DiscriTime_y && DiscriTime_y < 1400){
            x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
            y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

            x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
            y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
            x_error = 3 * 0.01 * x_p0_res_buf*sqrt(x_energy)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。
            y_error = 3 * 0.01 * y_p0_res_buf*sqrt(y_energy)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。
            distance_from_511_line = pow((x_energy + y_energy - 511.0),2.0) / 2.0;
            x_distance_btwn_2points = pow((511.0+x_energy-y_energy)/2.0 - (511.0-y_energy), 2.0) + pow((511.0-x_energy+y_energy)/2.0 - y_energy, 2.0);
            y_distance_btwn_2points = pow((511.0+x_energy-y_energy)/2.0 - x_energy, 2.0) + pow((511.0-x_energy+y_energy)/2.0 - (511.0-x_energy), 2.0);
            
            if((pow(x_error,2.0) > (distance_from_511_line + x_distance_btwn_2points)) && (pow(y_error,2.0) > (distance_from_511_line + y_distance_btwn_2points))){
                if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                    discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
                    discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];
                    Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, 20.0, discriTime_S1 - 50, discriTime_S1 + 600);
                    Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, 20.0, discriTime_A1 - 50, discriTime_A1 + 600);

                    Double_t energy_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
                    Double_t energy_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

                    Double_t energy_error_S1 = 3 * 0.01 * p0_res[S1_BoardID][S1_ChID]*sqrt(energy_S1)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。
                    Double_t energy_error_A1 = 3 * 0.01 * p0_res[A1_BoardID][A1_ChID]*sqrt(energy_A1)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。

                    distance_from_511_line = pow((energy_S1 + energy_A1 - 511.0),2.0) / 2.0;
                    x_distance_btwn_2points = pow((511.0+energy_S1-energy_A1)/2.0 - (511.0-energy_A1), 2.0) + pow((511.0-energy_S1+energy_A1)/2.0 - energy_A1, 2.0);
                    y_distance_btwn_2points = pow((511.0+energy_S1-energy_A1)/2.0 - energy_S1, 2.0) + pow((511.0-energy_S1+energy_A1)/2.0 - (511.0-energy_S1), 2.0);

                    if((pow(energy_error_S1,2.0) > (distance_from_511_line + x_distance_btwn_2points)) && (pow(energy_error_A1,2.0) > (distance_from_511_line + y_distance_btwn_2points))){
                        fH2Energy_PMTs->Fill(x_energy, y_energy);
                        fH1EnergySpectra[0]->Fill(x_energy);
                        fH1EnergySpectra[1]->Fill(y_energy);
                        fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                        counter++;
                    }
                }
            }
        }

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();
    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[2]->Draw();

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



// void PrecomputeErrors(Double_t *par, vector<Double_t>& x, vector<Double_t>& x_sigma, vector<Double_t>& y_sigma) {
//     // y = -x + 511 
//     for (Double_t x_val = 0; x_val <= 511; x_val += 0.05) {
//         Double_t x_err = par[0] * par[2] * sqrt(x_val) * 0.01 / (2 * sqrt(2 * log(2)));
//         Double_t y_err = par[1] * par[2] * sqrt(abs(511 - x_val)) * 0.01 / (2 * sqrt(2 * log(2)));
        
//         x.push_back(x_val);
//         x_sigma.push_back(x_err);
//         y_sigma.push_back(y_err); 
//     }
// }

// Double_t GetCurveUpper_y(Double_t x_energy, Double_t *par, vector<Double_t>& x, vector<Double_t>& x_sigma, vector<Double_t>& y_sigma){
   
//     Double_t min_diff = 1000;
//     size_t index = 0;

//     for (size_t i = 1; i < x.size(); i++) {
//         Double_t diff = abs(x[i] + x_sigma[i] - x_energy);
//         if (diff < min_diff) {
//             x_energy = x[i] + x_sigma[i];
//             min_diff = diff;
//             index = i;
//         }
//     }

//     return - x_energy + ( 2 * x_sigma[index] + y_sigma[index] ) + 511;
// }

// Double_t GetCurveLower_y(Double_t x_energy, Double_t *par, vector<Double_t>& x, vector<Double_t>& x_sigma, vector<Double_t>& y_sigma){

   

//     Double_t min_diff = 100;
//     size_t index = 0;

//     for (size_t i = 1; i < x.size(); i++) {
//         Double_t diff = abs(x[i] - x_sigma[i] - x_energy);
//         if (diff < min_diff) {
//             x_energy = x[i] - x_sigma[i];
//             min_diff = diff;
//             index = i;
//         }
//     }

//     return - x_energy - ( 2 * x_sigma[index] + y_sigma[index] ) + 511;
// }
          
Double_t CurveUpper_y(Double_t *x, Double_t *par){
        Double_t x_error = par[0] *  par[2] * sqrt(x[0]) * 0.01 / (2 * sqrt(2 * log(2)));
        Double_t y_error = par[1] *  par[2] * sqrt(abs(511 - x[0])) * 0.01 / (2 * sqrt(2 * log(2)));

             //return (- x_energy + sigma * sqrt((pow(x_error,2) + pow(y_error,2))) ) + 511 ;
        return - x[0] + y_error + 511 ;
    }

Double_t CurveLower_y(Double_t *x, Double_t *par){
        Double_t x_error = par[0] *  par[2] * sqrt(x[0]) * 0.01 / (2 * sqrt(2 * log(2)));
        Double_t y_error = par[1] *  par[2] * sqrt(abs(511 - x[0])) * 0.01 / (2 * sqrt(2 * log(2)));

    //  return  (- x_energy - sigma * sqrt((pow(x_error,2) + pow(y_error,2))) )  + 511;
        return  - x[0] - y_error + 511;
    }



Double_t DRS4Ana::EventSelection(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut = true, Double_t sigma = 3){
    
    fChain->SetBranchStatus("fSec",0);
    fChain->SetBranchStatus("fNanoSec",0);
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);
    
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t allcounter = 0;
    Long64_t validcounter = 0;
    Long64_t timecutcounter = 0;
    Long64_t scattercutcounter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 4000);
    canvas->Divide(2,4);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;

    TString key_Crystal_y;

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Double_t x_DiscriTime, y_DiscriTime, S1_DiscriTime, A1_DiscriTime;
    Double_t x_DiscriCell, y_DiscriCell, S1_DiscriCell , A1_DiscriCell;
    Double_t x_adcSum_timerange, y_adcSum_timerange;

    Double_t x_charge_buf, y_charge_buf, S1_charge_buf, A1_charge_buf;
    Double_t x_energy, y_energy, S1_energy, A1_energy;
    Double_t S1_error, x_error_upper, x_error_lower;
    
    Double_t y_upper, y_lower, A1_upper, A1_lower;

    
    Double_t x_p0_res = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res = p0_res[y_iBoard][y_iCh];
    Double_t S1_p0_res = p0_res[0][0];
    Double_t A1_p0_res = p0_res[0][2];

    Double_t S1_energy_vec[1], x_energy_vec[1];
    Double_t par_S1[3] = {S1_p0_res, A1_p0_res, sigma};
    Double_t par_x[3] = {x_p0_res, y_p0_res, sigma};

    Int_t TimeCut, ScatterCut, EnergyCut, AllCut;
    bool cut0,cut1,cut2,cut3,cut4,cut5,cut6,cut7;
    /*
    cut0 : all DC > 0
    cut1 : S1, A1, simultaneously
    cut2 : S1, S1, simultaneously
    cut3 : S2, A2_any deg, simultaneously
    cut4 : 256 - 3sigma < E_S1 < 256 + 3sigma(これもっと強くして良いんじゃないか)
    cut5 : 170 - 3sigma < E_S2 < 340 + 3sigma(これももっと強くして良いんじゃないか)
    cut6 : 511 - 3(sqrt(sigma^2 + sigma^2)) < E_S2 + E_A2 < ::
    cut7 : s1,a1 energy
    */
    bool TimeCutPassed, ScatterCutPassed, EnergyCutPassed;


    x_adcSum_timerange = 600;
   
    if ((y_iBoard == 0 && y_iCh == 1) || (y_iBoard == 1)) {
    y_adcSum_timerange = 180;
    key_Crystal_y = "GSO";
    }
    else if (y_iBoard == 0 && y_iCh == 2) {
    y_adcSum_timerange = 600; 
    key_Crystal_y = "NaI";
    }
    else{
        printf("\t\ny axis || type of crystal is invalid\n");
    }


    TH1D *fH1EnergySpectra[3];

    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI", x_iBoard, x_iCh), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, (maxEnergy-minEnergy)/nBins));
    
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    
    
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", 100, 0, 600);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh,(maxEnergy-minEnergy)/nBins));

    
    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d Ch%d energy [keV];Board%d CH%d energy [keV]", key.Data(), x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);
    

   
    
    // PrecomputeErrors(par_S1, S1, S1_error, A1_error);
    // PrecomputeErrors(par_x, x, x_error, y_error);
    
    TF1 *curve_upper = new TF1("curve_upper", CurveUpper_y, 0.0, 511.0, 3);  // パラメータ数は 3
    TF1 *curve_lower = new TF1("curve_lower", CurveLower_y, 0.0, 511.0, 3); 


    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        x_DiscriTime = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        y_DiscriTime = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        S1_DiscriTime = fTime[0][0][fDiscriCell[0][0]];
        A1_DiscriTime = fTime[0][2][fDiscriCell[0][2]];
        
        x_DiscriCell = fDiscriCell[x_iBoard][x_iCh];
        y_DiscriCell = fDiscriCell[y_iBoard][y_iCh];
        S1_DiscriCell = fDiscriCell[0][0];
        A1_DiscriCell = fDiscriCell[0][2];
        
        x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, x_DiscriTime - 50, x_DiscriTime + x_adcSum_timerange);
        y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, y_DiscriTime - 50, y_DiscriTime + y_adcSum_timerange);
        S1_charge_buf = -GetChargeIntegral(0, 0, 20, S1_DiscriTime - 50, S1_DiscriTime + 600);
        A1_charge_buf = -GetChargeIntegral(0, 2, 20, A1_DiscriTime - 50, A1_DiscriTime + 600);

        x_energy = p0[x_iBoard][x_iCh] + p1[x_iBoard][x_iCh] * x_charge_buf;
        y_energy = p0[y_iBoard][y_iCh] + p1[y_iBoard][y_iCh] * y_charge_buf;
        S1_energy = p0[0][0] + p1[0][0] * S1_charge_buf;
        A1_energy = p0[0][2] + p1[0][2] * A1_charge_buf;

        S1_energy_vec[0] = {S1_energy};
        x_energy_vec[0] =  {x_energy};

        S1_error = p0_res[0][0] * sqrt(256) * 0.01 / (sigma * sqrt(2 * log(2))); //90°散乱カット用のエラー
        // A1_error = p0_res[0][2] * sqrt(A1_energy) * 0.01 / (sigma * sqrt(2 * log(2))); 
        // y_error = p0_res[y_iBoard][y_iCh] * sqrt(y_energy) * 0.01 / (sigma * sqrt(2 * log(2))); 
   
        x_error_upper = x_p0_res * sqrt(256) * 0.01 / (sigma * sqrt(2 * log(2))); //散乱カット用のエラー
        x_error_lower = x_p0_res * sqrt(170) * 0.01 / (sigma * sqrt(2 * log(2)));

       
        //A1_upper = GetCurveUpper_y(A1_energy, par_S1);
        //A1_lower = GetCurveLower_y(A1_energy, par_S1);
        A1_upper = CurveUpper_y(S1_energy_vec, par_S1);
        A1_lower = CurveLower_y(S1_energy_vec, par_S1);
        
        y_upper = CurveUpper_y(x_energy_vec, par_x);
        y_lower = CurveLower_y(x_energy_vec, par_x);


    // Time Cut
        cut0 =  (0 < S1_DiscriCell) && (0 < A1_DiscriCell) && (0 < x_DiscriCell) && (0 < y_DiscriCell);
        cut1 =  (-17 <= S1_DiscriCell - A1_DiscriCell) && (S1_DiscriCell - A1_DiscriCell <= 22);
        cut2 =  (-22 <= S1_DiscriCell - x_DiscriCell) && (S1_DiscriCell - x_DiscriCell <= 15);

        cut3 = false;
        if (y_iBoard == 0){
            cut3 =  (-2 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 22);
        }else if (y_iBoard == 1 && y_iCh == 0){
            cut3 =  (6 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 34);

        }else if (y_iBoard == 1 && y_iCh == 1){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 35);

        }else if (y_iBoard == 1 && y_iCh == 2){
            cut3 =  (10 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 39);

        }else if (y_iBoard == 1 && y_iCh == 3){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 33);
            }
        
    // Scatter Cut
        cut4 = ( 256 - sigma * S1_error < S1_energy ) && ( S1_energy < 256 + sigma * S1_error ); // S1の90°散乱カット
        cut5 = ( 170 - sigma * x_error_lower < x_energy ) && ( x_energy < 340 + sigma * x_error_upper ); // S2の?~90°散乱カット

    // Energy Cut
        cut6 = ( y_lower <= y_energy && y_energy <= y_upper );
        cut7 = ( A1_lower <= A1_energy && A1_energy <= A1_upper );

        TimeCutPassed = !applyTimeCut || (cut0 && cut1 && cut2 && cut3);
        ScatterCutPassed = !applyScatterCut || (cut4 && cut5);
        EnergyCutPassed = !applyEnergyCut || (cut6 && cut7);

        if (TimeCutPassed){
            timecutcounter++;
            if(ScatterCutPassed){
                scattercutcounter++;
            
                if(EnergyCutPassed){
                    validcounter++;
                    fH2Energy_PMTs->Fill(x_energy, y_energy);
                    fH1EnergySpectra[0]->Fill(x_energy);
                    fH1EnergySpectra[1]->Fill(y_energy);
                    fH1EnergySpectra[2]->Fill(x_energy + y_energy);
                }
            }
            
        }

    
        if(Entry % 5000 == 0){
            printf("\tPoint plot : %d\n", Entry);
        }
        allcounter++;

    }

// Pad1: 2Dヒストグラム
    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kRainBow);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kBlack);
    line->SetLineWidth(2);
    line->Draw("SAME");

    curve_upper->SetParameters(x_p0_res, y_p0_res, sigma); // par[0], par[1], par[2]
    curve_upper->SetLineColor(kBlue);
    curve_upper->Draw("SAME");

    
    curve_lower->SetParameters(x_p0_res, y_p0_res, sigma);  
    curve_lower->SetLineColor(kBlue);
    curve_lower->Draw("SAME");

   
// Pad2: y_energy 
    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[1]->Draw();

// Pad3: x_energy 
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[0]->Draw();

// Pad4: x_energy + y_energy 
    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[2]->Draw();
   
    canvas->Update();

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    filename_figure += "_fH2Energy_PMTs.pdf";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure.Data()));
    
    
    std::cout << "all events : " << allcounter << std::endl;
    std::cout << "timecut events : " << timecutcounter << std::endl;
    std::cout << "scattercut events : " << scattercutcounter << std::endl;
    std::cout << "valid events :" << validcounter << std::endl;

    return 0;
}


void DRS4Ana::NaI_waveform_ukai(Int_t x_iBoard = 0,Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1){
    TCanvas *c1 = new TCanvas("title", "name", 1200, 2400);
    c1->Divide(3,2);
    TH2D* hists[3][3];
    //TH2D* hists[3][2];

    // Double_t minEnergy, maxEnergy;
    // Int_t nBins = 100;
    // minEnergy = 0.0;
    // maxEnergy = 600.0;


    Long64_t nentries = fChain->GetEntriesFast();
    Double_t x_DiscriTime, y_DiscriTime, S1_DiscriTime, A1_DiscriTime;
    

    TH1F* fH1TriggerTimes[3];
    //TH1D *fH1EnergySpectra[3];
   
    for(Int_t iCh=0; iCh<4; iCh++){
        if (iCh == 1) {
            continue;  // i が 1 の場合はスキップする
        }
        else{
        hists[0][iCh] = new TH2D(Form("iBoard 0, iCh %d", iCh), Form("iBoard 0, iCh %d",iCh), 500, 0, 1500, 500, -0.55, 0.05);
        hists[0][iCh]->SetTitle(Form("waveform: iBoard 0, iCh %d;time [ns]; voltage [V]",iCh));
        fH1TriggerTimes[iCh] = new TH1F("trigger time", Form("iBoard 0,iCh %d trigger time", iCh), 128, 0, 1023);
        fH1TriggerTimes[iCh]->SetTitle(Form("trigger time: iBoard 0, iCh %d;time [ns]; counts",iCh));
        //fH1EnergySpectra[iCh] = new TH1D("fH1EnergySpectra", Form("y-axis energy spectrum : iBoard 0, iCh %d",iCh, nBins, minEnergy, maxEnergy));
        //fH1EnergySpectra[iCh]->SetTitle(Form("energy spectrum : iBoard 0, iCh %d;energy [keV]; count per %.2f keV", iCh, (maxEnergy-minEnergy)/nBins));
        }
    }
    
    

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);
        
        x_DiscriTime = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        y_DiscriTime = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        S1_DiscriTime = fTime[0][0][fDiscriCell[0][0]];
        A1_DiscriTime = fTime[0][2][fDiscriCell[0][2]];

        Double_t difference_DiscriTime = abs(S1_DiscriTime - x_DiscriTime);


       //if((100 < x_DiscriTime && S1_DiscriTime && A1_DiscriTime < 220) && (difference_DiscriTime < 5) ){
             //&& ( S1_DiscriTime < A1_DiscriTime ) && ( x_DiscriTime < A1_DiscriTime)){

            
              for(Int_t iCh=0; iCh<4; iCh++){

                if (iCh == 1) {
                  continue;  // i=1 の場合はスキップする
                }
                else{
                fH1TriggerTimes[iCh]->Fill(fTime[0][iCh][fDiscriCell[0][iCh]]);
                  for (Int_t iCell = 0; iCell < 1024; iCell++){
                     hists[0][iCh]->Fill(fTime[0][iCh][iCell], fWaveform[0][iCh][iCell]);   
                  }
                }         
              }
        
           
        //}

        if(Entry % 5000 == 0){
            printf("\tPoint plot : %d\n", Entry);
        }

    }

    
        for(Int_t iCh=1; iCh<4; iCh++){ //canvas1~3
            c1->cd(iCh);
            if (iCh == 1) {
            hists[0][iCh-1]->Draw();
            }
            else{
            hists[0][iCh]->Draw();
            }
            gPad->SetLogz();
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    
        for(Int_t iCh=1; iCh<4; iCh++){ //canvas4~6
            c1->cd(iCh+3);
            if (iCh == 1) {
            fH1TriggerTimes[iCh-1]->Draw();  // i が 1 の場合はスキップする
            }
            else{
            fH1TriggerTimes[iCh]->Draw();
            }
        }
        
        // for(Int_t iCh=1; iCh<4; iCh++){ //canvas4~6
        //     if (i == 1) {
        //     c1->cd(iCh+6);
        //     fH1TriggerTimes[0][iCh-1]->Draw();  // i が 1 の場合はスキップする
        //     }
        //     c1->cd(iCh+3);
        //     fH1TriggerTimes[0][iCh]->Draw();
        // }

    
}


void DRS4Ana::osci_ukai(Int_t x_iBoard = 0,Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1){
    
  const int nPoints = 1024;      // サンプル数（適宜変更）
  const int nGraphsPerPad = 4;   // 1つの pad に重ねるグラフの数
  const int maxPads = 16;        // 最大 pad 数（16分割まで対応）
  std::vector<int> validEntries;
  
    

  Long64_t nentries = fChain->GetEntriesFast();

    TString key_Crystal_y;
    TString key = "0204";

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Double_t x_energy, y_energy, S1_energy, A1_energy;
    Double_t x_error, y_error, S1_error, A1_error, x_error_upper, x_error_lower;
    Double_t x_charge_buf, y_charge_buf, S1_charge_buf, A1_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    Double_t y_upper, y_lower;

    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    
    Double_t x_DiscriTime, y_DiscriTime, S1_DiscriTime, A1_DiscriTime;
    Double_t x_adcSum_timerange, y_adcSum_timerange;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    
    x_adcSum_timerange = 600;


    if ((y_iBoard == 0 && y_iCh == 1) || (y_iBoard == 1)) {
    y_adcSum_timerange = 180;
    key_Crystal_y = "GSO";
    }
    else if (y_iBoard == 0 && y_iCh == 2) {
    y_adcSum_timerange = 600; 
    key_Crystal_y = "NaI";
    }
    else{
        printf("\t\ny axis || type of crystal is invalid\n");
    }


    TF1 *curve_upper = new TF1("curve_upper", CurveUpper_y, 0.0, 511.0, 3);  
    TF1 *curve_lower = new TF1("curve_lower", CurveLower_y, 0.0, 511.0, 3); 



  for (Int_t Entry = 0; Entry < nentries; Entry++) {
        fChain->GetEntry(Entry);

        x_DiscriTime = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        y_DiscriTime = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        S1_DiscriTime = fTime[0][0][fDiscriCell[0][0]];
        A1_DiscriTime = fTime[0][2][fDiscriCell[0][2]];

        Double_t S1A1_DiscriTime = abs(S1_DiscriTime - A1_DiscriTime);
        Double_t S2A1_DiscriTime = abs(x_DiscriTime - A1_DiscriTime);

        S1_charge_buf = -GetChargeIntegral(0, 0, 20, S1_DiscriTime - 50, S1_DiscriTime + 600);
        A1_charge_buf = -GetChargeIntegral(0, 2, 20, A1_DiscriTime - 50, A1_DiscriTime + 600);

        S1_energy = p0[0][0] + p1[0][0]* S1_charge_buf;
        A1_energy = p0[0][2] + p1[0][2]* A1_charge_buf;
        S1_error = p0_res[0][0] * sqrt(256) * 0.01 / (2 * sqrt(2 * log(2)));
        A1_error = p0_res[0][2] * sqrt(A1_energy) * 0.01 / (2 * sqrt(2 * log(2)));

        x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, x_DiscriTime - 50, x_DiscriTime + x_adcSum_timerange);
        y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, y_DiscriTime - 50, y_DiscriTime + y_adcSum_timerange);

        x_energy = x_p0_buf + x_p1_buf * x_charge_buf;
        y_energy = y_p0_buf + y_p1_buf * y_charge_buf;

        x_error = x_p0_res_buf * sqrt(x_energy) * 0.01 / (2 * sqrt(2 * log(2)));
        y_error = y_p0_res_buf * sqrt(y_energy) * 0.01 / (2 * sqrt(2 * log(2)));
          
        x_error_upper = x_p0_res_buf * sqrt(256) * 0.01 / (2 * sqrt(2 * log(2)));
        x_error_lower = x_p0_res_buf * sqrt(170) * 0.01 / (2 * sqrt(2 * log(2)));

        y_upper = curve_upper->Eval(x_energy);
        y_lower = curve_lower->Eval(x_energy);


        // 条件を満たすかチェック
        if ( (100 < y_DiscriTime && y_DiscriTime < 220) && ( 135 < A1_DiscriTime ) 
              && (10 < y_energy)
              && ( 256 - 3 * S1_error < S1_energy )&& (S1_energy < 256 + 3 * S1_error) && ( 170 - 3 * x_error_lower < x_energy ) && (x_energy < 256 + 3 * x_error_upper) 
              //&& (y_lower <= y_energy && y_energy <= y_upper)) 
        ){
            validEntries.push_back(Entry);  // 条件を満たすイベントを追加
        }

        if (Entry % 1000 == 0) {
            printf("\tPoint plot : %d\n", Entry);
        }
    }

      
  int colors[] = {kOrange, kBlue-4, kRed-7, kGreen};
  
  // 条件を満たすイベント数に応じてキャンバス分割数を決定
    int nValidEntries = validEntries.size();
    int nPads = std::min(nValidEntries, maxPads);  // 最大 maxPads 個まで描画
    int nDiv = ceil(sqrt(nPads));  // 分割数を動的に決定（例: 9個なら 3×3, 4個なら 2×2）


  TCanvas* canvas = new TCanvas("canvas", "Canvas with 9 Pads", 1200, 900);
  canvas->Divide(nDiv, nDiv);


// 条件を満たす最初の9つのイベントを描画
    for (int iPad = 0; iPad < nPads; ++iPad) {
        canvas->cd(iPad + 1);  // 各 pad を選択

        // 条件を満たすデータが足りない場合、そのパッドを空白にする
        if (iPad >= validEntries.size()) {
            std::cout << "Pad " << iPad + 1 << " is empty due to insufficient valid entries." << std::endl;
            continue;  // そのパッドをスキップして次の pad に進む
        }

        int eventNumber = validEntries[iPad];
        fChain->GetEntry(eventNumber);


        // 4つの波形を重ね書き
        for (int iGraph = 0; iGraph < nGraphsPerPad; ++iGraph) {

            // それぞれの波形データを取得
            double x[nPoints], y[nPoints];
            
            // x波形データとy波形データを それぞれのインデックスで取得し、グラフを作成
            for (int iCell = 0; iCell < nPoints; ++iCell) {
                // (0,0) の波形
                if (iGraph == 0) {
                    x[iCell] = fTime[0][0][iCell];
                    y[iCell] = fWaveform[0][0][iCell]; 
                }
                // (0,2) の波形
                else if (iGraph == 1) {
                    x[iCell] = fTime[0][2][iCell];  
                    y[iCell] = fWaveform[0][2][iCell]; 
                }
                // (0,3) の波形
                else if (iGraph == 2) {
                    x[iCell] = fTime[0][3][iCell];  
                    y[iCell] = fWaveform[0][3][iCell]; 
                }
                // (y_iBoard, y_iCh) の波形
                else {
                    x[iCell] = fTime[y_iBoard][y_iCh][iCell]; 
                    y[iCell] = fWaveform[y_iBoard][y_iCh][iCell];
                }
            }

            // グラフを作成
            TGraph* graph = new TGraph(nPoints, x, y);
            graph->SetLineColor(colors[iGraph]);  // 色の設定
            graph->SetLineWidth(2);

            graph->SetTitle(Form("event %d;Time [ns];Voltage [V]",eventNumber));

            graph->GetXaxis()->SetRangeUser(0, 1500); 
            graph->GetYaxis()->SetRangeUser(-0.55, 0.05);

            if (iGraph == 0) graph->Draw("AL");  
            else graph->Draw("L SAME");  
        }
    
    } 
    

}






Double_t DRS4Ana::Plot_2Dhist_energy_with_cut1(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 1200);
    canvas->Divide(2,2);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    printf("\n\n\ty_p0_res_buf %f\n\n", y_p0_res_buf);





    Double_t distance_from_511_line, x_distance_btwn_2points, y_distance_btwn_2points;
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

            x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
            y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
            x_error = 3 * 0.01 * x_p0_res_buf*sqrt(x_energy)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。
            y_error = 3 * 0.01 * y_p0_res_buf*sqrt(y_energy)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。

            x_error = x_p0_res_buf*sqrt(x_energy)/(2*sqrt(2*log(2)));
            y_error = y_p0_res_buf*sqrt(y_energy)/(2*sqrt(2*log(2)));
            
            distance_from_511_line = pow((x_energy + y_energy - 511.0),2.0) / 2.0;
            x_distance_btwn_2points = pow((511.0+x_energy-y_energy)/2.0 - (511.0-y_energy), 2.0) + pow((511.0-x_energy+y_energy)/2.0 - y_energy, 2.0);
            y_distance_btwn_2points = pow((511.0+x_energy-y_energy)/2.0 - x_energy, 2.0) + pow((511.0-x_energy+y_energy)/2.0 - (511.0-x_energy), 2.0);
            
            if((pow(x_error,2.0) > (distance_from_511_line + x_distance_btwn_2points)) && (pow(y_error,2.0) > (distance_from_511_line + y_distance_btwn_2points))){
                if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                    discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
                    discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];
                    Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, 20.0, discriTime_S1 - 50, discriTime_S1 + 600);
                    Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, 20.0, discriTime_A1 - 50, discriTime_A1 + 600);

                    Double_t energy_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
                    Double_t energy_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

                    Double_t energy_error_S1 = 3 * 0.01 * p0_res[S1_BoardID][S1_ChID]*sqrt(energy_S1)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。
                    Double_t energy_error_A1 = 3 * 0.01 * p0_res[A1_BoardID][A1_ChID]*sqrt(energy_A1)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。

                    distance_from_511_line = pow((energy_S1 + energy_A1 - 511.0),2.0) / 2.0;
                    x_distance_btwn_2points = pow((511.0+energy_S1-energy_A1)/2.0 - (511.0-energy_A1), 2.0) + pow((511.0-energy_S1+energy_A1)/2.0 - energy_A1, 2.0);
                    y_distance_btwn_2points = pow((511.0+energy_S1-energy_A1)/2.0 - energy_S1, 2.0) + pow((511.0-energy_S1+energy_A1)/2.0 - (511.0-energy_S1), 2.0);

                    if((pow(energy_error_S1,2.0) > (distance_from_511_line + x_distance_btwn_2points)) && (pow(energy_error_A1,2.0) > (distance_from_511_line + y_distance_btwn_2points))){
                        fH2Energy_PMTs->Fill(x_energy, y_energy);
                        fH1EnergySpectra[0]->Fill(x_energy);
                        fH1EnergySpectra[1]->Fill(y_energy);
                        fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                        counter++;
                    }
                }
            }
        }

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();
    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[2]->Draw();

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


Double_t DRS4Ana::Plot_2Dhist_energy_with_cut2(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_sato = 4, Int_t nSigma_GSO = 4){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 2000);
    canvas->Divide(2,3);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 50, -50, 600, 50, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy of two PMTs (data from cfg/%s/data.txt), cut by %d sigma;Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), nSigma_GSO , x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    TH1F* fH1TriggerTimes[2];
    fH1TriggerTimes[0] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", x_iBoard, x_iCh), 128, 0, 1023);
    fH1TriggerTimes[1] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", y_iBoard, y_iCh), 128, 0, 1023);

    gPad->SetGrid();
    // gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    // printf("\n\n\ty_p0_res_buf %f\n\n", y_p0_res_buf);
    Double_t distance_from_511_line;
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

    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kDeepSea);

    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(5);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(6);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        //satoのdiscriCell分布の右の山を消す。
        if(fDiscriCell[0][2]<133){
            // if(150 < DiscriTime_y && DiscriTime_y < 250){
            if(true){
                x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
                y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

                x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
                y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
                
                // if(abs(y_energy - (511.0 - x_energy)) < nSigma_GSO*0.01*(y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0))))){
                    if(abs(y_energy - (511.0 - x_energy)) < nSigma_GSO*0.01*(y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))))){
                    if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                        discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
                        discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];
                        Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, 20.0, discriTime_S1 - 50, discriTime_S1 + 600);
                        Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, 20.0, discriTime_A1 - 50, discriTime_A1 + 600);

                        Double_t energy_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
                        Double_t energy_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

                        if((energy_A1 > 50) && (energy_S1 > 50) && (energy_A1< 400) && (energy_S1 < 400)){
                            if(abs(energy_A1 - (511.0 - energy_S1)) < nSigma_sato*0.01*(p0_res[A1_BoardID][A1_ChID] * sqrt(abs(511.0-energy_S1)) / (2.0*sqrt(2.0*log(2.0))) + p0_res[S1_BoardID][S1_ChID] * sqrt(abs(energy_S1)) / (2.0*sqrt(2.0*log(2.0))))){
                            fH2Energy_PMTs->Fill(x_energy, y_energy);
                            fH1EnergySpectra[0]->Fill(x_energy);
                            fH1EnergySpectra[1]->Fill(y_energy);
                            fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                            fH1TriggerTimes[0]->Fill(fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]]);
                            fH1TriggerTimes[1]->Fill(fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]]);
                            counter++;
                            }
                        }
                    }
                }
            }
        }
        

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 3);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf,nSigma_GSO);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 3);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf,nSigma_GSO);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");

    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 3);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf,1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 3);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf,1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gStyle->SetOptStat(1);
    fH1EnergySpectra[2]->Draw();
    canvas->cd(5);
    fH1TriggerTimes[0]->Draw();
    canvas->cd(6);
    fH1TriggerTimes[1]->Draw();

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
    TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + "_fH2Energy_PMTs.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    return counter;
}

Double_t DRS4Ana::Plot_2Dhist_energy_with_cut2_1(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma = 4){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 1200);
    canvas->Divide(2,2);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    printf("\n\n\ty_p0_res_buf %f\n\n", y_p0_res_buf);
    Double_t distance_from_511_line;
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
        //satoのdiscriCell分布の右の山を消す。
        if(fDiscriCell[y_iBoard][y_iCh]<135){
            x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
            // y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);
            y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, 153 - 50, 153 + adcSum_timerange_y);

            x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
            y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
            // x_error = 4 * 0.01 * x_p0_res_buf*sqrt(x_energy)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。
            // y_error = 4 * 0.01 * y_p0_res_buf*sqrt(y_energy)/(2*sqrt(2*log(2))); //0.01はenergy resolution (percent)を割合に変えるため。
            // distance_from_511_line = pow((x_energy + y_energy - 511.0),2.0) / 2.0;
            
            if(abs(y_energy - (511.0 - x_energy)) < nSigma*0.01*(y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0))))){
                if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                    fH2Energy_PMTs->Fill(x_energy, y_energy);
                    fH1EnergySpectra[0]->Fill(x_energy);
                    fH1EnergySpectra[1]->Fill(y_energy);
                    fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                    counter++;
                }
            }
        }
        

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kInvertedDarkBodyRadiator);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 3);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf,nSigma);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 3);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf,nSigma);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");
    //カット範囲の図示
    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 3);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf,1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 3);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf,1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[2]->Draw();

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
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    return counter;
}


Double_t DRS4Ana::Plot_2Dhist_energy_with_cut3(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 2000);
    canvas->Divide(2,3);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 50, -50, 600, 50, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy of two PMTs (data from cfg/%s/data.txt), cut (S2 %d sigma, A2 %d sigma), cut (S1 %d sigma, A1 %d sigma);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), nSigma_x_S2, nSigma_y_A2, nSigma_S1, nSigma_A1, x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    TH1F* fH1TriggerTimes[2];
    fH1TriggerTimes[0] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", x_iBoard, x_iCh), 128, 0, 1023);
    fH1TriggerTimes[1] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", y_iBoard, y_iCh), 128, 0, 1023);

    gPad->SetGrid();
    // gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    // printf("\n\n\ty_p0_res_buf %f\n\n", y_p0_res_buf);
    Double_t distance_from_511_line;
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

    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kDeepSea);

    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(5);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(6);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        //satoのdiscriCell分布の右の山を消す。
        if(fDiscriCell[0][2]<133)
        {
            if(100 < DiscriTime_y && DiscriTime_y < 220)
            {
                x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
                y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

                x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
                y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
                
                if(abs(y_energy - (511.0 - x_energy)) < 0.01*(nSigma_y_A2 * y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_x_S2 * x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0)))))
                {
                    if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                        discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
                        discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];
                        Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, 20.0, discriTime_S1 - 50, discriTime_S1 + 600);
                        Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, 20.0, discriTime_A1 - 50, discriTime_A1 + 600);

                        Double_t energy_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
                        Double_t energy_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

                        if((energy_A1 > 50) && (energy_S1 > 50) && (energy_A1< 400) && (energy_S1 < 400))
                        {
                            if(abs(energy_A1 - (511.0 - energy_S1)) < 0.01*(nSigma_A1 * p0_res[A1_BoardID][A1_ChID] * sqrt(abs(511.0-energy_S1)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_S1 * p0_res[S1_BoardID][S1_ChID] * sqrt(abs(energy_S1)) / (2.0*sqrt(2.0*log(2.0))))){
                            fH2Energy_PMTs->Fill(x_energy, y_energy);
                            fH1EnergySpectra[0]->Fill(x_energy);
                            fH1EnergySpectra[1]->Fill(y_energy);
                            fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                            fH1TriggerTimes[0]->Fill(fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]]);
                            fH1TriggerTimes[1]->Fill(fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]]);
                            counter++;
                            }
                        }
                    }
                }
            }
        }
        

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");

    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gStyle->SetOptStat(1);
    fH1EnergySpectra[2]->Draw();
    canvas->cd(5);
    fH1TriggerTimes[0]->Draw();
    canvas->cd(6);
    fH1TriggerTimes[1]->Draw();

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
    TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + "_fH2Energy_PMTs.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    return counter;
}

Double_t DRS4Ana::Plot_2Dhist_energy_with_cut3_t(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2){
    //κ求めるのには約立たないけど、huruno1とsatoでのカットをしないバージョン
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 2000);
    canvas->Divide(2,3);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 50, -50, 600, 50, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy of two PMTs (data from cfg/%s/data.txt), cut (S2 %d sigma, A2 %d sigma);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), nSigma_x_S2, nSigma_y_A2, x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    TH1F* fH1TriggerTimes[2];
    fH1TriggerTimes[0] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", x_iBoard, x_iCh), 128, 0, 1023);
    fH1TriggerTimes[1] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", y_iBoard, y_iCh), 128, 0, 1023);

    gPad->SetGrid();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
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

    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kDeepSea);

    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(5);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(6);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        //satoのdiscriCell分布の右の山を消す。
        if(fDiscriCell[0][2]>133)
        {
            if(100 < DiscriTime_y && DiscriTime_y < 220)
            {
                x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
                y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

                x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
                y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
                
                if(abs(y_energy - (511.0 - x_energy)) < 0.01*(nSigma_y_A2 * y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_x_S2 * x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0)))))
                {
                    if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400))
                    {
                        fH2Energy_PMTs->Fill(x_energy, y_energy);
                        fH1EnergySpectra[0]->Fill(x_energy);
                        fH1EnergySpectra[1]->Fill(y_energy);
                        fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                        fH1TriggerTimes[0]->Fill(fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]]);
                        fH1TriggerTimes[1]->Fill(fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]]);
                        counter++;
                    }
                }
            }
        }
        

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");

    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gStyle->SetOptStat(1);
    fH1EnergySpectra[2]->Draw();
    canvas->cd(5);
    fH1TriggerTimes[0]->Draw();
    canvas->cd(6);
    fH1TriggerTimes[1]->Draw();

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
    TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + "_fH2Energy_PMTs.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    return counter;
}


Double_t DRS4Ana::Plot_2Dhist_energy_with_cut4(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 2000, 2000);
    canvas->Divide(2,3);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 50, -50, 600, 50, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy of two PMTs (data from cfg/%s/data.txt), cut (S2 %d sigma, A2 %d sigma), cut (S1 %d sigma, A1 %d sigma);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), nSigma_x_S2, nSigma_y_A2, nSigma_S1, nSigma_A1, x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    TH1F* fH1TriggerTimes[2];
    fH1TriggerTimes[0] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", x_iBoard, x_iCh), 128, 0, 1023);
    fH1TriggerTimes[1] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", y_iBoard, y_iCh), 128, 0, 1023);

    gPad->SetGrid();
    // gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    //他のA2でヒットがないことをカット条件に入れたい。
    Int_t iCh_other_A2s[3];
    iCh_other_A2s[0] = (y_iCh + 1) % 4;
    iCh_other_A2s[1] = (y_iCh + 2) % 4;
    iCh_other_A2s[2] = (y_iCh + 3) % 4;

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    // printf("\n\n\ty_p0_res_buf %f\n\n", y_p0_res_buf);
    Double_t distance_from_511_line;
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

    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kDeepSea);

    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(5);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    canvas->cd(6);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        //satoのdiscriCell分布の右の山を消す。
        if(fDiscriCell[0][2]<133)
        {
            if(100 < DiscriTime_y && DiscriTime_y < 220)
            {
                x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
                y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

                x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
                y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
                
                if(abs(y_energy - (511.0 - x_energy)) < 0.01*(nSigma_y_A2 * y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_x_S2 * x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0)))))
                {
                    if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                        discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
                        discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];
                        Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, 20.0, discriTime_S1 - 50, discriTime_S1 + 600);
                        Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, 20.0, discriTime_A1 - 50, discriTime_A1 + 600);

                        Double_t energy_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
                        Double_t energy_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

                        if((energy_A1 > 50) && (energy_S1 > 50) && (energy_A1< 400) && (energy_S1 < 400))
                        {
                            if(abs(energy_A1 - (511.0 - energy_S1)) < 0.01*(nSigma_A1 * p0_res[A1_BoardID][A1_ChID] * sqrt(abs(511.0-energy_S1)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_S1 * p0_res[S1_BoardID][S1_ChID] * sqrt(abs(energy_S1)) / (2.0*sqrt(2.0*log(2.0)))))
                            {
                                Int_t flag_other_counter_cut = 0;
                                Double_t chargeIntegral_another_A2s[3];
                                // Double_t discriTime_other_A2s[3];
                                // for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                // {
                                //     discriTime_other_A2s[another_iCh] = fTime[y_iBoard][iCh_other_A2s[another_iCh]][fDiscriCell[y_iBoard][iCh_other_A2s[another_iCh]]];
                                // }
                                for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                {
                                    chargeIntegral_another_A2s[another_iCh] = GetChargeIntegral(y_iBoard, iCh_other_A2s[another_iCh], 20, DiscriTime_y - 50.0, DiscriTime_y + 180.0);
                                }
                                Double_t energy_other_A2s[3];
                                for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                {
                                    Double_t energy_another_buf = p0[y_iBoard][iCh_other_A2s[another_iCh]] + p1[y_iBoard][iCh_other_A2s[another_iCh]] * (-chargeIntegral_another_A2s[another_iCh]);
                                    energy_other_A2s[another_iCh] = energy_another_buf;
                                    if(energy_another_buf > 100)
                                    {
                                        flag_other_counter_cut++;
                                        printf("\n\tEntry %d, y_iBoard %d, iCh %d, abnormal energy %f keV\n", Entry, y_iBoard, iCh_other_A2s[another_iCh], energy_another_buf);
                                    }
                                }
                                if(flag_other_counter_cut == 0)
                                {
                                    fH2Energy_PMTs->Fill(x_energy, y_energy);
                                    fH1EnergySpectra[0]->Fill(x_energy);
                                    fH1EnergySpectra[1]->Fill(y_energy);
                                    fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                                    fH1TriggerTimes[0]->Fill(fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]]);
                                    fH1TriggerTimes[1]->Fill(fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]]);
                                    counter++;
                                }
                            
                            }
                        }
                    }
                }
            }
        }
        

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");

    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gStyle->SetOptStat(1);
    fH1EnergySpectra[2]->Draw();
    canvas->cd(5);
    fH1TriggerTimes[0]->Draw();
    canvas->cd(6);
    fH1TriggerTimes[1]->Draw();

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
    TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + "_fH2Energy_PMTs.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    for(Int_t another_iCh=0; another_iCh<3; another_iCh++){
        printf("\tother GSO counter %d\n", iCh_other_A2s[another_iCh]);
    }
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    return counter;
}


Double_t DRS4Ana::Plot_TriggerTimeDist_8ch_sato_cut(){
    // satoのdiscriCellで条件をつけてみたい。
    Double_t nentries = fChain->GetEntriesFast();
    Double_t counter = 0.0;

    TCanvas *c1 = new TCanvas("title", "name", 2400, 12000);
    c1->Divide(2,4);
    TH1D* hists[2][4];
    TF1* fits[2][4];
    gPad->SetLogz();
    gPad->SetGrid();
    gStyle->SetOptStat(0);
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            hists[iBoard][iCh] = new TH1D(Form("title_ib%d_ic%d", iBoard, iCh), Form("name_ib%d_ic%d", iBoard, iCh), 512, fWaveformXmin, fWaveformXmax);
            fits[iBoard][iCh] = new TF1("fit%d %d", "gaus", 150, 220);
        }
    }
    Double_t trig_time_buf;
    for(Int_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);
        if(fDiscriCell[0][2] > 133)
        {
            if(fTime[0][0][fDiscriCell[0][0]] < 150)
            for(Int_t iBoard=0; iBoard<2; iBoard++){
                for(Int_t iCh=0; iCh<4; iCh++){
                    for(Int_t iCell=0; iCell<1024; iCell++){
                        trig_time_buf = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
                        if(trig_time_buf < 1400 && trig_time_buf > 10)
                        hists[iBoard][iCh]->Fill(trig_time_buf);
                    }
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
            hists[iBoard][iCh]->Fit("gaus", "", "", 100, 200);
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    TString folderPath = Makedir_Date();
    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + "_allCH_triggertime.pdf";
    TString filename_figure_png = filename_figure + "_allCH_triggertime.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));
    IfFile_duplication(folderPath, filename_figure_png);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    return (Double_t)counter;
}


Double_t DRS4Ana::Plot_TriggerTimeDist_8ch_difference(){
    Double_t nentries = fChain->GetEntriesFast();
    Double_t counter = 0.0;

    TCanvas *c1 = new TCanvas("title", "name", 3000, 2000);
    c1->Divide(2,4);
    gStyle->SetLabelSize(0.08, "XYZ");  // 軸ラベルサイズ
    gStyle->SetTitleSize(0.10, "XYZ");  // 軸タイトルサイズ
    gStyle->SetTitleSize(0.1, "t"); // "t" はタイトル全体を指す
    
    TH1D* hists[2][4];
    gPad->SetLogz();
    gPad->SetGrid();
    gStyle->SetOptStat(0);
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            if(iBoard == 0 && iCh == 0){
                hists[iBoard][iCh] = new TH1D(Form("title_ib%d_ic%d", iBoard, iCh), Form("name_ib%d_ic%d", iBoard, iCh), 512, fWaveformXmin, fWaveformXmax);
            }
            else{
                hists[iBoard][iCh] = new TH1D(Form("title_ib%d_ic%d", iBoard, iCh), Form("name_ib%d_ic%d", iBoard, iCh), 512, fWaveformXmin - 200.0, fWaveformXmax - 200.0);
            }
            
            // fits[iBoard][iCh] = new TF1("fit%d %d", "gaus", 150, 220);
        }
    }
    Double_t trig_time_buf;
    Double_t trig_time_base_buf;
    for(Int_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);
        trig_time_base_buf = fTime[0][0][fDiscriCell[0][0]];
        if(fDiscriCell[0][2] < 135)
        {
            for(Int_t iBoard=0; iBoard<2; iBoard++)
            {
                for(Int_t iCh=0; iCh<4; iCh++)
                {
                    trig_time_buf = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
                    if(trig_time_buf < 1400 && trig_time_buf > 10){
                        if(iBoard == 0 && iCh == 0){
                            hists[iBoard][iCh]->Fill(trig_time_buf);
                        }
                        else{
                            hists[iBoard][iCh]->Fill(trig_time_buf - trig_time_base_buf);
                        }
                        
                    }
                    
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
            hists[iBoard][iCh]->Fit("gaus", "", "", 100, 200);
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    TString folderPath = Makedir_Date();
    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + "_allCH_triggertime.pdf";
    TString filename_figure_png = filename_figure + "_allCH_triggertime.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));
    IfFile_duplication(folderPath, filename_figure_png);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    return (Double_t)counter;
}


Double_t DRS4Ana::Plot_2Dhist_energy_with_cut5(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2){
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 4000, 4000);
    canvas->Divide(2,4);
    gStyle->SetLabelSize(0.08, "XYZ");  // 軸ラベルサイズ
    gStyle->SetTitleSize(0.10, "XYZ");  // 軸タイトルサイズ
    gStyle->SetTitleSize(0.1, "t"); // "t" はタイトル全体を指す
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 50, -50, 600, 50, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy of two PMTs (data from cfg/%s/data.txt), cut (S2 %d sigma, A2 %d sigma), cut (S1 %d sigma, A1 %d sigma);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), nSigma_x_S2, nSigma_y_A2, nSigma_S1, nSigma_A1, x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    TH1F* fH1TriggerTimes[2];
    fH1TriggerTimes[0] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", x_iBoard, x_iCh), 128, 0, 1023);
    fH1TriggerTimes[1] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", y_iBoard, y_iCh), 128, 0, 1023);

    gPad->SetGrid();
    // gPad->SetLogz();
    gStyle->SetOptStat(0);

    TH2D *fH2DiscriCells[2];
    fH2DiscriCells[0] = new TH2D("fH2DiscriCell", Form("iBoard %d iCh %d vs iBoard %d iCh %d discriCell", 0,0,0,2), 200,0,199,200,0,199);
    fH2DiscriCells[1] = new TH2D("fH2DiscriCell", Form("iBoard %d iCh %d vs iBoard %d iCh %d discriCell(cut passed)", 0,0,0,2), 200,0,199,200,0,199);
    gPad->SetGrid();

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    //他のA2でヒットがないことをカット条件に入れたい。
    Int_t iCh_other_A2s[3];
    iCh_other_A2s[0] = (y_iCh + 1) % 4;
    iCh_other_A2s[1] = (y_iCh + 2) % 4;
    iCh_other_A2s[2] = (y_iCh + 3) % 4;

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    Double_t distance_huruno1_sato_discriCell_line;
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

    canvas->cd(1);
    gPad->SetGrid();
    gStyle->SetPalette(kDeepSea);

    canvas->cd(2);
    gPad->SetGrid();

    canvas->cd(3);
    gPad->SetGrid();

    canvas->cd(4);
    gPad->SetGrid();

    canvas->cd(5);
    gPad->SetGrid();

    canvas->cd(6);
    gPad->SetGrid();

    canvas->cd(7);
    gPad->SetGrid();

    canvas->cd(8);
    gPad->SetGrid();

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        //huruno1とsatoのdiscriCellでのカット
        distance_huruno1_sato_discriCell_line = pow(fDiscriCell[0][0] - fDiscriCell[0][2] - 7, 2.0) / 2.0;
        if(distance_huruno1_sato_discriCell_line < 50.0)
        {
            fH2DiscriCells[0]->Fill(fDiscriCell[0][0], fDiscriCell[0][2]);
            //satoのdiscriCell分布の右の山を消す。
            if(fDiscriCell[0][2]<135)
            {
                if(100 < DiscriTime_y && DiscriTime_y < 220)
                {
                    x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
                    y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

                    x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
                    y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
                    
                    if(abs(y_energy - (511.0 - x_energy)) < 0.01*(nSigma_y_A2 * y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_x_S2 * x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0)))))
                    {
                        if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                            discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
                            discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];
                            Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, 20.0, discriTime_S1 - 50, discriTime_S1 + 600);
                            Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, 20.0, discriTime_A1 - 50, discriTime_A1 + 600);

                            Double_t energy_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
                            Double_t energy_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

                            if((energy_A1 > 50) && (energy_S1 > 50) && (energy_A1< 400) && (energy_S1 < 400))
                            {
                                if(abs(energy_A1 - (511.0 - energy_S1)) < 0.01*(nSigma_A1 * p0_res[A1_BoardID][A1_ChID] * sqrt(abs(511.0-energy_S1)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_S1 * p0_res[S1_BoardID][S1_ChID] * sqrt(abs(energy_S1)) / (2.0*sqrt(2.0*log(2.0)))))
                                {
                                    Int_t flag_other_counter_cut = 0;
                                    Double_t chargeIntegral_another_A2s[3];
                                    // Double_t discriTime_other_A2s[3];
                                    // for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                    // {
                                    //     discriTime_other_A2s[another_iCh] = fTime[y_iBoard][iCh_other_A2s[another_iCh]][fDiscriCell[y_iBoard][iCh_other_A2s[another_iCh]]];
                                    // }
                                    for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                    {
                                        chargeIntegral_another_A2s[another_iCh] = GetChargeIntegral(y_iBoard, iCh_other_A2s[another_iCh], 20, DiscriTime_y - 50.0, DiscriTime_y + 180.0);
                                    }
                                    Double_t energy_other_A2s[3];
                                    for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                    {
                                        Double_t energy_another_buf = p0[y_iBoard][iCh_other_A2s[another_iCh]] + p1[y_iBoard][iCh_other_A2s[another_iCh]] * (-chargeIntegral_another_A2s[another_iCh]);
                                        energy_other_A2s[another_iCh] = energy_another_buf;
                                        if(energy_another_buf > 100)
                                        {
                                            flag_other_counter_cut++;
                                            printf("\n\tEntry %d, y_iBoard %d, iCh %d, abnormal energy %f keV\n", Entry, y_iBoard, iCh_other_A2s[another_iCh], energy_another_buf);
                                        }
                                    }
                                    if(y_iBoard == 0 && y_iCh == 1){
                                        flag_other_counter_cut = 0;
                                    }
                                    if(flag_other_counter_cut == 0)
                                    {
                                        fH2DiscriCells[1]->Fill(fDiscriCell[0][0], fDiscriCell[0][2]);
                                        fH2Energy_PMTs->Fill(x_energy, y_energy);
                                        fH1EnergySpectra[0]->Fill(x_energy);
                                        fH1EnergySpectra[1]->Fill(y_energy);
                                        fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                                        fH1TriggerTimes[0]->Fill(fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]]);
                                        fH1TriggerTimes[1]->Fill(fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]]);
                                        counter++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");

    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gStyle->SetOptStat(1);
    fH1EnergySpectra[2]->Draw();
    canvas->cd(5);
    fH1TriggerTimes[0]->Draw();
    canvas->cd(6);
    fH1TriggerTimes[1]->Draw();
    canvas->cd(7);
    gPad->SetLogz();
    fH2DiscriCells[0]->Draw();
    canvas->cd(8);
    gPad->SetLogz();
    fH2DiscriCells[1]->Draw();

    canvas->cd(1);
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kBlack);
    line->SetLineWidth(2);
    line->Draw("SAME");

    canvas->ResizePad();
    canvas->Update();

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + "_fH2Energy_PMTs.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    for(Int_t another_iCh=0; another_iCh<3; another_iCh++){
        printf("\tother GSO counter %d\n", iCh_other_A2s[another_iCh]);
    }
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    return counter;
}


Double_t DRS4Ana::Plot_2Dhist_energy_with_cut5_t(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2){
    /*
    ・S1, S2のANDでDAQしたもの
    ・DRS4のdaisy-chainのDAQレート(— 370 acq/s)に張り付いていた
    ・S1で511 keVのエネルギーを落としたイベントだけ使う。(S1でコンプトン散乱してGSOに入るイベントを排除したい)
    */
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", Form("%s cut5_t", fRootFile.Data()), 2000, 3000);
    TCanvas *canvas_S1_energy = new TCanvas("canvas_S1_energy", "S1 energy", 1200, 800);
    canvas->Divide(2,3);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 50, -50, 600, 50, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy of two PMTs (data from cfg/%s/data.txt), cut (S2 %d sigma, A2 %d sigma);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), nSigma_x_S2, nSigma_y_A2, x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    TH1F* fH1TriggerTimes[2];
    fH1TriggerTimes[0] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", x_iBoard, x_iCh), 128, 0, 1023);
    fH1TriggerTimes[1] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", y_iBoard, y_iCh), 128, 0, 1023);

    TH1F *fH1EnergySpectrum = new TH1F("energy_spectrum", Form("Energy spectrum S1 (when cut iBoard %d iCh %d and iBoard %d iCh %d)", x_iBoard, x_iCh, y_iBoard, y_iCh), 200, 0, 600.0);

    gPad->SetGrid();
    // gPad->SetLogz();
    gStyle->SetOptStat(0);

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    //huruno1で511 keVを落としたことをカット条件に入れたい。
    Double_t S1_chargeInt, S1_energy, S1_discriTime;
    Double_t S1_p0_buf = p0[0][0];
    Double_t S1_p1_buf = p1[0][0];
    Double_t S1_p0_res_buf = p0_res[0][0];
    Double_t S1_error = 0.01 * S1_p0_res_buf * sqrt(511.0) / (2.0*sqrt(2.0*log(2.0)));
    printf("\n\tDEBUG : S1_error %f\n", S1_error);

    //他のA2でヒットがないことをカット条件に入れたい。
    Int_t iCh_other_A2s[3];
    iCh_other_A2s[0] = (y_iCh + 1) % 4;
    iCh_other_A2s[1] = (y_iCh + 2) % 4;
    iCh_other_A2s[2] = (y_iCh + 3) % 4;

    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
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

    canvas->cd(1);
    gPad->SetGrid();
    gStyle->SetPalette(kDeepSea);

    canvas->cd(2);
    gPad->SetGrid();

    canvas->cd(3);
    gPad->SetGrid();

    canvas->cd(4);
    gPad->SetGrid();

    canvas->cd(5);
    gPad->SetGrid();

    canvas->cd(6);
    gPad->SetGrid();

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        S1_discriTime = fTime[0][0][fDiscriCell[0][0]];
        S1_chargeInt = GetChargeIntegral(0, 0, 20, S1_discriTime - 50.0, S1_discriTime + 600.0);
        S1_energy = S1_p0_buf + S1_p1_buf * (-S1_chargeInt);

        if(abs(S1_energy - 500) < 2.0 * S1_error)
        {
            fH1EnergySpectrum->Fill(S1_energy);
            DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
            DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];

            if(100 < DiscriTime_y && DiscriTime_y < 220)
            {
                x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
                y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

                x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
                y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
                
                if(abs(y_energy - (511.0 - x_energy)) < 0.01*(nSigma_y_A2 * y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_x_S2 * x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0)))))
                {
                    if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400))
                    {
                        Int_t flag_other_counter_cut = 0;
                        Double_t chargeIntegral_another_A2s[3];
                        for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                        {
                            chargeIntegral_another_A2s[another_iCh] = GetChargeIntegral(y_iBoard, iCh_other_A2s[another_iCh], 20, DiscriTime_y - 50.0, DiscriTime_y + 180.0);
                        }
                        Double_t energy_other_A2s[3];
                        for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                        {
                            Double_t energy_another_buf = p0[y_iBoard][iCh_other_A2s[another_iCh]] + p1[y_iBoard][iCh_other_A2s[another_iCh]] * (-chargeIntegral_another_A2s[another_iCh]);
                            energy_other_A2s[another_iCh] = energy_another_buf;
                            if(energy_another_buf > 100)
                            {
                                flag_other_counter_cut++;
                                // printf("\n\tEntry %d, y_iBoard %d, iCh %d, abnormal energy %f keV\n", Entry, y_iBoard, iCh_other_A2s[another_iCh], energy_another_buf);
                            }
                        }
                        // if(flag_other_counter_cut == 0)
                        if(true)
                        {
                            fH2Energy_PMTs->Fill(x_energy, y_energy);
                            fH1EnergySpectra[0]->Fill(x_energy);
                            fH1EnergySpectra[1]->Fill(y_energy);
                            fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                            fH1TriggerTimes[0]->Fill(fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]]);
                            fH1TriggerTimes[1]->Fill(fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]]);
                            counter++;
                        }
                    }
                }
            }
        }

        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");

    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gStyle->SetOptStat(1);
    fH1EnergySpectra[2]->Draw();
    canvas->cd(5);
    fH1TriggerTimes[0]->Draw();
    canvas->cd(6);
    fH1TriggerTimes[1]->Draw();

    canvas->cd(1);
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kBlack);
    line->SetLineWidth(2);
    line->Draw("SAME");

    canvas_S1_energy->cd();
    fH1EnergySpectrum->Draw();
    canvas_S1_energy->Update();

    // canvas->ResizePad();
    canvas->Update();

    

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + "_fH2Energy_PMTs.png";
    TString filename_figure_pdf_S1 = filename_figure + "_energy_S1.pdf";
    TString filename_figure_png_S1 = filename_figure + "_energy_S1.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    IfFile_duplication(folderPath, filename_figure_pdf_S1);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf_S1.Data()));

    IfFile_duplication(folderPath, filename_figure_png_S1);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png_S1.Data()));

    for(Int_t another_iCh=0; another_iCh<3; another_iCh++){
        printf("\tother GSO counter %d\n", iCh_other_A2s[another_iCh]);
    }
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    return counter;
}

void DRS4Ana::PlotTrigger(){
    TCanvas* c1 = new TCanvas("title","name",1200,6000);
    c1->Divide(2,4);
    TH1D* hists[2][4];
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime;

    // ヒストグラムの初期化
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t ich=0; ich<4; ich++){
            hists[iBoard][ich] = new TH1D(Form("ib%d_ic%d_Trigger", iBoard, ich),
                                          Form("Trigger_ib%d_ic%d", iBoard, ich),
                                          1101, 0, 1100);
            hists[iBoard][ich]->SetXTitle("[ns]");
            hists[iBoard][ich]->SetYTitle("[counts]");
        }
    }

    // データ取得 & ヒストグラムに Fill
    for(Long64_t Entry=0; Entry<100000; Entry++){
        fChain->GetEntry(Entry);

        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t ich=0; ich<4; ich++){
                DiscriTime = fTime[iBoard][ich][fDiscriCell[iBoard][ich]];
                hists[iBoard][ich]->Fill(DiscriTime);
            }
        }
    }

    // 描画
    c1->cd();
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t ich=0; ich<4; ich++){
            c1->cd(iBoard*4+ich+1);
            hists[iBoard][ich]->Draw();
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    c1->Update();
    gPad->WaitPrimitive();
}

void DRS4Ana::PlotDiscriADC(Int_t iBoard = 0, Int_t iCh = 0){
    TCanvas* c2 = new TCanvas("c2", "DiscriTime Range", 1200, 1500);
    c2->Divide(2, 5);  // 10 分割

    // 5つの範囲 × 2 種類のデータを保存するヒストグラム
    TH1D* hists[2][5];
    for (Int_t i = 0; i < 5; i++) {
        hists[0][i] = new TH1D(Form("Trigger_%dto%d",100 + 25 * i , 100 + 25 * (i+1)),
                                   Form("Trigger_%dto%d",100 + 25 * i , 100 + 25 * (i+1)),
                                   1400, 0, 1400);
        hists[0][i]->SetXTitle("Time [ns]");
        hists[0][i]->SetYTitle("counts");

        hists[1][i] = new TH1D(Form("PlotEnergy_%dto%d",100 + 25 * i , 100 + 25 * (i+1)),
                                   Form("PlotEnergy_%dto%d",100 + 25 * i , 100 + 25 * (i+1)),
                                   650, 0, 650);
        hists[1][i]->SetXTitle("Energy [keV]");
        hists[1][i]->SetYTitle("counts");
    }

    // エネルギー較正データの読み込み
    TString key = "0204";
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);
    
    // データ取得 & フィル
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime, energy_buf;

    for (Long64_t Entry = 0; Entry < nentries; Entry++) {
        fChain->GetEntry(Entry);
        DiscriTime = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
        Double_t chargeIntegral = GetChargeIntegral(iBoard, iCh, 20, DiscriTime - 50, DiscriTime + 600);
        
        if (chargeIntegral > -9999.9) {
            energy_buf = p0[iBoard][iCh] + p1[iBoard][iCh] * (-chargeIntegral);
        }

        // 範囲による分岐
        if (100 <= DiscriTime && DiscriTime < 125) {
            hists[0][0]->Fill(DiscriTime);  
            hists[1][0]->Fill(energy_buf);
        } else if (125 <= DiscriTime && DiscriTime < 150) {
            hists[0][1]->Fill(DiscriTime);
            hists[1][1]->Fill(energy_buf);
        } else if (150 <= DiscriTime && DiscriTime < 175) {
            hists[0][2]->Fill(DiscriTime);
            hists[1][2]->Fill(energy_buf);
        } else if (175 <= DiscriTime && DiscriTime < 200) {
            hists[0][3]->Fill(DiscriTime);
            hists[1][3]->Fill(energy_buf);
        } else if (200 <= DiscriTime && DiscriTime < 225) {
            hists[0][4]->Fill(DiscriTime);
            hists[1][4]->Fill(energy_buf);
        }
    }

    // 描画（10個の `gPad` に分ける）
    for (Int_t i = 0; i < 5; i++) {
        for (Int_t j = 0; j < 2; j++) {
            c2->cd(i * 2 + j + 1);
            hists[j][i]->Draw();
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    c2->Update();
    gPad->WaitPrimitive();
}

Double_t DRS4Ana::DiscriAna(Int_t Scell = 100 , Int_t Fcell = 120){
    gErrorIgnoreLevel = kError; 
    TCanvas* c2 = new TCanvas("c2", "DiscriTime Range", 1200, 1500);
    c2->Divide(2, 3); 

    TH1D* hists[6];

    std::vector<std::string> strVec = {"huruno1", "huruno2", "sato"};

    for (Int_t i = 0; i < 3; i++) {
        hists[i] = new TH1D(Form("%s_Trigger_%dto%d",strVec[i].c_str(),Scell, Fcell),
                                   Form("%s_Trigger_%dto%d",strVec[i].c_str(),Scell,Fcell),
                                   301, 0, 300);
        hists[i]->SetXTitle("Time [ns]");
        hists[i]->SetYTitle("counts");
    }

    for (Int_t i = 3; i < 6; i++) {
        hists[i] = new TH1D(Form("%s_PlotEnergy_%dto%d",strVec[i%3].c_str(),Scell, Fcell),
                                   Form("%s_PlotEnergy_%dto%d",strVec[i%3].c_str(),Scell, Fcell),
                                   650, 0, 650);
        hists[i]->SetXTitle("Energy [keV]");
        hists[i]->SetYTitle("counts");
    }

        // エネルギー較正データの読み込み
    TString key = "0204";
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);
    
    // データ取得 & フィル
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime1,DiscriTime2,DiscriTime3,energy_buf1,energy_buf2,energy_buf3;

    for (Long64_t Entry = 0; Entry < nentries; Entry++) {
        fChain->GetEntry(Entry);
        DiscriTime1 = fTime[0][0][fDiscriCell[0][0]];
        DiscriTime2 = fTime[0][3][fDiscriCell[0][3]];
        DiscriTime3 = fTime[0][2][fDiscriCell[0][2]];
        Double_t chargeIntegral1 = GetChargeIntegral(0, 0, 20, DiscriTime1 - 50, DiscriTime1 + 600);
        Double_t chargeIntegral2 = GetChargeIntegral(0, 3, 20, DiscriTime2 - 50, DiscriTime2 + 600);
        Double_t chargeIntegral3 = GetChargeIntegral(0, 2, 20, DiscriTime3 - 50, DiscriTime3 + 600);

        if (chargeIntegral1 > -9999.9) {
            energy_buf1 = p0[0][0] + p1[0][0] * (-chargeIntegral1);
        }

        if (chargeIntegral2 > -9999.9) {
            energy_buf2 = p0[0][3] + p1[0][3] * (-chargeIntegral2);
        }

        if (chargeIntegral3 > -9999.9) {
            energy_buf3 = p0[0][2] + p1[0][2] * (-chargeIntegral3);
        }

        // 範囲による分岐
        if (Scell <= DiscriTime1 && DiscriTime1 < Fcell) {
            hists[0]->Fill(DiscriTime1);  
            hists[1]->Fill(DiscriTime2); 
            hists[2]->Fill(DiscriTime3); 
            hists[3]->Fill(energy_buf1);  
            hists[4]->Fill(energy_buf2); 
            hists[5]->Fill(energy_buf3); 
           
        }
    }




    for (Int_t i = 0; i < 6; i++) {
        c2->cd(i+1);
        hists[i]->Draw();
        gPad->SetGrid();
        gStyle->SetOptStat(1);
    }

    c2->Update();
    return nentries;
}

void DRS4Ana::DiscriAna2(Int_t Scell = 100 , Int_t Fcell = 120){

    TCanvas* c2 = new TCanvas("c2", "DiscriTime Range", 1200, 1500);
    c2->Divide(2, 3); 

    TH1D* hists[6];

    std::vector<std::string> strVec = {"huruno1", "huruno2", "sato"};

    for (Int_t i = 0; i < 3; i++) {
        hists[i] = new TH1D(Form("%s_Trigger_%dto%d",strVec[i].c_str(),Scell, Fcell),
                                   Form("%s_Trigger_%dto%d",strVec[i].c_str(),Scell,Fcell),
                                   101, 100, 200);
        hists[i]->SetXTitle("Time [ns]");
        hists[i]->SetYTitle("counts");
    }

    for (Int_t i = 3; i < 6; i++) {
        hists[i] = new TH1D(Form("%s_PlotEnergy_%dto%d",strVec[i%3].c_str(),Scell, Fcell),
                                   Form("%s_PlotEnergy_%dto%d",strVec[i%3].c_str(),Scell, Fcell),
                                   650, 0, 650);
        hists[i]->SetXTitle("Energy [keV]");
        hists[i]->SetYTitle("counts");
    }

        // エネルギー較正データの読み込み
    TString key = "0204";
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);
    
    // データ取得 & フィル
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime1,DiscriTime2,DiscriTime3,energy_buf1,energy_buf2,energy_buf3;

    for (Long64_t Entry = 0; Entry < nentries; Entry++) {
        fChain->GetEntry(Entry);
        DiscriTime1 = fTime[0][0][fDiscriCell[0][0]];
        DiscriTime2 = fTime[0][3][fDiscriCell[0][3]];
        DiscriTime3 = fTime[0][2][fDiscriCell[0][2]];
        Double_t chargeIntegral1 = GetChargeIntegral(0, 0, 20, DiscriTime1 - 50, DiscriTime1 + 600);
        Double_t chargeIntegral2 = GetChargeIntegral(0, 3, 20, DiscriTime2 - 50, DiscriTime2 + 600);
        Double_t chargeIntegral3 = GetChargeIntegral(0, 2, 20, DiscriTime3 - 50, DiscriTime3 + 600);

        if (chargeIntegral1 > -9999.9) {
            energy_buf1 = p0[0][0] + p1[0][0] * (-chargeIntegral1);
        }

        if (chargeIntegral2 > -9999.9) {
            energy_buf2 = p0[0][3] + p1[0][3] * (-chargeIntegral2);
        }

        if (chargeIntegral3 > -9999.9) {
            energy_buf3 = p0[0][2] + p1[0][2] * (-chargeIntegral3);
        }

        // 範囲による分岐
        if (Scell <= DiscriTime1 && DiscriTime1 < Fcell) {
            hists[0]->Fill(DiscriTime1);  
            hists[1]->Fill(DiscriTime2); 
            hists[2]->Fill(DiscriTime3); 
            hists[3]->Fill(energy_buf1);  
            hists[4]->Fill(energy_buf2); 
            hists[5]->Fill(energy_buf3); 
           
        }
    }




    for (Int_t i = 0; i < 6; i++) {
        c2->cd(i+1);
        hists[i]->Draw();
        gPad->SetGrid();
        gStyle->SetOptStat(1);
    }

    c2->Update();
}


void DRS4Ana::Discricut(){

    TCanvas* c2 = new TCanvas("c2", "DiscriTime Range", 1200, 1500);
    c2->Divide(2, 4); 

    TH1D* hists[8];

    std::vector<std::string> strVec = {"huruno1", "huruno2", "sato"};

    for (Int_t i = 0; i < 3; i++) {
        hists[i] = new TH1D(Form("%s_Trigger",strVec[i].c_str()),
                                   Form("%s_Trigger",strVec[i].c_str()),
                                   151, 0, 150);
        hists[i]->SetXTitle("DiscriCell");
        hists[i]->SetYTitle("counts");
    }

    for (Int_t i = 3; i < 6; i++) {
        hists[i] = new TH1D(Form("%s_PlotEnergy",strVec[i%3].c_str()),
                                   Form("%s_PlotEnergy",strVec[i%3].c_str()),
                                   650, 0, 650);
        hists[i]->SetXTitle("Energy [keV]");
        hists[i]->SetYTitle("counts");
    }

    for (Int_t i = 6; i < 8; i++) {
        hists[i] = new TH1D(Form("huruno1_huruno2_gap"),
                                   Form("huruno1_sato_gap"),
                                   101, 0, 100);
        hists[i]->SetXTitle("DiscriCell_gap");
        hists[i]->SetYTitle("counts");
    }

        // エネルギー較正データの読み込み
    TString key = "0204";
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);
    
    // データ取得 & フィル
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime1,DiscriTime2,DiscriTime3,energy_buf1,energy_buf2,energy_buf3,DiscriCell1,DiscriCell2,DiscriCell3;

    for (Long64_t Entry = 0; Entry < nentries; Entry++) {
        fChain->GetEntry(Entry);
        DiscriTime1 = fTime[0][0][fDiscriCell[0][0]];
        DiscriTime2 = fTime[0][3][fDiscriCell[0][3]];
        DiscriTime3 = fTime[0][2][fDiscriCell[0][2]];
        Double_t chargeIntegral1 = GetChargeIntegral(0, 0, 20, DiscriTime1 - 50, DiscriTime1 + 600);
        Double_t chargeIntegral2 = GetChargeIntegral(0, 3, 20, DiscriTime2 - 50, DiscriTime2 + 600);
        Double_t chargeIntegral3 = GetChargeIntegral(0, 2, 20, DiscriTime3 - 50, DiscriTime3 + 600);
        DiscriCell1=fDiscriCell[0][0];
        DiscriCell2=fDiscriCell[0][3];
        DiscriCell3=fDiscriCell[0][2];


        if (chargeIntegral1 > -9999.9) {
            energy_buf1 = p0[0][0] + p1[0][0] * (-chargeIntegral1);
        }

        if (chargeIntegral2 > -9999.9) {
            energy_buf2 = p0[0][3] + p1[0][3] * (-chargeIntegral2);
        }

        if (chargeIntegral3 > -9999.9) {
            energy_buf3 = p0[0][2] + p1[0][2] * (-chargeIntegral3);
        }


        Double_t gap_huruno1_huruno2 = DiscriCell1 - DiscriCell2;
        Double_t gap_huruno1_sato = DiscriCell1 - DiscriCell3;

        // 範囲による分岐
        // if (120 >= DiscriTime1 && DiscriTime1 >=50)  {
        //     if (DiscriTime2<=160 && DiscriTime2 >=50){
        //         if(DiscriTime3<=150 && DiscriTime3 >=50){
        //             hists[0]->Fill(DiscriTime1);  
        //             hists[1]->Fill(DiscriTime2); 
        //             hists[2]->Fill(DiscriTime3); 
        //             hists[3]->Fill(energy_buf1);  
        //             hists[4]->Fill(energy_buf2); 
        //             hists[5]->Fill(energy_buf3); 
        //         }
        //     }
        // }else if(140 >= DiscriTime1 && DiscriTime1 >120){
        //     if (DiscriTime2<=172 && DiscriTime2 >=50){
        //         if(DiscriTime3<=165 && DiscriTime3 >=50){
        //             hists[0]->Fill(DiscriTime1);  
        //             hists[1]->Fill(DiscriTime2); 
        //             hists[2]->Fill(DiscriTime3); 
        //             hists[3]->Fill(energy_buf1);  
        //             hists[4]->Fill(energy_buf2); 
        //             hists[5]->Fill(energy_buf3); 
        //         }
        //     }
        // }else if(160 >= DiscriTime1 && DiscriTime1 >140){
        //     if (DiscriTime2<=183 && DiscriTime2 >=50){
        //         if(DiscriTime3<=176 && DiscriTime3 >=50){
        //             hists[0]->Fill(DiscriTime1);  
        //             hists[1]->Fill(DiscriTime2); 
        //             hists[2]->Fill(DiscriTime3); 
        //             hists[3]->Fill(energy_buf1);  
        //             hists[4]->Fill(energy_buf2); 
        //             hists[5]->Fill(energy_buf3); 
        //         }
        //     }
        // }else if(180 >= DiscriTime1 && DiscriTime1 >160){
        //     if(DiscriTime3<=190 && DiscriTime3 >=50){
        //         hists[0]->Fill(DiscriTime1);  
        //         hists[1]->Fill(DiscriTime2); 
        //         hists[2]->Fill(DiscriTime3); 
        //         hists[3]->Fill(energy_buf1);  
        //         hists[4]->Fill(energy_buf2); 
        //         hists[5]->Fill(energy_buf3); 
        //     }
        // }else if(200 >= DiscriTime1 && DiscriTime1 >180){
        //     hists[0]->Fill(DiscriTime1);  
        //     hists[1]->Fill(DiscriTime2); 
        //     hists[2]->Fill(DiscriTime3); 
        //     hists[3]->Fill(energy_buf1);  
        //     hists[4]->Fill(energy_buf2); 
        //     hists[5]->Fill(energy_buf3); 

        // }
        if (DiscriCell1>0 && DiscriCell2>0 && DiscriCell3>0){   
        }else{
            continue;
        }



        if (21 >=gap_huruno1_sato  && gap_huruno1_sato >= -14) {
            if (18 >=gap_huruno1_huruno2  && gap_huruno1_huruno2 >= -22){

            }else{
                continue;
            }
        }else{
            continue;
        }

        hists[0]->Fill(DiscriCell1);
        hists[1]->Fill(DiscriCell2);
        hists[2]->Fill(DiscriCell3);
        hists[3]->Fill(energy_buf1);
        hists[4]->Fill(energy_buf2);
        hists[5]->Fill(energy_buf3);
        hists[6]->Fill(gap_huruno1_huruno2);
        hists[7]->Fill(gap_huruno1_sato);
    }




    for (Int_t i = 0; i < 8; i++) {
        c2->cd(i+1);
        hists[i]->Draw();
        gPad->SetGrid();
        gStyle->SetOptStat(1);
    }
    // waveform(nentries);

    c2->Update();
}

void DRS4Ana::waveform(Int_t nentries ){
    TCanvas *c1 = new TCanvas("title", "name", 1200, 6000);
    c1->Divide(2,4);
    TH2D* hists[2][4];
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t iCh=0; iCh<4; iCh++){
            hists[iBoard][iCh] = new TH2D(Form("title_ib%d_ic%d", iBoard, iCh), Form("name_ib%d_ic%d", iBoard, iCh), 500, 0, 1500, 500, -0.55, 0.05);
        }
    }
    Double_t discriTime;
    for(Int_t jentry=0; jentry<nentries; jentry++){
        fChain->GetEntry(jentry);
        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                    for(Int_t iCell=0; iCell<1024; iCell++){
                        hists[iBoard][iCh]->Fill(fTime[iBoard][iCh][iCell], fWaveform[iBoard][iCh][iCell]);   
                    }
            }
        }
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


}

Double_t DRS4Ana::Plot_discriCell_each_chain(Int_t nentries = 10000){
    /*
    DRSOscのdelayがいつしか1490 nsから1502 nsになっていた症状のデバッグ用。TChainの中のすべてのファイルについてdiscriCellを出力する。
    */
    TObjArray *fileElements = fChain->GetListOfFiles();
    TIter next(fileElements);
    TChainElement *chEl = nullptr;

    Int_t temp_discriCell[2][4];
    Double_t counter=0;

    while((chEl = (TChainElement*)next())){
        TString filename = chEl->GetTitle();
        std::cout << "file : " << filename << std::endl;

        TChain *tempChain = new TChain(fChain->GetName());
        tempChain->Add(filename);
        tempChain->SetBranchAddress("discriCell", temp_discriCell);

        TCanvas *canvas = new TCanvas("discriCell", Form("discriCell %s", filename.Data()), 12000, 8000);
        gStyle->SetLabelSize(0.08, "XYZ");  // 軸ラベルサイズ
        gStyle->SetTitleSize(0.10, "XYZ");  // 軸タイトルサイズ
        gStyle->SetTitleSize(0.1, "t"); // "t" はタイトル全体を指す
        gStyle->SetStatFontSize(0.10);  // 適宜サイズを調整

        canvas->Divide(2,4);
        TH1I* hist[2][4];
        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                TString histname = Form("%s : iBoard %d, iCh %d discriCell", filename.Data(), iBoard, iCh);
                hist[iBoard][iCh] = new TH1I(histname, histname, 256, 0, 255);
                hist[iBoard][iCh]->SetTitle(Form("%s;discriCell;count per %.2f cells", histname.Data(), 1.0));
            }
        }

        Int_t discriCell_buf;
        Int_t temp_nentries = tempChain->GetEntries();
        if(nentries < temp_nentries){
            temp_nentries = nentries;
        }
        for(Int_t EntryID=0; EntryID<temp_nentries; EntryID++){
            tempChain->GetEntry(EntryID);
            for(Int_t iBoard=0; iBoard<2; iBoard++){
                for(Int_t iCh=0; iCh<4; iCh++){
                    discriCell_buf = temp_discriCell[iBoard][iCh];
                    if(discriCell_buf > 10)
                    hist[iBoard][iCh]->Fill(discriCell_buf);
                }
            }
            if(EntryID % 5000 == 0){
                std::cout << Form("EntryID : %d", EntryID) << std::endl; 
            }
        }

        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                canvas->cd(iBoard*4+iCh+1);
                hist[iBoard][iCh]->Draw();
            }
        }

        //保存用のディレクトリを作る
        TString folderPath = Makedir_Date();
        TString filename_figure = filename(filename.Last('/')+1, filename.Length()-filename.Last('/'));
        filename_figure.ReplaceAll(".", "_");
        TString filename_figure_pdf = filename_figure + "_discriCell.pdf";
        TString filename_figure_png = filename_figure + "_discriCell.png";

        IfFile_duplication(folderPath, filename_figure_pdf);
        canvas->SaveAs(Form("%s/%s", folderPath.Data(),filename_figure_pdf.Data()));
        IfFile_duplication(folderPath, filename_figure_png);
        canvas->SaveAs(Form("%s/%s", folderPath.Data(),filename_figure_png.Data()));
        counter++;
    }

    return counter;
}


Double_t DRS4Ana::Plot_2Dhist_energy_with_cut6(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2){
    /*
    satoのdiscriCellのカットを強くした。
    */
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

    TCanvas *canvas = new TCanvas("canvas", "title", 4000, 4000);
    canvas->Divide(2,4);
    gStyle->SetLabelSize(0.08, "XYZ");  // 軸ラベルサイズ
    gStyle->SetTitleSize(0.10, "XYZ");  // 軸タイトルサイズ
    gStyle->SetTitleSize(0.1, "t"); // "t" はタイトル全体を指す
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;
    TH1D *fH1EnergySpectra[3];
    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", x_iBoard, x_iCh, key_Crystal_x.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, key_Crystal_x.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh, (maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 50, -50, 600, 50, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy of two PMTs (data from cfg/%s/data.txt), cut (S2 %d sigma, A2 %d sigma), cut (S1 %d sigma, A1 %d sigma);Board%d CH%d energy (keV);Board%d CH%d energy (keV)", key.Data(), nSigma_x_S2, nSigma_y_A2, nSigma_S1, nSigma_A1, x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    // fH2Energy_PMTs->Draw();

    TH1F* fH1TriggerTimes[2];
    fH1TriggerTimes[0] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", x_iBoard, x_iCh), 128, 0, 1023);
    fH1TriggerTimes[1] = new TH1F("trigger time", Form("iBoard %d iCh %d trigger time", y_iBoard, y_iCh), 128, 0, 1023);

    gPad->SetGrid();
    // gPad->SetLogz();
    gStyle->SetOptStat(0);

    TH2D *fH2DiscriCells[2];
    fH2DiscriCells[0] = new TH2D("fH2DiscriCell", Form("iBoard %d iCh %d vs iBoard %d iCh %d discriCell", 0,0,0,2), 200,0,199,200,0,199);
    fH2DiscriCells[1] = new TH2D("fH2DiscriCell", Form("iBoard %d iCh %d vs iBoard %d iCh %d discriCell(cut passed)", 0,0,0,2), 200,0,199,200,0,199);
    gPad->SetGrid();

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    //他のA2でヒットがないことをカット条件に入れたい。
    Int_t iCh_other_A2s[3];
    iCh_other_A2s[0] = (y_iCh + 1) % 4;
    iCh_other_A2s[1] = (y_iCh + 2) % 4;
    iCh_other_A2s[2] = (y_iCh + 3) % 4;

    Int_t S1_BoardID = 0;
    Int_t S1_ChID = 0;
    Int_t A1_BoardID = 0;
    Int_t A1_ChID = 2;
    Double_t discriTime_S1, discriTime_A1;
    Double_t x_energy, y_energy, x_error, y_error;
    Double_t x_charge_buf, y_charge_buf;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    Double_t DiscriTime_x, DiscriTime_y;
    Double_t adcSum_timerange_x, adcSum_timerange_y;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    Double_t distance_huruno1_sato_discriCell_line;
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

    canvas->cd(1);
    gPad->SetGrid();
    gStyle->SetPalette(kDeepSea);

    canvas->cd(2);
    gPad->SetGrid();

    canvas->cd(3);
    gPad->SetGrid();

    canvas->cd(4);
    gPad->SetGrid();

    canvas->cd(5);
    gPad->SetGrid();

    canvas->cd(6);
    gPad->SetGrid();

    canvas->cd(7);
    gPad->SetGrid();

    canvas->cd(8);
    gPad->SetGrid();

    Int_t cutCounter[8];
    for(Int_t Entry=0; Entry<nentries; Entry++)
    {
        fChain->GetEntry(Entry);

        //satoのdiscriCell分布の右の山を消す。huruno1,2も勝手にほぼ同時になってくれる。
        if(fDiscriCell[0][2]<135 && fDiscriCell[0][2]>115)
        {
            cutCounter[0] += 1;
            fH2DiscriCells[0]->Fill(fDiscriCell[0][0], fDiscriCell[0][2]);
            DiscriTime_x = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
            DiscriTime_y = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
            //huruno1とsatoのdiscriCellでのカット
            distance_huruno1_sato_discriCell_line = pow(fDiscriCell[0][0] - fDiscriCell[0][2] - 7, 2.0) / 2.0;
            if(distance_huruno1_sato_discriCell_line < 50.0)
            {
                cutCounter[1] += 1;
                if(100 < DiscriTime_y && DiscriTime_y < 220)
                {
                    cutCounter[2] += 1;
                    x_charge_buf = -GetChargeIntegral(x_iBoard, x_iCh, 20, DiscriTime_x - 50, DiscriTime_x + adcSum_timerange_x);
                    y_charge_buf = -GetChargeIntegral(y_iBoard, y_iCh, 20, DiscriTime_y - 50, DiscriTime_y + adcSum_timerange_y);

                    x_energy = x_p0_buf + x_p1_buf*x_charge_buf;
                    y_energy = y_p0_buf + y_p1_buf*y_charge_buf;
                    
                    if(abs(y_energy - (511.0 - x_energy)) < 0.01*(nSigma_y_A2 * y_p0_res_buf * sqrt(abs(511.0-x_energy)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_x_S2 * x_p0_res_buf * sqrt(abs(x_energy)) / (2.0*sqrt(2.0*log(2.0)))))
                    {
                        cutCounter[3] += 1;
                        if((x_energy > 50) && (y_energy > 50) && (x_energy < 400) && (y_energy < 400)){
                            cutCounter[4] += 1;
                            discriTime_S1 = fTime[S1_BoardID][S1_ChID][fDiscriCell[S1_BoardID][S1_ChID]];
                            discriTime_A1 = fTime[A1_BoardID][A1_ChID][fDiscriCell[A1_BoardID][A1_ChID]];
                            Double_t chargeIntegral_S1 = GetChargeIntegral(S1_BoardID , S1_ChID, 20.0, discriTime_S1 - 50, discriTime_S1 + 600);
                            Double_t chargeIntegral_A1 = GetChargeIntegral(A1_BoardID , A1_ChID, 20.0, discriTime_A1 - 50, discriTime_A1 + 600);

                            Double_t energy_S1 = p0[S1_BoardID][S1_ChID] + p1[S1_BoardID][S1_ChID]*(-chargeIntegral_S1);
                            Double_t energy_A1 = p0[A1_BoardID][A1_ChID] + p1[A1_BoardID][A1_ChID]*(-chargeIntegral_A1);

                            if((energy_A1 > 50) && (energy_S1 > 50) && (energy_A1< 400) && (energy_S1 < 400))
                            {
                                cutCounter[5] += 1;
                                if(abs(energy_A1 - (511.0 - energy_S1)) < 0.01*(nSigma_A1 * p0_res[A1_BoardID][A1_ChID] * sqrt(abs(511.0-energy_S1)) / (2.0*sqrt(2.0*log(2.0))) + nSigma_S1 * p0_res[S1_BoardID][S1_ChID] * sqrt(abs(energy_S1)) / (2.0*sqrt(2.0*log(2.0)))))
                                {
                                    cutCounter[6] += 1;
                                    Int_t flag_other_counter_cut = 0;
                                    Double_t chargeIntegral_another_A2s[3];
                                    // Double_t discriTime_other_A2s[3];
                                    // for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                    // {
                                    //     discriTime_other_A2s[another_iCh] = fTime[y_iBoard][iCh_other_A2s[another_iCh]][fDiscriCell[y_iBoard][iCh_other_A2s[another_iCh]]];
                                    // }
                                    for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                    {
                                        chargeIntegral_another_A2s[another_iCh] = GetChargeIntegral(y_iBoard, iCh_other_A2s[another_iCh], 20, DiscriTime_y - 50.0, DiscriTime_y + 180.0);
                                    }
                                    Double_t energy_other_A2s[3];
                                    for(Int_t another_iCh=0; another_iCh<3; another_iCh++)
                                    {
                                        Double_t energy_another_buf = p0[y_iBoard][iCh_other_A2s[another_iCh]] + p1[y_iBoard][iCh_other_A2s[another_iCh]] * (-chargeIntegral_another_A2s[another_iCh]);
                                        energy_other_A2s[another_iCh] = energy_another_buf;
                                        if(energy_another_buf > 100)
                                        {
                                            flag_other_counter_cut++;
                                            printf("\n\tEntry %d, y_iBoard %d, iCh %d, abnormal energy %f keV\n", Entry, y_iBoard, iCh_other_A2s[another_iCh], energy_another_buf);
                                        }
                                    }
                                    if(y_iBoard == 0 && y_iCh == 1){
                                        flag_other_counter_cut = 0;
                                    }
                                    if(flag_other_counter_cut == 0)
                                    {
                                        cutCounter[7] += 1;
                                        fH2DiscriCells[1]->Fill(fDiscriCell[0][0], fDiscriCell[0][2]);
                                        fH2Energy_PMTs->Fill(x_energy, y_energy);
                                        fH1EnergySpectra[0]->Fill(x_energy);
                                        fH1EnergySpectra[1]->Fill(y_energy);
                                        fH1EnergySpectra[2]->Fill(x_energy+y_energy);
                                        fH1TriggerTimes[0]->Fill(fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]]);
                                        fH1TriggerTimes[1]->Fill(fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]]);
                                        counter++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    
        if(Entry % 5000 == 0){
            printf("\tPoint processed : %d\n", Entry);
        }
    }
    canvas->cd(1);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();

    //カット範囲の図示
    TF1 *curve_upper = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_upper->SetLineColor(kBlue);
    curve_upper->SetLineWidth(1);
    curve_upper->Draw("SAME");
    TF1 *curve_lower = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower->SetParameters(y_p0_res_buf, x_p0_res_buf, nSigma_y_A2, nSigma_x_S2);
    curve_lower->SetLineColor(kBlue);
    curve_lower->SetLineWidth(1);
    curve_lower->Draw("SAME");

    TF1 *curve_upper_1sigma = new TF1("error curve", error_curve_upper, 0.0, 511.0, 4);
    curve_upper_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_upper_1sigma->SetLineColor(kBlack);
    curve_upper_1sigma->SetLineWidth(1);
    curve_upper_1sigma->SetLineStyle(2);
    curve_upper_1sigma->Draw("SAME");
    TF1 *curve_lower_1sigma = new TF1("error curve", error_curve_lower, 0.0, 511.0, 4);
    curve_lower_1sigma->SetParameters(y_p0_res_buf, x_p0_res_buf, 1, 1);
    curve_lower_1sigma->SetLineColor(kBlack);
    curve_lower_1sigma->SetLineWidth(1);
    curve_lower_1sigma->SetLineStyle(2);
    curve_lower_1sigma->Draw("SAME");

    canvas->cd(2);
    fH1EnergySpectra[1]->Draw();
    canvas->cd(3);
    fH1EnergySpectra[0]->Draw();
    canvas->cd(4);
    gStyle->SetOptStat(1);
    fH1EnergySpectra[2]->Draw();
    canvas->cd(5);
    fH1TriggerTimes[0]->Draw();
    canvas->cd(6);
    fH1TriggerTimes[1]->Draw();
    canvas->cd(7);
    gPad->SetLogz();
    fH2DiscriCells[0]->Draw();
    canvas->cd(8);
    gPad->SetLogz();
    fH2DiscriCells[1]->Draw();

    canvas->cd(1);
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kBlack);
    line->SetLineWidth(2);
    line->Draw("SAME");

    canvas->ResizePad();
    canvas->Update();

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + "_fH2Energy_PMTs.png";
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    for(Int_t another_iCh=0; another_iCh<3; another_iCh++){
        printf("\tother GSO counter %d\n", iCh_other_A2s[another_iCh]);
    }
    
    printf("ibx icx iby icy counter : %d %d %d %d %d\n",x_iBoard, x_iCh, y_iBoard, y_iCh, static_cast<Int_t>(counter));
    for(Int_t cutIndex=0; cutIndex<sizeof(cutCounter)/sizeof(Int_t); cutIndex++){
        printf("\tcut %d : survived %d events\n", cutIndex+1, cutCounter[cutIndex]);
    }
    return counter;
}

void DRS4Ana::Discricut2(){
    TCanvas* c2 = new TCanvas("c2", "DiscriTime Range", 1200, 1500);
    c2->Divide(2, 3);

    TH1D* hists[6];

    std::vector<std::string> strVec = {"huruno1", "huruno2", "sato"};

    for (Int_t i = 0; i < 3; i++) {
        hists[i] = new TH1D(Form("%s_Trigger",strVec[i].c_str()),
                                   Form("%s_Trigger",strVec[i].c_str()),
                                   201, 50, 250);
        hists[i]->SetXTitle("Time [ns]");
        hists[i]->SetYTitle("counts");
    }

    for (Int_t i = 3; i < 6; i++) {
        hists[i] = new TH1D(Form("%s_PlotEnergy",strVec[i%3].c_str()),
                                   Form("%s_PlotEnergy",strVec[i%3].c_str()),
                                   650, 0, 650);
        hists[i]->SetXTitle("Energy [keV]");
        hists[i]->SetYTitle("counts");
    }

        // エネルギー較正データの読み込み
    TString key = "0204";
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);
    
    // データ取得 & フィル
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime1,DiscriTime2,DiscriTime3,energy_buf1,energy_buf2,energy_buf3;

    for (Long64_t Entry = 0; Entry < nentries; Entry++) {
        fChain->GetEntry(Entry);
        DiscriTime1 = fTime[0][0][fDiscriCell[0][0]];
        DiscriTime2 = fTime[0][3][fDiscriCell[0][3]];
        DiscriTime3 = fTime[0][2][fDiscriCell[0][2]];
        Double_t chargeIntegral1 = GetChargeIntegral(0, 0, 20, DiscriTime1 - 50, DiscriTime1 + 600);
        Double_t chargeIntegral2 = GetChargeIntegral(0, 3, 20, DiscriTime2 - 50, DiscriTime2 + 600);
        Double_t chargeIntegral3 = GetChargeIntegral(0, 2, 20, DiscriTime3 - 50, DiscriTime3 + 600);

        if (chargeIntegral1 > -9999.9) {
            energy_buf1 = p0[0][0] + p1[0][0] * (-chargeIntegral1);
        }

        if (chargeIntegral2 > -9999.9) {
            energy_buf2 = p0[0][3] + p1[0][3] * (-chargeIntegral2);
        }

        if (chargeIntegral3 > -9999.9) {
            energy_buf3 = p0[0][2] + p1[0][2] * (-chargeIntegral3);
        }

        // 範囲による分岐
        if (120 >= DiscriTime1 && DiscriTime1 >= 50) {
            if (!(DiscriTime2 <= 160 && DiscriTime2 >= 50)) continue;
            if (!(DiscriTime3 <= 150 && DiscriTime3 >= 50)) continue;
        } else if (140 >= DiscriTime1 && DiscriTime1 > 120) {
            if (!(DiscriTime2 <= 172 && DiscriTime2 >= 50)) continue;
            if (!(DiscriTime3 <= 165 && DiscriTime3 >= 50)) continue;
        } else if (160 >= DiscriTime1 && DiscriTime1 > 140) {
            if (!(DiscriTime2 <= 183 && DiscriTime2 >= 50)) continue;
            if (!(DiscriTime3 <= 176 && DiscriTime3 >= 50)) continue;
        } else if (180 >= DiscriTime1 && DiscriTime1 > 160) {
            if (!(DiscriTime3 <= 190 && DiscriTime3 >= 50)) continue;
        } else if (!(200 >= DiscriTime1 && DiscriTime1 > 180)) {
            continue;
        }

        hists[0]->Fill(DiscriTime1);
        hists[1]->Fill(DiscriTime2);
        hists[2]->Fill(DiscriTime3);
        hists[3]->Fill(energy_buf1);
        hists[4]->Fill(energy_buf2);
        hists[5]->Fill(energy_buf3);



        
    }




    for (Int_t i = 0; i < 6; i++) {
        c2->cd(i+1);
        hists[i]->Draw();
        gPad->SetGrid();
        gStyle->SetOptStat(1);
    }
    waveform(nentries);

    c2->Update();


}

void DRS4Ana::Energy_fit(Int_t iBoard=0 , Int_t iCh=0 ,Int_t xMin=0, Int_t xMax=650, Int_t fitRangeMin=450 , Int_t fitRangeMax=580){
    TCanvas* c2 = new TCanvas("c2", "DiscriTime Range", 1200, 1500);

    TH1D* hist;
    hist = new TH1D(Form("PlotEnergy_iBoard%d_iCh%d",iBoard,iCh),
                                   Form("PlotEnergy_iBoard%d_iCh%d",iBoard,iCh),
                                    1000, xMin, xMax);
    hist->SetXTitle("Energy [keV]");
    hist->SetYTitle("counts");
        // エネルギー較正データの読み込み
    TString key = "0204";
    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);
    
    // データ取得 & フィル
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime1,DiscriTime2,DiscriTime3,energy_buf1,energy_buf2,energy_buf3,energy_buf;

    for (Long64_t Entry = 0; Entry < nentries; Entry++) {
        fChain->GetEntry(Entry);
        DiscriTime1 = fTime[0][0][fDiscriCell[0][0]];
        DiscriTime2 = fTime[0][3][fDiscriCell[0][3]];
        DiscriTime3 = fTime[0][2][fDiscriCell[0][2]];
        Double_t chargeIntegral1 = GetChargeIntegral(0, 0, 20, DiscriTime1 - 50, DiscriTime1 + 600);
        Double_t chargeIntegral2 = GetChargeIntegral(0, 3, 20, DiscriTime2 - 50, DiscriTime2 + 600);
        Double_t chargeIntegral3 = GetChargeIntegral(0, 2, 20, DiscriTime3 - 50, DiscriTime3 + 600);
        energy_buf = 0;
        if (chargeIntegral1 > -9999.9) {
            energy_buf1 = p0[0][0] + p1[0][0] * (-chargeIntegral1);
        }

        if (chargeIntegral2 > -9999.9) {
            energy_buf2 = p0[0][3] + p1[0][3] * (-chargeIntegral2);
        }

        if (chargeIntegral3 > -9999.9) {
            energy_buf3 = p0[0][2] + p1[0][2] * (-chargeIntegral3);
        }

        // 範囲による分岐
        if (120 >= DiscriTime1 && DiscriTime1 >= 50) {
            if (!(DiscriTime2 <= 160 && DiscriTime2 >= 50)) continue;
            if (!(DiscriTime3 <= 150 && DiscriTime3 >= 50)) continue;
        } else if (140 >= DiscriTime1 && DiscriTime1 > 120) {
            if (!(DiscriTime2 <= 172 && DiscriTime2 >= 50)) continue;
            if (!(DiscriTime3 <= 165 && DiscriTime3 >= 50)) continue;
        } else if (160 >= DiscriTime1 && DiscriTime1 > 140) {
            if (!(DiscriTime2 <= 183 && DiscriTime2 >= 50)) continue;
            if (!(DiscriTime3 <= 176 && DiscriTime3 >= 50)) continue;
        } else if (180 >= DiscriTime1 && DiscriTime1 > 160) {
            if (!(DiscriTime3 <= 190 && DiscriTime3 >= 50)) continue;
        } else if (!(200 >= DiscriTime1 && DiscriTime1 > 180)) {
            continue;
        }

        if (iCh==0){
            energy_buf=energy_buf1;
        }else if(iCh==3){
            energy_buf=energy_buf2;
        }else if(iCh==2){
            energy_buf=energy_buf3;
        }

        hist->Fill(energy_buf);
    }

    
    c2->cd();
    hist->Draw();
    gPad->SetGrid();
    gStyle->SetOptStat(1);
    c2->Update();


}

void DRS4Ana::PlotTrigger2(){
    TCanvas* c1 = new TCanvas("title","name",1200,6000);
    c1->Divide(2,4);
    TH1D* hists[2][4];
    Long64_t nentries = fChain->GetEntriesFast();
    Double_t DiscriTime;

    // ヒストグラムの初期化
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t ich=0; ich<4; ich++){
            hists[iBoard][ich] = new TH1D(Form("ib%d_ic%d_Trigger", iBoard, ich),
                                          Form("Trigger Timing_ib%d_ic%d", iBoard, ich),
                                          300, 1, 300);
            hists[iBoard][ich]->SetXTitle("trigger timing [ns]");
            hists[iBoard][ich]->SetYTitle("counts / 1 ns");
            hists[iBoard][ich]->GetXaxis()->SetTitleSize(0.05);
            hists[iBoard][ich]->GetYaxis()->SetTitleSize(0.06);
            hists[iBoard][ich]->GetXaxis()->SetTitleOffset(0.8);
            hists[iBoard][ich]->GetYaxis()->SetTitleOffset(0.9);
        }
    }

    // データ取得 & ヒストグラムに Fill
    for(Long64_t Entry=0; Entry<300000; Entry++){
        fChain->GetEntry(Entry);

        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t ich=0; ich<4; ich++){
                DiscriTime = fTime[iBoard][ich][fDiscriCell[iBoard][ich]];
                hists[iBoard][ich]->Fill(DiscriTime);
            }
        }
    }

    // 描画
    c1->cd();
    for(Int_t iBoard=0; iBoard<2; iBoard++){
        for(Int_t ich=0; ich<4; ich++){
            c1->cd(iBoard*4+ich+1);
            hists[iBoard][ich]->Draw();
            gPad->SetGrid();
            gStyle->SetOptStat(0);
        }
    }

    c1->Update();
    gPad->WaitPrimitive();
}

void DRS4Ana::PlotdiscriTime_difference(Int_t iBoard1 = 0, Int_t iCh1 = 0, Int_t iBoard2 = 0, Int_t iCh2 = 2, Int_t fit_flag = 1, Double_t fit_min = -5.0, Double_t fit_max = 25.0, Double_t sigma = 2.0){

    // Int_t fit_flag = 1; // フィットするなら1,しないなら0 描画範囲を決める
    
    // Long64_t nentries = fChain->GetEntriesFast();
    Long64_t nentries = 100000;

    if(fH1TriggerTimeDifference != NULL){
        delete fH1TriggerTimeDifference;
    }

    TCanvas *c1 = new TCanvas("c1", Form("(Board%d:ch%d) - (Board%d:ch%d) discriTime_difference", iBoard1, iCh1, iBoard2, iCh2), 1600, 1200);
    c1->Draw();
    gPad->SetGrid();

    Int_t histDiv, xmin, xmax;

    if(fit_flag == 0){
        histDiv = 500;
        xmin = -250;
        xmax = 250;
    }

    if(fit_flag == 1){
        histDiv = 100;
        xmin = -50;
        xmax = 50;
    }
    
    fH1TriggerTimeDifference = new TH1F("fH1TriggerTimeDifference", Form("(Board%d:ch%d) - (Board%d:ch%d) discriTime_difference", iBoard1, iCh1, iBoard2, iCh2), histDiv, xmin, xmax);
    fH1TriggerTimeDifference->SetXTitle("[ns]");
    fH1TriggerTimeDifference->SetYTitle(Form("counts per %d ns", (xmax-xmin)/histDiv));

    Double_t discriTime1, discriTime2, Time_difference;

    for (Long64_t jentry = 0; jentry < nentries; jentry++){
        fChain->GetEntry(jentry);
        discriTime1 = fTime[iBoard1][iCh1][fDiscriCell[iBoard1][iCh1]];
        discriTime2 = fTime[iBoard2][iCh2][fDiscriCell[iBoard2][iCh2]];

        if(fDiscriCell[iBoard1][iCh1] > 3 && fDiscriCell[iBoard2][iCh2] > 3){
            Time_difference = discriTime1 - discriTime2;
            fH1TriggerTimeDifference->Fill(Time_difference);
        }
        
    }

    fH1TriggerTimeDifference->Draw();


    if(fit_flag == 1){

        TF1* gaussian_plus_pedestal = new TF1("gaussian_plus_pedestal", "gaus+pol0(3)", fit_min, fit_max);
        gaussian_plus_pedestal->SetParameters(10000, (fit_max + fit_min) / 2, sigma, 100.0);
        fH1TriggerTimeDifference->Fit(gaussian_plus_pedestal, "R");
        gaussian_plus_pedestal->Draw("LSAME");

        TF1* pedestal = new TF1("linear", "pol1", fit_min, fit_max);
        pedestal->SetParameters(
            gaussian_plus_pedestal->GetParameter(3), // 切片
            gaussian_plus_pedestal->GetParameter(4)  // 傾き
        );
        pedestal->SetLineColor(kGreen+1);
        pedestal->SetLineStyle(1);
        pedestal->Draw("LSAME");
    }


    c1->Update();
    gStyle->SetOptFit(1);
}

Double_t DRS4Ana::PlotDiscriCell_difference_NaIs(Int_t iBoard1 = 0, Int_t iCh1 = 0, Int_t iBoard2 = 0, Int_t iCh2 = 2, Int_t entry_flag = 0, Int_t figure_option = 0, Int_t xmin = -1050, Int_t xmax = 1050, Int_t ped_nega_lower = -130, Int_t ped_nega_upper = -30, Int_t ped_posi_lower = 30, Int_t ped_posi_upper = 130){
    
    // figure_option 1のときDRS4読み出し開始時にすでに波があるイベントをカット
    //               2のときDRS4読み出し開始時にすでに波があるイベントをカット、閾値を超えたビンだけ残して描画

    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
    fChain->SetBranchStatus("fSec",0);
    fChain->SetBranchStatus("fNanoSec",0);
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);

    Long64_t nentries;

    if(entry_flag ==0){
        nentries = 100000;
    }
    if(entry_flag ==1){
        nentries = fChain->GetEntriesFast();
    }

    if(fH1TriggerCellDifference != NULL){
        delete fH1TriggerCellDifference;
    }

    TCanvas *c1 = new TCanvas("c1", Form("(Board%d:ch%d) - (Board%d:ch%d) DiscriCell_difference", iBoard1, iCh1, iBoard2, iCh2), 1600, 1200);
    c1->Draw();
    gPad->SetGrid();

    Int_t bin_width = 1;
    Int_t histDiv = (xmax - xmin) / bin_width;
    
    fH1TriggerCellDifference = new TH1F("fH1TriggerCellDifference", Form("(Board%d:ch%d) - (Board%d:ch%d) DiscriCell_difference", iBoard1, iCh1, iBoard2, iCh2), 4200, -2100, 2100);
    fH1TriggerCellDifference->SetXTitle("DiscriCell difference");
    fH1TriggerCellDifference->SetYTitle(Form("Counts per %d Cells", (xmax-xmin)/histDiv));

    Double_t DiscriCell1, DiscriCell2, Cell_difference;

    Int_t pedestal_counts = 0;
    Double_t pedestal = 0.0;
    Double_t pedestal_sigma_counts = 0.0;
    Double_t pedestal_sigma = 0.0;

    for (Long64_t jentry = 0; jentry < nentries; jentry++){
        fChain->GetEntry(jentry);
        DiscriCell1 = fDiscriCell[iBoard1][iCh1];
        DiscriCell2 = fDiscriCell[iBoard2][iCh2];

        if(figure_option == 0){
            Cell_difference = DiscriCell1 - DiscriCell2;
            fH1TriggerCellDifference->Fill(Cell_difference);
        }

        if(figure_option == 1 || figure_option == 2){
            if(DiscriCell1 > 3 && DiscriCell2 > 3){
                Cell_difference = DiscriCell1 - DiscriCell2;
                fH1TriggerCellDifference->Fill(Cell_difference);
            }
        }

        if((Cell_difference >= ped_nega_lower && Cell_difference <= ped_nega_upper) || 
           (Cell_difference >= ped_posi_lower && Cell_difference <= ped_posi_upper)){
            pedestal_counts++;
        }
            
    }

    pedestal = static_cast<Double_t>(pedestal_counts) / ((ped_nega_upper - ped_nega_lower + 1) + (ped_posi_upper - ped_posi_lower + 1));

    Double_t x_minimum = fH1TriggerCellDifference->GetXaxis()->GetXmin();
    Int_t Bin_min = fH1TriggerCellDifference->FindBin(x_minimum);
    Double_t x_maximum = fH1TriggerCellDifference->GetXaxis()->GetXmax();
    Int_t Bin_max = fH1TriggerCellDifference->FindBin(x_maximum);
    Int_t Bin_counts = 0;
    Int_t Bin_ped_counts = 0;
    Double_t coef = 5.0;

    Int_t Bin_ped_nega_upper = fH1TriggerCellDifference->FindBin(ped_nega_upper);
    Int_t Bin_ped_nega_lower = fH1TriggerCellDifference->FindBin(ped_nega_lower);
    Int_t Bin_ped_posi_upper = fH1TriggerCellDifference->FindBin(ped_posi_upper);
    Int_t Bin_ped_posi_lower = fH1TriggerCellDifference->FindBin(ped_posi_lower);

    for (Long64_t bin_index_nega = Bin_ped_nega_lower; bin_index_nega <= Bin_ped_nega_upper; bin_index_nega++){
        Int_t bin_content_nega = fH1TriggerCellDifference->GetBinContent(bin_index_nega);
        pedestal_sigma_counts += pow(pedestal - bin_content_nega, 2);
        Bin_ped_counts++;
    }

    for (Long64_t bin_index_posi = Bin_ped_posi_lower; bin_index_posi <= Bin_ped_posi_upper; bin_index_posi++){
        Int_t bin_content_posi = fH1TriggerCellDifference->GetBinContent(bin_index_posi);
        pedestal_sigma_counts += pow(pedestal - bin_content_posi, 2);
        Bin_ped_counts++;
    }

    pedestal_sigma = sqrt(pedestal_sigma_counts / (Bin_ped_counts - 1));
    Double_t threshold = pedestal + coef * pedestal_sigma;

    Double_t x_minimum_wo_ped = 0.0;
    Double_t x_maximum_wo_ped = 0.0;

    if(figure_option == 2){
        for (Long64_t i = Bin_min; i <= Bin_max; i++){
            Int_t bin_content = fH1TriggerCellDifference->GetBinContent(i);
            if(bin_content - threshold >= 0){
                fH1TriggerCellDifference->SetBinContent(i, bin_content);
                Bin_counts++;
            }
            else{
                fH1TriggerCellDifference->SetBinContent(i, 0);
                Bin_counts++;
            }

            if(fH1TriggerCellDifference->GetBinContent(i) >= threshold &&
                fH1TriggerCellDifference->GetBinContent(i-1) >= threshold &&
                fH1TriggerCellDifference->GetBinContent(i-2) == 0){
                x_minimum_wo_ped = fH1TriggerCellDifference->GetBinLowEdge(i-1);
            }

            if(fH1TriggerCellDifference->GetBinContent(i) == 0 && 
                fH1TriggerCellDifference->GetBinContent(i-1) == 0 && 
                fH1TriggerCellDifference->GetBinContent(i-2) >= threshold){
                x_maximum_wo_ped = (xmax-xmin)/histDiv + fH1TriggerCellDifference->GetBinLowEdge(i-2);
            }
        }
    }

    fH1TriggerCellDifference->GetXaxis()->SetRangeUser(xmin, xmax);
    fH1TriggerCellDifference->Draw();

    c1->Update();

    TString folderPath = Makedir_Date();

    TString filename_figure_pdf = Form("chains456_disc_dif_NaIs_[%i][%i]-[%i][%i]_new.pdf", iBoard1, iCh1, iBoard2, iCh2);
    TString filename_figure_png = Form("chains456_disc_dif_NaIs_[%i][%i]-[%i][%i]_new.png", iBoard1, iCh1, iBoard2, iCh2);
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    std::cout << "pedestal = " << pedestal << std::endl;
    std::cout << "pedestal_sigma = " << pedestal_sigma << std::endl;
    if(figure_option == 2){
        std::cout << "x_minimum_wo_ped = " << x_minimum_wo_ped << std::endl;
        std::cout << "x_maximum_wo_ped = " << x_maximum_wo_ped << std::endl;
    }

    return fChain->GetEntriesFast();
}

Double_t DRS4Ana::PlotdiscriCell_difference_S2A2(Int_t iBoard1 = 0, Int_t iCh1 = 0, Int_t iBoard2 = 0, Int_t iCh2 = 2, Int_t entry_flag = 0, Int_t figure_option = 0, Int_t xmin = -1050, Int_t xmax = 1050, Int_t ped_nega_lower = -130, Int_t ped_nega_upper = -30, Int_t ped_posi_lower = 30, Int_t ped_posi_upper = 130){

    // figure_option 1のときDRS4読み出し開始時にすでに波があるイベントをカット
    //               2のときS1S2A1の同時性を条件に課す
    //               3のときS1S2A1の同時性を条件に課し、閾値を超えたビンだけ残して描画

    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
    fChain->SetBranchStatus("fSec",0);
    fChain->SetBranchStatus("fNanoSec",0);
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);

    Long64_t nentries;

    if(entry_flag ==0){
        nentries = 100000;
    }
    if(entry_flag ==1){
        nentries = fChain->GetEntriesFast();
    }

    if(fH1TriggerCellDifference != NULL){
        delete fH1TriggerCellDifference;
    }

    TCanvas *c1 = new TCanvas("c1", Form("(Board%d:ch%d) - (Board%d:ch%d) DiscriCell_difference", iBoard1, iCh1, iBoard2, iCh2), 1600, 1200);
    c1->Draw();
    gPad->SetGrid();

    Int_t bin_width = 1;
    Int_t histDiv = (xmax - xmin) / bin_width;
    
    fH1TriggerCellDifference = new TH1F("fH1TriggerCellDifference", Form("(Board%d:ch%d) - (Board%d:ch%d) DiscriCell_difference", iBoard1, iCh1, iBoard2, iCh2), 4200, -2100, 2100);
    fH1TriggerCellDifference->SetXTitle("DiscriCell difference");
    fH1TriggerCellDifference->SetYTitle(Form("Counts per %d Cells", (xmax-xmin)/histDiv));

    Double_t DiscriCell1, DiscriCell2, Cell_difference;
    Double_t DiscriCellS1, DiscriCellS2, DiscriCellA1;

    Int_t pedestal_counts = 0;
    Double_t pedestal = 0.0;
    Double_t pedestal_sigma_counts = 0.0;
    Double_t pedestal_sigma = 0.0;

    for (Long64_t jentry = 0; jentry < nentries; jentry++){
        fChain->GetEntry(jentry);
        DiscriCell1 = fDiscriCell[iBoard1][iCh1];
        DiscriCell2 = fDiscriCell[iBoard2][iCh2];
        DiscriCellS1 = fDiscriCell[0][0];
        DiscriCellS2 = fDiscriCell[0][3];
        DiscriCellA1 = fDiscriCell[0][2];


        if(figure_option == 0){
            Cell_difference = DiscriCell1 - DiscriCell2;
            fH1TriggerCellDifference->Fill(Cell_difference);
        }

        if(figure_option == 1){
            if(DiscriCell1 > 10 && DiscriCell2 > 10){
                Cell_difference = DiscriCell1 - DiscriCell2;
                fH1TriggerCellDifference->Fill(Cell_difference);
            }
        }

        if(figure_option == 2){
            if (DiscriCell1 > 10 && DiscriCell2 > 10 &&
                DiscriCellS1 - DiscriCellS2 >= -22 && DiscriCellS1 - DiscriCellS2 <= 18 &&
                DiscriCellS1 - DiscriCellA1 >= -14 && DiscriCellS1 - DiscriCellA1 <= 21){

                Cell_difference = DiscriCell1 - DiscriCell2;
                fH1TriggerCellDifference->Fill(Cell_difference);
            }
        }

        if(figure_option == 3){
            if (DiscriCell1 > 10 && DiscriCell2 > 10 &&
                DiscriCellS1 - DiscriCellS2 >= -22 && DiscriCellS1 - DiscriCellS2 <= 18 &&
                DiscriCellS1 - DiscriCellA1 >= -14 && DiscriCellS1 - DiscriCellA1 <= 21){

                Cell_difference = DiscriCell1 - DiscriCell2;
                fH1TriggerCellDifference->Fill(Cell_difference);
            }
        }

        if((Cell_difference >= ped_nega_lower && Cell_difference <= ped_nega_upper) || 
            (Cell_difference >= ped_posi_lower && Cell_difference <= ped_posi_upper)){
            pedestal_counts++;
        }
        
    }

    pedestal = static_cast<Double_t>(pedestal_counts) / ((ped_nega_upper - ped_nega_lower + 1) + (ped_posi_upper - ped_posi_lower + 1));

    Double_t x_minimum = fH1TriggerCellDifference->GetXaxis()->GetXmin();
    Int_t Bin_min = fH1TriggerCellDifference->FindBin(x_minimum);
    Double_t x_maximum = fH1TriggerCellDifference->GetXaxis()->GetXmax();
    Int_t Bin_max = fH1TriggerCellDifference->FindBin(x_maximum);
    Int_t Bin_counts = 0;
    Int_t Bin_ped_counts = 0;
    Double_t coef = 5.0;

    Int_t Bin_ped_nega_upper = fH1TriggerCellDifference->FindBin(ped_nega_upper);
    Int_t Bin_ped_nega_lower = fH1TriggerCellDifference->FindBin(ped_nega_lower);
    Int_t Bin_ped_posi_upper = fH1TriggerCellDifference->FindBin(ped_posi_upper);
    Int_t Bin_ped_posi_lower = fH1TriggerCellDifference->FindBin(ped_posi_lower);

    for (Long64_t bin_index_nega = Bin_ped_nega_lower; bin_index_nega <= Bin_ped_nega_upper; bin_index_nega++){
        Int_t bin_content_nega = fH1TriggerCellDifference->GetBinContent(bin_index_nega);
        pedestal_sigma_counts += pow(pedestal - bin_content_nega, 2);
        Bin_ped_counts++;
    }

    for (Long64_t bin_index_posi = Bin_ped_posi_lower; bin_index_posi <= Bin_ped_posi_upper; bin_index_posi++){
        Int_t bin_content_posi = fH1TriggerCellDifference->GetBinContent(bin_index_posi);
        pedestal_sigma_counts += pow(pedestal - bin_content_posi, 2);
        Bin_ped_counts++;
    }

    pedestal_sigma = sqrt(pedestal_sigma_counts / (Bin_ped_counts - 1));
    Double_t threshold = pedestal + coef * pedestal_sigma;

    Double_t x_minimum_wo_ped = 0.0;
    Double_t x_maximum_wo_ped = 0.0;

    if(figure_option == 3){
        for (Long64_t i = Bin_min; i <= Bin_max; i++){
            Int_t bin_content = fH1TriggerCellDifference->GetBinContent(i);
            if(bin_content - threshold >= 0){
                fH1TriggerCellDifference->SetBinContent(i, bin_content);
                Bin_counts++;
            }
            else{
                fH1TriggerCellDifference->SetBinContent(i, 0);
                Bin_counts++;
            }

            if(fH1TriggerCellDifference->GetBinContent(i) >= threshold &&
                fH1TriggerCellDifference->GetBinContent(i-1) >= threshold &&
                fH1TriggerCellDifference->GetBinContent(i-2) == 0){
                x_minimum_wo_ped = fH1TriggerCellDifference->GetBinLowEdge(i-1);
            }

            if(fH1TriggerCellDifference->GetBinContent(i) == 0 && 
                fH1TriggerCellDifference->GetBinContent(i-1) == 0 && 
                fH1TriggerCellDifference->GetBinContent(i-2) >= threshold){
                x_maximum_wo_ped = (xmax-xmin)/histDiv + fH1TriggerCellDifference->GetBinLowEdge(i-2);
            }
        }
    }
    
    fH1TriggerCellDifference->GetXaxis()->SetRangeUser(xmin, xmax);
    fH1TriggerCellDifference->Draw();

    c1->Update();

    TString folderPath = Makedir_Date();

    TString filename_figure_pdf = Form("chains456_disc_dif_S2A2_[%i][%i]-[%i][%i]_new.pdf", iBoard1, iCh1, iBoard2, iCh2);
    TString filename_figure_png = Form("chains456_disc_dif_S2A2_[%i][%i]-[%i][%i]_new.png", iBoard1, iCh1, iBoard2, iCh2);
    printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    c1->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));

    std::cout << "pedestal = " << pedestal << std::endl;
    std::cout << "pedestal_sigma = " << pedestal_sigma << std::endl;
    if(figure_option == 3){
        std::cout << "x_minimum_wo_ped = " << x_minimum_wo_ped << std::endl;
        std::cout << "x_maximum_wo_ped = " << x_maximum_wo_ped << std::endl;
    }

    return fChain->GetEntriesFast();
}

Double_t DRS4Ana::PlotEnergy2(TString key = "0204", Int_t iBoard = 0, Int_t iCh = 0, Double_t xmin = 0, Double_t xmax = 600.0){
    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
    fChain->SetBranchStatus("fSec",0);
    fChain->SetBranchStatus("fNanoSec",0);
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);

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
    std::cout << "xmin:" << " " <<xmin << std::endl;
    std::cout << "xmax:" << " " <<xmax << std::endl;


    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t counter = 0;

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

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], dummy1[2][4], dummy2[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, dummy1, dummy2);

    Double_t energy_buf, chargeIntegral;

    for (Long64_t jentry = 0; jentry < nentries; jentry++){
        fChain->GetEntry(jentry);
        chargeIntegral = fAdcSum_crystals[iBoard][iCh];
        if (chargeIntegral > -9999.9)
        {
            energy_buf = p0[iBoard+flag_SlaveOnly][iCh] + p1[iBoard+flag_SlaveOnly][iCh]*(-chargeIntegral);
            counter++;
            fH1ChargeIntegral->Fill(energy_buf);
        }
    }

    
    fH1ChargeIntegral->Draw();

    TF1* gaussian_plus_linear = new TF1("gaussian_plus_linear", "gaus+pol1(3)", 440, 580);
    gaussian_plus_linear->SetParameters(7000, 500, 1.0, 50.0, -5.0);
    fH1ChargeIntegral -> Fit(gaussian_plus_linear, "R");
    gaussian_plus_linear -> Draw("same");

    // TF1* gauss1 = new TF1("gauss1", "gaus", 440, 580);
    // gauss1->SetParameters(
    //     gaussian_plus_linear->GetParameter(0), // 振幅
    //     gaussian_plus_linear->GetParameter(1), // 中心
    //     gaussian_plus_linear->GetParameter(2)  // 幅
    // );
    // gauss1->SetLineColor(kOrange+7);
    // gauss1->SetLineStyle(1);
    // gauss1->Draw("LSAME");

    TF1* linear = new TF1("linear", "pol1", 440, 580);
    linear->SetParameters(
        gaussian_plus_linear->GetParameter(3), // 切片
        gaussian_plus_linear->GetParameter(4)  // 傾き
    );
    linear->SetLineColor(kGreen+1);
    linear->SetLineStyle(1);
    linear->Draw("same");



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


Double_t DRS4Ana::EventSelection2(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Double_t sigma4_S1 = 1.0, Double_t sigma5_S2 = 1.0, Double_t sigma6_GSO_lower = 1.0, Double_t sigma6_GSO_upper = 1.0, Double_t sigma7_sato_lower = 1.0, Double_t sigma7_sato_upper = 1.0, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut =true){
    
    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
    fChain->SetBranchStatus("fSec",0);
    fChain->SetBranchStatus("fNanoSec",0);
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);
    
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t allcounter = 0;
    Long64_t validcounter = 0;
    Long64_t timecutcounter = 0;
    Long64_t scattercutcounter = 0;

    TCanvas *canvas = new TCanvas("canvas", "EventSelection2", 4000, 3000);
    canvas->Divide(2,2);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;

    TString key_Crystal_y;

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Double_t x_energy, y_energy, S1_energy, A1_energy;
    Double_t x_error, y_error, S1_error, A1_error;
    Double_t x_error_upper, x_error_lower;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    Double_t y_upper, y_lower, A1_lower, A1_upper;

    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    
    Double_t x_DiscriTime, y_DiscriTime, S1_DiscriTime, A1_DiscriTime;
    Double_t x_DiscriCell, y_DiscriCell, S1_DiscriCell , A1_DiscriCell;
    Double_t x_adcSum_timerange, y_adcSum_timerange;

    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    Double_t S1_p0_res_buf = p0_res[0][0];
    Double_t A1_p0_res_buf = p0_res[0][2];

    Double_t S1_charge_buf, A1_charge_buf;
    Double_t x_charge_buf, y_charge_buf;

    Double_t S1_energy_vec[1], x_energy_vec[1];
    Double_t par_S1_lower[3] = {S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_lower};
    Double_t par_S1_upper[3] = {S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_upper};
    Double_t par_x_lower[3] = {x_p0_res_buf, y_p0_res_buf, sigma6_GSO_lower};
    Double_t par_x_upper[3] = {x_p0_res_buf, y_p0_res_buf, sigma6_GSO_upper};
    

    Int_t TimeCut, ScatterCut, EnergyCut, AllCut;
    bool cut0,cut1,cut2,cut3,cut4,cut5,cut6,cut7;
    /*
    cut0 : all DC > 0
    cut1 : S1, A1, simultaneously
    cut2 : S1, S1, simultaneously
    cut3 : S2, A2_any deg, simultaneously
    cut4 : 256 - 3sigma < E_S1 < 256 + 3sigma(これもっと強くして良いんじゃないか)
    cut5 : 170 - 3sigma < E_S2 < 340 + 3sigma(これももっと強くして良いんじゃないか)
    cut6 : 511 - 3(sqrt(sigma^2 + sigma^2)) < E_S2 + E_A2 < ::
    cut7 : s1,a1 energy
    */
    bool TimeCutPassed, ScatterCutPassed, EnergyCutPassed;

    TH1D *fH1EnergySpectra[3];

    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI", x_iBoard, x_iCh), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, (maxEnergy-minEnergy)/nBins));
    
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", 100, 0, 600);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh,(maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d Ch%d energy [keV];Board%d CH%d energy [keV]", key.Data(), x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);

    TF1 *curve_upper = new TF1("curve_upper", CurveUpper_y, 0.0, 511.0, 3);  // パラメータ数は 3
    TF1 *curve_lower = new TF1("curve_lower", CurveLower_y, 0.0, 511.0, 3); 

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        x_DiscriTime = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        y_DiscriTime = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        S1_DiscriTime = fTime[0][0][fDiscriCell[0][0]];
        A1_DiscriTime = fTime[0][2][fDiscriCell[0][2]];
        
        x_DiscriCell = fDiscriCell[x_iBoard][x_iCh];
        y_DiscriCell = fDiscriCell[y_iBoard][y_iCh];
        S1_DiscriCell = fDiscriCell[0][0];
        A1_DiscriCell = fDiscriCell[0][2];
        
        x_charge_buf = -fAdcSum_crystals[x_iBoard][x_iCh];
        y_charge_buf = -fAdcSum_crystals[y_iBoard][y_iCh];
        S1_charge_buf = -fAdcSum_crystals[0][0];
        A1_charge_buf = -fAdcSum_crystals[0][2];

        x_energy = x_p0_buf + x_p1_buf * x_charge_buf;
        y_energy = y_p0_buf + y_p1_buf * y_charge_buf;
        S1_energy = p0[0][0] + p1[0][0] * S1_charge_buf;
        A1_energy = p0[0][2] + p1[0][2] * A1_charge_buf;
 
        x_energy_vec[0] =  {x_energy};
        S1_energy_vec[0] = {S1_energy};

        S1_error = p0_res[0][0] * sqrt(256) * 0.01 / (2 * sqrt(2 * log(2)));
        
        x_error_upper = x_p0_res_buf * sqrt(340) * 0.01 / (2 * sqrt(2 * log(2)));
        x_error_lower = x_p0_res_buf * sqrt(170) * 0.01 / (2 * sqrt(2 * log(2)));

        A1_upper = CurveUpper_y(S1_energy_vec, par_S1_upper);
        A1_lower = CurveLower_y(S1_energy_vec, par_S1_lower);
        y_upper = CurveUpper_y(x_energy_vec, par_x_upper);
        y_lower = CurveLower_y(x_energy_vec, par_x_lower);


    // Time Cut
        cut0 =  (0 < S1_DiscriCell) && (0 < A1_DiscriCell) && (0 < x_DiscriCell) && (0 < y_DiscriCell);
        cut1 =  (-17 <= S1_DiscriCell - A1_DiscriCell) && (S1_DiscriCell - A1_DiscriCell <= 22);
        cut2 =  (-22 <= S1_DiscriCell - x_DiscriCell) && (S1_DiscriCell - x_DiscriCell <= 15);

        cut3 = false;
        if (y_iBoard == 0){
            cut3 =  (-2 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 22);
        }else if (y_iBoard == 1 && y_iCh == 0){
            cut3 =  (6 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 34);

        }else if (y_iBoard == 1 && y_iCh == 1){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 35);

        }else if (y_iBoard == 1 && y_iCh == 2){
            cut3 =  (10 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 39);

        }else if (y_iBoard == 1 && y_iCh == 3){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 33);
            }
        
    // Scatter Cut
        cut4 = ( 245.4 - sigma4_S1 * S1_error < S1_energy ) && (S1_energy < 268.9 + sigma4_S1 * S1_error);
        cut5 = ( 173.2 - sigma5_S2 * x_error_lower < x_energy ) && (x_energy < 281.5 + sigma5_S2 * x_error_upper);

    // Energy Cut
        cut6 = (y_lower <= y_energy && y_energy <= y_upper);
        cut7 = (A1_lower <= A1_energy && A1_energy <= A1_upper);

        TimeCutPassed = !applyTimeCut || (cut0 && cut1 && cut2 && cut3);
        ScatterCutPassed = !applyScatterCut || (cut4 && cut5);
        EnergyCutPassed = !applyEnergyCut || (cut6 && cut7);

        // if (TimeCutPassed && ScatterCutPassed && EnergyCutPassed) {
        // fH2Energy_PMTs->Fill(x_energy, y_energy);
        // fH1EnergySpectra[0]->Fill(x_energy);
        // fH1EnergySpectra[1]->Fill(y_energy);
        // fH1EnergySpectra[2]->Fill(x_energy + y_energy);
        // }
        if (TimeCutPassed){
            timecutcounter++;
            if(ScatterCutPassed){
                scattercutcounter++;
            
                if(EnergyCutPassed){
                    validcounter++;
                    fH2Energy_PMTs->Fill(x_energy, y_energy);
                    fH1EnergySpectra[0]->Fill(x_energy);
                    fH1EnergySpectra[1]->Fill(y_energy);
                    fH1EnergySpectra[2]->Fill(x_energy + y_energy);
                }
            }
            
        }

        if(Entry % 5000 == 0){
            printf("\tPoint plot : %d\n", Entry);
        }
        allcounter++;

    }



// Pad1: 2Dヒストグラム
    canvas->cd(1);
    gPad->SetLogz();
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kRainBow);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kRed);
    line->SetLineWidth(2);
    line->Draw("SAME");

    // Double_t x1 = 194.2 - sigma5_S2 * x_error_lower;
    // Double_t x2 = 267.8 + sigma5_S2 * x_error_upper;
    Double_t x1 = 173.2 - sigma5_S2 * x_error_lower;
    Double_t x2 = 281.5 + sigma5_S2 * x_error_upper;
    TLine *line1 = new TLine(x1, 0, x1, 600);
    TLine *line2 = new TLine(x2, 0, x2, 600);
    line1->SetLineStyle(2); //破線
    line2->SetLineStyle(2); //破線
    line1->SetLineColor(kBlack);
    line2->SetLineColor(kBlack);
    line1->SetLineWidth(2);
    line2->SetLineWidth(2);
    line1->Draw("SAME");
    line2->Draw("SAME");

    curve_upper->SetParameters(x_p0_res_buf, y_p0_res_buf, sigma6_GSO_upper);
    curve_upper->SetLineStyle(2);
    curve_upper->SetLineColor(kBlack);
    curve_upper->Draw("SAME");

    
    curve_lower->SetParameters(x_p0_res_buf, y_p0_res_buf, sigma6_GSO_lower);  // par[0]=x_p0_res_buf, par[1]=y_p0_res_buf
    curve_lower->SetLineStyle(2);
    curve_lower->SetLineColor(kBlack);
    curve_lower->Draw("SAME");

   
// Pad2: y_energy ヒストグラム
    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[1]->Draw();

// Pad3: x_energy ヒストグラム
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[0]->Draw();

// Pad4: x_energy + y_energy ヒストグラム
    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[2]->Draw();
    // hs->Draw("hist stack");
    // hs->GetHistogram()->Draw("axis same");
    // gPad->Update();
    // gPad->RedrawAxis();
    //hs->GetXaxis()->SetRangeUser(0, 600);  // 必要な範囲に設定
    //hs->GetYaxis()->SetRangeUser(0, 4000);
   
    canvas->Update();

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + Form("_EventSelection_y_iB%diC%d_.pdf", y_iBoard, y_iCh);
    TString filename_figure_png = filename_figure + Form("_EventSelection_y_iB%diC%d_.png", y_iBoard, y_iCh);
    // printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));
    
    
    std::cout << "all events : " << allcounter << std::endl;
    std::cout << "timecut events : " << timecutcounter << std::endl;
    std::cout << "scattercut events : " << scattercutcounter << std::endl;
    std::cout << "valid events :" << validcounter << std::endl;

    std::ofstream ofs;
    ofs.open("./output/ES2.txt", std::ios::app);
    auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
    std::tm local_tm = *std::localtime(&now_c);
    ofs << std::endl << "================================================================" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl << Form("x_iB %d,x_iC %d,y_iB %d,x_iC %d", x_iBoard, x_iCh, y_iBoard, y_iCh) << std::endl << "\tall events : " << allcounter << std::endl  << "\ttimecut events : " << timecutcounter << std::endl << "\tscattercut events : " << scattercutcounter << std::endl << "\tvalid events :" << validcounter << std::endl;
    ofs.close();

    return 0; //　これはreturn 0;でok
}


Double_t DRS4Ana::EventSelection2_eff(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Double_t sigma4_S1 = 1.0, Double_t sigma5_S2 = 1.0, Double_t sigma6_GSO_lower = 1.0, Double_t sigma6_GSO_upper = 1.0, Double_t sigma7_sato_lower = 1.0, Double_t sigma7_sato_upper = 1.0, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut =true){
    fChain->SetBranchStatus("fSec",0);
    fChain->SetBranchStatus("fNanoSec",0);
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);
    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
    
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t allcounter = 0;
    Long64_t validcounter = 0;
    Long64_t timecutcounter = 0;
    Long64_t scattercutcounter = 0;

    TCanvas *canvas = new TCanvas("canvas", "EventSelection2_eff", 4000, 3000);
    canvas->Divide(2,2);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;

    TString key_Crystal_y;

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Double_t x_energy, y_energy, S1_energy, A1_energy;
    Double_t x_error, y_error, S1_error_eff, A1_error;
    Double_t x_error_upper, x_error_lower;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    Double_t y_upper, y_lower, A1_lower, A1_upper;

    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    
    Double_t x_DiscriTime, y_DiscriTime, S1_DiscriTime, A1_DiscriTime;
    Double_t x_DiscriCell, y_DiscriCell, S1_DiscriCell , A1_DiscriCell;
    Double_t x_adcSum_timerange, y_adcSum_timerange;
    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    Double_t S1_p0_res_buf = p0_res[0][0];
    Double_t A1_p0_res_buf = p0_res[0][2];


    Double_t S1_charge_buf, A1_charge_buf;
    Double_t x_charge_buf, y_charge_buf;

    Double_t S1_energy_vec[1], x_energy_vec[1];
    Double_t par_S1_lower[3] = {S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_lower};
    Double_t par_S1_upper[3] = {S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_upper};
    Double_t par_x_lower[3] = {x_p0_res_buf, y_p0_res_buf, sigma6_GSO_lower};
    Double_t par_x_upper[3] = {x_p0_res_buf, y_p0_res_buf, sigma6_GSO_upper};

    Int_t TimeCut, ScatterCut, EnergyCut, AllCut;
    bool cut0,cut1,cut2,cut3,cut4_eff,cut5,cut6,cut7;
    /*
    cut0 : all DC > 0
    cut1 : S1, A1, simultaneously
    cut2 : S1, S1, simultaneously
    cut3 : S2, A2_any deg, simultaneously
    cut4 : 256 - 3sigma < E_S1 < 256 + 3sigma(これもっと強くして良いんじゃないか)
    cut5 : 170 - 3sigma < E_S2 < 340 + 3sigma(これももっと強くして良いんじゃないか)
    cut6 : 511 - 3(sqrt(sigma^2 + sigma^2)) < E_S2 + E_A2 < ::
    cut7 : s1,a1 energy
    */
    bool TimeCutPassed, ScatterCutPassed, EnergyCutPassed;

    TH1D *fH1EnergySpectra[3];

    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI", x_iBoard, x_iCh), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, (maxEnergy-minEnergy)/nBins));
    
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s", y_iBoard, y_iCh, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", y_iBoard, y_iCh, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", 100, 0, 600);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", x_iBoard, x_iCh, y_iBoard, y_iCh,(maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d Ch%d energy [keV];Board%d CH%d energy [keV]", key.Data(), x_iBoard, x_iCh, y_iBoard, y_iCh));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);

    TF1 *curve_upper = new TF1("curve_upper", CurveUpper_y, 0.0, 511.0, 3);  // パラメータ数は 2
    TF1 *curve_lower = new TF1("curve_lower", CurveLower_y, 0.0, 511.0, 3); 

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        x_DiscriTime = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        y_DiscriTime = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        S1_DiscriTime = fTime[0][0][fDiscriCell[0][0]];
        A1_DiscriTime = fTime[0][2][fDiscriCell[0][2]];
        
        x_DiscriCell = fDiscriCell[x_iBoard][x_iCh];
        y_DiscriCell = fDiscriCell[y_iBoard][y_iCh];
        S1_DiscriCell = fDiscriCell[0][0];
        A1_DiscriCell = fDiscriCell[0][2];
        
        x_charge_buf = -fAdcSum_crystals[x_iBoard][x_iCh];
        y_charge_buf = -fAdcSum_crystals[y_iBoard][y_iCh];
        S1_charge_buf = -fAdcSum_crystals[0][0];
        A1_charge_buf = -fAdcSum_crystals[0][2];
        
        x_energy = x_p0_buf + x_p1_buf * x_charge_buf;
        y_energy = y_p0_buf + y_p1_buf * y_charge_buf;
        S1_energy = p0[0][0] + p1[0][0]* S1_charge_buf;
        A1_energy = p0[0][2] + p1[0][2]* A1_charge_buf;

        x_energy_vec[0] =  {x_energy};
        S1_energy_vec[0] = {S1_energy};
    
        x_error_upper = x_p0_res_buf * sqrt(340) * 0.01 / (sqrt(2 * log(2))); //散乱カット用のエラー
        x_error_lower = x_p0_res_buf * sqrt(170) * 0.01 / (sqrt(2 * log(2)));
        S1_error_eff = p0_res[0][0] * sqrt(511) * 0.01 / (sqrt(2 * log(2))); //90°散乱カット用のエラー

        A1_upper = CurveUpper_y(S1_energy_vec, par_S1_upper);
        A1_lower = CurveLower_y(S1_energy_vec, par_S1_lower);
        y_upper = CurveUpper_y(x_energy_vec, par_x_upper);
        y_lower = CurveLower_y(x_energy_vec, par_x_lower);

 
    // Time Cut
        cut0 =  (0 < S1_DiscriCell) && (0 < x_DiscriCell) && (0 < y_DiscriCell);
        cut1 =  (-17 <= S1_DiscriCell - A1_DiscriCell) && (S1_DiscriCell - A1_DiscriCell <= 22);
        cut2 =  (-22 <= S1_DiscriCell - x_DiscriCell) && (S1_DiscriCell - x_DiscriCell <= 15);

        cut3 = false;
        if (y_iBoard == 0){
            cut3 =  (-2 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 22);
        }else if (y_iBoard == 1 && y_iCh == 0){
            cut3 =  (6 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 34);

        }else if (y_iBoard == 1 && y_iCh == 1){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 35);

        }else if (y_iBoard == 1 && y_iCh == 2){
            cut3 =  (10 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 39);

        }else if (y_iBoard == 1 && y_iCh == 3){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 33);
            }
        
    // Scatter Cut
        cut4_eff = ( 511 - sigma4_S1 * S1_error_eff < S1_energy ) && (S1_energy < 511 + sigma4_S1 * S1_error_eff);
        cut5 = ( 173.2 - sigma5_S2 * x_error_lower < x_energy ) && (x_energy < 281.5 + sigma5_S2 * x_error_upper);

    // Energy Cut
        cut6 = (y_lower <= y_energy && y_energy <= y_upper);
        cut7 = (A1_lower <= A1_energy && A1_energy <= A1_upper);

        TimeCutPassed = !applyTimeCut || (cut0 && cut2 && cut3);
        ScatterCutPassed = !applyScatterCut || (cut4_eff && cut5);
        EnergyCutPassed = !applyEnergyCut || (cut6);
        // if (TimeCutPassed && ScatterCutPassed && EnergyCutPassed) {
        // fH2Energy_PMTs->Fill(x_energy, y_energy);
        // fH1EnergySpectra[0]->Fill(x_energy);
        // fH1EnergySpectra[1]->Fill(y_energy);
        // fH1EnergySpectra[2]->Fill(x_energy + y_energy);
        // }
        if (TimeCutPassed){
            timecutcounter++;
            if(ScatterCutPassed){
                scattercutcounter++;
            
                if(EnergyCutPassed){
                    validcounter++;
                    fH2Energy_PMTs->Fill(x_energy, y_energy);
                    fH1EnergySpectra[0]->Fill(x_energy);
                    fH1EnergySpectra[1]->Fill(y_energy);
                    fH1EnergySpectra[2]->Fill(x_energy + y_energy);
                }
            }
            
        }

        if(Entry % 50000 == 0){
            printf("\tPoint plot : %d\n", Entry);
        }
        allcounter++;

    }


// 各パッドに描画する

// Pad1: 2Dヒストグラム
    canvas->cd(1);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kRainBow);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kRed);
    line->SetLineWidth(2);
    line->Draw("SAME");

    Double_t x1 = 173.2 - sigma5_S2 * x_error_lower;
    Double_t x2 = 281.5 + sigma5_S2 * x_error_upper;
    TLine *line1 = new TLine(x1, 0, x1, 600);
    TLine *line2 = new TLine(x2, 0, x2, 600);
    line1->SetLineStyle(2); //破線
    line2->SetLineStyle(2); //破線
    line1->SetLineColor(kBlack);
    line2->SetLineColor(kBlack);
    line1->SetLineWidth(2);
    line2->SetLineWidth(2);
    line1->Draw("SAME");
    line2->Draw("SAME");

    curve_upper->SetParameters(x_p0_res_buf, y_p0_res_buf, sigma6_GSO_upper); 
    curve_upper->SetLineColor(kBlack);
    curve_upper->SetLineStyle(2); //破線
    curve_upper->Draw("SAME");

    
    curve_lower->SetParameters(x_p0_res_buf, y_p0_res_buf, sigma6_GSO_lower);  // par[0]=x_p0_res_buf, par[1]=y_p0_res_buf
    curve_lower->SetLineColor(kBlack);
    curve_lower->SetLineStyle(2); //破線
    curve_lower->Draw("SAME");

   
// Pad2: y_energy ヒストグラム
    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[1]->Draw();

// Pad3: x_energy ヒストグラム
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[0]->Draw();

// Pad4: x_energy + y_energy ヒストグラム
    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[2]->Draw();
    // hs->Draw("hist stack");
    // hs->GetHistogram()->Draw("axis same");
    // gPad->Update();
    // gPad->RedrawAxis();
    //hs->GetXaxis()->SetRangeUser(0, 600);  // 必要な範囲に設定
    //hs->GetYaxis()->SetRangeUser(0, 4000);
   
    canvas->Update();

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    // TString filename_figure_pdf = filename_figure + "_fH2Energy_PMTs.pdf";
    TString filename_figure_png = filename_figure + Form("_EventSelection__eff_y_iB%diC%d_.png", y_iBoard, y_iCh);
    // printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    // IfFile_duplication(folderPath, filename_figure_pdf);
    // canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));
    
    
    std::cout << "all events : " << allcounter << std::endl;
    std::cout << "timecut events : " << timecutcounter << std::endl;
    std::cout << "scattercut events : " << scattercutcounter << std::endl;
    std::cout << "valid events :" << validcounter << std::endl;

    std::ofstream ofs;
    ofs.open("./output/ES2_eff.txt", std::ios::app);
    auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
    std::tm local_tm = *std::localtime(&now_c);
    ofs << std::endl << "================================================================" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl << Form("x_iB %d,x_iC %d,y_iB %d,x_iC %d", x_iBoard, x_iCh, y_iBoard, y_iCh) << std::endl << "\tall events : " << allcounter << std::endl  << "\ttimecut events : " << timecutcounter << std::endl << "\tscattercut events : " << scattercutcounter << std::endl << "\tvalid events :" << validcounter << std::endl;
    ofs.close();

    return 0;
}


Double_t DRS4Ana::EventSelection2_S1A1(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Double_t sigma4_S1 = 1.0, Double_t sigma5_S2 = 1.0, Double_t sigma6_GSO_lower = 1.0, Double_t sigma6_GSO_upper = 1.0, Double_t sigma7_sato_lower = 1.0, Double_t sigma7_sato_upper = 1.0, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut =true){
    
    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
    fChain->SetBranchStatus("fSec",0);
    fChain->SetBranchStatus("fNanoSec",0);
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);
    
    Long64_t nentries = fChain->GetEntriesFast();
    Long64_t allcounter = 0;
    Long64_t validcounter = 0;
    Long64_t timecutcounter = 0;
    Long64_t scattercutcounter = 0;

    TCanvas *canvas = new TCanvas("canvas", "EventSelection2_S1A1", 4000, 3000);
    canvas->Divide(2,2);
    if(fH2Energy_PMTs != NULL){
        delete fH2Energy_PMTs;
    }
    Double_t minEnergy, maxEnergy;
    Int_t nBins = 100;
    minEnergy = 0.0;
    maxEnergy = 600.0;

    TString key_Crystal_y;

    Double_t p0[2][4], p0e[2][4], p1[2][4], p1e[2][4], p0_res[2][4], p0e_res[2][4];
    Load_EnergycalbData(key, p0, p0e, p1, p1e, p0_res, p0e_res);

    Double_t x_energy, y_energy, S1_energy, A1_energy;
    Double_t x_error, y_error, S1_error, A1_error;
    Double_t x_error_upper, x_error_lower;
    Double_t x_p0_buf, y_p0_buf, x_p1_buf, y_p1_buf;
    Double_t y_upper, y_lower, A1_lower, A1_upper;

    x_p0_buf = p0[x_iBoard][x_iCh];
    x_p1_buf = p1[x_iBoard][x_iCh];
    y_p0_buf = p0[y_iBoard][y_iCh];
    y_p1_buf = p1[y_iBoard][y_iCh];
    
    Double_t x_DiscriTime, y_DiscriTime, S1_DiscriTime, A1_DiscriTime;
    Double_t x_DiscriCell, y_DiscriCell, S1_DiscriCell , A1_DiscriCell;
    Double_t x_adcSum_timerange, y_adcSum_timerange;

    Double_t x_p0_res_buf = p0_res[x_iBoard][x_iCh];
    Double_t y_p0_res_buf = p0_res[y_iBoard][y_iCh];
    Double_t S1_p0_res_buf = p0_res[0][0];
    Double_t A1_p0_res_buf = p0_res[0][2];

    Double_t S1_charge_buf, A1_charge_buf;
    Double_t x_charge_buf, y_charge_buf;

    Double_t S1_energy_vec[1], x_energy_vec[1];
    Double_t par_S1_lower[3] = {S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_lower};
    Double_t par_S1_upper[3] = {S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_upper};
    Double_t par_x_lower[3] = {x_p0_res_buf, y_p0_res_buf, sigma6_GSO_lower};
    Double_t par_x_upper[3] = {x_p0_res_buf, y_p0_res_buf, sigma6_GSO_upper};
    

    Int_t TimeCut, ScatterCut, EnergyCut, AllCut;
    bool cut0,cut1,cut2,cut3,cut4,cut5,cut6,cut7;
    /*
    cut0 : all DC > 0
    cut1 : S1, A1, simultaneously
    cut2 : S1, S1, simultaneously
    cut3 : S2, A2_any deg, simultaneously
    cut4 : 256 - 3sigma < E_S1 < 256 + 3sigma(これもっと強くして良いんじゃないか)
    cut5 : 170 - 3sigma < E_S2 < 340 + 3sigma(これももっと強くして良いんじゃないか)
    cut6 : 511 - 3(sqrt(sigma^2 + sigma^2)) < E_S2 + E_A2 < ::
    cut7 : s1,a1 energy
    */
    bool TimeCutPassed, ScatterCutPassed, EnergyCutPassed;

    TH1D *fH1EnergySpectra[3];

    fH1EnergySpectra[0] = new TH1D("fH1EnergySpectra", Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI", 0, 0), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[0]->SetTitle(Form("x-axis energy spectrum : iBoard %d, iCh %d, crystal NaI;energy [keV]; count per %.2f keV", 0, 0, (maxEnergy-minEnergy)/nBins));
    
    fH1EnergySpectra[1] = new TH1D("fH1EnergySpectra", Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s", 0, 2, key_Crystal_y.Data()), nBins, minEnergy, maxEnergy);
    fH1EnergySpectra[1]->SetTitle(Form("y-axis energy spectrum : iBoard %d, iCh %d, crystal %s;energy [keV]; count per %.2f keV", 0, 2, key_Crystal_y.Data(), (maxEnergy-minEnergy)/nBins));
    
    fH1EnergySpectra[2] = new TH1D("fH1EnergySpectra", "Sum energy spectrum", 100, 0, 600);
    fH1EnergySpectra[2]->SetTitle(Form("Sum energy spectrum : iBoard %d, iCh %d, and iBoard %d, iCh %d;energy [keV]; count per %.2f keV", 0, 0, 0, 2,(maxEnergy-minEnergy)/nBins));

    fH2Energy_PMTs = new TH2F("name", "title", 200, -50, 600, 200, -50, 600);
    fH2Energy_PMTs->SetTitle(Form("energy between two PMTs (data from cfg/%s/data.txt);Board%d Ch%d energy [keV];Board%d CH%d energy [keV]", key.Data(), 0, 0, 0, 2));
    canvas->cd(1);
    fH2Energy_PMTs->Draw();

    gPad->SetGrid();
    gPad->SetLogz();
    gStyle->SetOptStat(0);

    TF1 *curve_upper = new TF1("curve_upper", CurveUpper_y, 0.0, 511.0, 3);  // パラメータ数は 3
    TF1 *curve_lower = new TF1("curve_lower", CurveLower_y, 0.0, 511.0, 3); 

    for(Int_t Entry=0; Entry<nentries; Entry++){
        fChain->GetEntry(Entry);

        x_DiscriTime = fTime[x_iBoard][x_iCh][fDiscriCell[x_iBoard][x_iCh]];
        y_DiscriTime = fTime[y_iBoard][y_iCh][fDiscriCell[y_iBoard][y_iCh]];
        S1_DiscriTime = fTime[0][0][fDiscriCell[0][0]];
        A1_DiscriTime = fTime[0][2][fDiscriCell[0][2]];
        
        x_DiscriCell = fDiscriCell[x_iBoard][x_iCh];
        y_DiscriCell = fDiscriCell[y_iBoard][y_iCh];
        S1_DiscriCell = fDiscriCell[0][0];
        A1_DiscriCell = fDiscriCell[0][2];
        
        x_charge_buf = -fAdcSum_crystals[x_iBoard][x_iCh];
        y_charge_buf = -fAdcSum_crystals[y_iBoard][y_iCh];
        S1_charge_buf = -fAdcSum_crystals[0][0];
        A1_charge_buf = -fAdcSum_crystals[0][2];

        x_energy = x_p0_buf + x_p1_buf * x_charge_buf;
        y_energy = y_p0_buf + y_p1_buf * y_charge_buf;
        S1_energy = p0[0][0] + p1[0][0] * S1_charge_buf;
        A1_energy = p0[0][2] + p1[0][2] * A1_charge_buf;
 
        x_energy_vec[0] =  {x_energy};
        S1_energy_vec[0] = {S1_energy};

        S1_error = p0_res[0][0] * sqrt(256) * 0.01 / (2 * sqrt(2 * log(2)));
        
        x_error_upper = x_p0_res_buf * sqrt(340) * 0.01 / (2 * sqrt(2 * log(2)));
        x_error_lower = x_p0_res_buf * sqrt(170) * 0.01 / (2 * sqrt(2 * log(2)));

        A1_upper = CurveUpper_y(S1_energy_vec, par_S1_upper);
        A1_lower = CurveLower_y(S1_energy_vec, par_S1_lower);
        y_upper = CurveUpper_y(x_energy_vec, par_x_upper);
        y_lower = CurveLower_y(x_energy_vec, par_x_lower);


    // Time Cut
        cut0 =  (0 < S1_DiscriCell) && (0 < A1_DiscriCell) && (0 < x_DiscriCell) && (0 < y_DiscriCell);
        cut1 =  (-17 <= S1_DiscriCell - A1_DiscriCell) && (S1_DiscriCell - A1_DiscriCell <= 22);
        cut2 =  (-22 <= S1_DiscriCell - x_DiscriCell) && (S1_DiscriCell - x_DiscriCell <= 15);

        cut3 = false;
        if (y_iBoard == 0){
            cut3 =  (-2 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 22);
        }else if (y_iBoard == 1 && y_iCh == 0){
            cut3 =  (6 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 34);

        }else if (y_iBoard == 1 && y_iCh == 1){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 35);

        }else if (y_iBoard == 1 && y_iCh == 2){
            cut3 =  (10 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 39);

        }else if (y_iBoard == 1 && y_iCh == 3){
            cut3 =  (12 <= x_DiscriCell- y_DiscriCell) && (x_DiscriCell- y_DiscriCell <= 33);
            }
        
    // Scatter Cut
        cut4 = ( 243.4 - sigma4_S1 * S1_error < S1_energy ) && (S1_energy < 268.9 + sigma4_S1 * S1_error);
        cut5 = ( 173.2 - sigma5_S2 * x_error_lower < x_energy ) && (x_energy < 281.5 + sigma5_S2 * x_error_upper);

    // Energy Cut
        cut6 = (y_lower <= y_energy && y_energy <= y_upper);
        cut7 = (A1_lower <= A1_energy && A1_energy <= A1_upper);

        TimeCutPassed = !applyTimeCut || (cut0 && cut1 && cut2 && cut3);
        ScatterCutPassed = !applyScatterCut || (cut4 && cut5);
        EnergyCutPassed = !applyEnergyCut || (cut6 && cut7);

        // if (TimeCutPassed && ScatterCutPassed && EnergyCutPassed) {
        // fH2Energy_PMTs->Fill(x_energy, y_energy);
        // fH1EnergySpectra[0]->Fill(x_energy);
        // fH1EnergySpectra[1]->Fill(y_energy);
        // fH1EnergySpectra[2]->Fill(x_energy + y_energy);
        // }
        if (TimeCutPassed){
            timecutcounter++;
            if(ScatterCutPassed){
                scattercutcounter++;
            
                if(EnergyCutPassed){
                    validcounter++;
                    fH2Energy_PMTs->Fill(S1_energy, A1_energy);
                    fH1EnergySpectra[0]->Fill(S1_energy);
                    fH1EnergySpectra[1]->Fill(A1_energy);
                    fH1EnergySpectra[2]->Fill(S1_energy + A1_energy);
                }
            }
            
        }

        if(Entry % 5000 == 0){
            printf("\tPoint plot : %d\n", Entry);
        }
        allcounter++;

    }



// Pad1: 2Dヒストグラム
    canvas->cd(1);
    gPad->SetLogz();
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    gStyle->SetPalette(kSolar);
    gPad->Update();
    // gPad->SetBottomMargin(0.15);  // 下の余白を広げる
    fH2Energy_PMTs->Draw();
    TLine *line = new TLine(0, 511, 511,0);
    line->SetLineColor(kRed);
    line->SetLineWidth(2);
    line->Draw("SAME");

    // Double_t x1 = 194.2 - sigma5_S2 * x_error_lower;
    // Double_t x2 = 267.8 + sigma5_S2 * x_error_upper;
    Double_t x1 = 243.4 - sigma4_S1 * S1_error;
    Double_t x2 = 268.9 + sigma4_S1 * S1_error;
    TLine *line1 = new TLine(x1, 0, x1, 600);
    TLine *line2 = new TLine(x2, 0, x2, 600);
    line1->SetLineStyle(2); //破線
    line2->SetLineStyle(2); //破線
    line1->SetLineColor(kBlack);
    line2->SetLineColor(kBlack);
    line1->SetLineWidth(2);
    line2->SetLineWidth(2);
    line1->Draw("SAME");
    line2->Draw("SAME");

    curve_upper->SetParameters(S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_upper);
    curve_upper->SetLineStyle(2);
    curve_upper->SetLineColor(kBlack);
    curve_upper->Draw("SAME");

    
    curve_lower->SetParameters(S1_p0_res_buf, A1_p0_res_buf, sigma7_sato_lower);  // par[0]=x_p0_res_buf, par[1]=y_p0_res_buf
    curve_lower->SetLineStyle(2);
    curve_lower->SetLineColor(kBlack);
    curve_lower->Draw("SAME");

   
// Pad2: y_energy ヒストグラム
    canvas->cd(2);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[1]->Draw();

// Pad3: x_energy ヒストグラム
    canvas->cd(3);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[0]->Draw();

// Pad4: x_energy + y_energy ヒストグラム
    canvas->cd(4);
    gPad->SetLeftMargin(0.15);  // 左の余白を広げる
    gPad->SetGrid();
    fH1EnergySpectra[2]->Draw();
    // hs->Draw("hist stack");
    // hs->GetHistogram()->Draw("axis same");
    // gPad->Update();
    // gPad->RedrawAxis();
    //hs->GetXaxis()->SetRangeUser(0, 600);  // 必要な範囲に設定
    //hs->GetYaxis()->SetRangeUser(0, 4000);
   
    canvas->Update();

    //保存用のディレクトリを作る
    TString folderPath = Makedir_Date();

    TString filename_figure = fRootFile(fRootFile.Last('/')+1, fRootFile.Length()-fRootFile.Last('/'));
    filename_figure.ReplaceAll(".", "_");
    TString filename_figure_pdf = filename_figure + Form("_EventSelection_y_iB%diC%d_sato_.pdf", y_iBoard, y_iCh);
    TString filename_figure_png = filename_figure + Form("_EventSelection_y_iB%diC%d_sato_.png", y_iBoard, y_iCh);
    // printf("\n\tfigure saved as: %s/%s\n", folderPath.Data(), filename_figure_pdf.Data());

    IfFile_duplication(folderPath, filename_figure_pdf);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_pdf.Data()));

    IfFile_duplication(folderPath, filename_figure_png);
    canvas->SaveAs(Form("%s/%s", folderPath.Data(), filename_figure_png.Data()));
    
    
    std::cout << "all events : " << allcounter << std::endl;
    std::cout << "timecut events : " << timecutcounter << std::endl;
    std::cout << "scattercut events : " << scattercutcounter << std::endl;
    std::cout << "valid events :" << validcounter << std::endl;

    std::ofstream ofs;
    ofs.open("./output/ES2.txt", std::ios::app);
    auto now = std::chrono::system_clock::now();                      // 現在時刻を取得
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);    // time_t に変換
    std::tm local_tm = *std::localtime(&now_c);
    ofs << std::endl << "================================================================" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl << Form("x_iB %d,x_iC %d,y_iB %d,x_iC %d", x_iBoard, x_iCh, y_iBoard, y_iCh) << std::endl << "\tall events : " << allcounter << std::endl  << "\ttimecut events : " << timecutcounter << std::endl << "\tscattercut events : " << scattercutcounter << std::endl << "\tvalid events :" << validcounter << std::endl;
    ofs.close();

    return 0; //　これはreturn 0;でok
}


Double_t DRS4Ana::PlotRunPeriods(){
    fChain->SetBranchStatus("fTriggerCell",0);
    fChain->SetBranchStatus("adcSum",0);
    fChain->SetBranchStatus("waveform",0);
    fChain->SetBranchStatus("time",0);
    Long64_t nentries = fChain->GetEntries();
    std::cout << "nentries: " << nentries << std::endl;

    Long64_t counter = 0;
    if(fH1TriggerRate != NULL){
        delete fH1TriggerRate;
    }

    std::vector<TString> fileNames;
    std::vector<Int_t> eventCounts;
    std::vector<Double_t> daqDurations;
    std::vector<Double_t> eventRates;

    // ファイルリスト取得
    TObjArray *fileList = fChain->GetListOfFiles();
    if (!fileList) {
        std::cerr << "No files found in the TChain." << std::endl;
        return;
    }

    // 各ファイルごとの処理
    TIter next(fileList);
    TChainElement *element;
    Int_t EventInSec, EventInNanoSec;
    while ((element = (TChainElement *)next())) {
        const char *fileName = element->GetTitle();
        TFile file(fileName);
        if (file.IsZombie()) continue;

        TTree *tree = (TTree *)file.Get("treeDRS4BoardEvent");
        if (!tree) continue;

        tree->SetBranchAddress("fSec", &EventInSec);

        Long64_t nEntries = tree->GetEntries();
        if (nEntries == 0) continue;

        // fSec の最小・最大を取得
        tree->GetEntry(0);
        Int_t minTime = EventInSec;
        tree->GetEntry(nEntries - 1);
        Int_t maxTime = EventInSec;

        Double_t daqTime = maxTime - minTime; // DAQ稼働時間 [秒]
        Double_t rate = (daqTime > 0) ? (Double_t)nEntries / daqTime : 0.0; // イベントレート [Hz]

        // 結果を保存
        fileNames.push_back(fileName);
        eventCounts.push_back(nEntries);
        daqDurations.push_back(daqTime);
        eventRates.push_back(rate);

        file.Close();
    }

    // 結果を表示
    std::cout << "File Name\tEvents\tDAQ Time [s]\tRate [Hz]" << std::endl;
    for (size_t i = 0; i < fileNames.size(); ++i) {
        std::cout << fileNames[i] << "\t"
                  << eventCounts[i] << "\t"
                  << daqDurations[i] << "\t"
                  << eventRates[i] << std::endl;
    }

    // //秒数を60で割って、60sあたりのトリガー数を入れたい
    // fH1TriggerRate = new TH1F("fH1TriggerRate", Form("%s:ch%d_Trigger_Rate", fRootFile.Data(), iCh), howLong_DAQ_spent/60.0, 0, howLong_DAQ_spent);
    // fH1TriggerRate->SetXTitle("time [s]");
    // fH1TriggerRate->SetYTitle("[counts]/1min");

    // for (Long64_t jentry = 0; jentry < nentries; jentry++)
    // {
    //     fChain->GetEntry(jentry);
    //     fH1TriggerRate->Fill(-eventTime_begin_InSec+fEventTimeInSec+fEventTimeInNanoSec*10e-9);
    //     counter++;
    // }
    // fH1TriggerRate->Draw();
    return(counter);
}
