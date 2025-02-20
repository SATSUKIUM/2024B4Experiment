/*======================================================================================================
 Name:           DRS4Ana.h
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

#ifndef DRS4Ana_h
#define DRS4Ana_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TString.h>

// Header file for the classes stored in the TTree if any.
#include "TTimeStamp.h"

#include <vector>

#define NUM_OF_BOARDS 2

extern std::vector<TString> fRootFile_pars;

class DRS4Ana
{
public:
    static TString fRootFile;
    TChain *fChain;  //! pointer to the analyzed TTree or TChain
    Int_t fCurrent; //! current Tree number in a TChain

    // Fixed size dimensions of array or collections stored in the TTree if any.

    // Declaration of leaf types
    Int_t fNumOfBoards;
    // TTimeStamp      *eventTime;
    Int_t fEventTimeInSec;
    Int_t fEventTimeInNanoSec;
    Int_t fTriggerCell[2];          //[fNumOfBoards]
    Double_t fWaveform[2][4][1024]; //[fNumOfBoards]
    Double_t fTime[2][4][1024];     //[fNumOfBoards]
    Double_t fAdcSum[2][4];         //[fNumOfBoards]
    Int_t fDiscriCell[2][4];
    Int_t fSerialNumber[2];
    Double_t fAdcSum_crystals[2][4];

    // List of branches
    TBranch *b_numOfBoards;        //!
    TBranch *b_eventTimeInSec;     //!
    TBranch *b_eventTimeInNanoSec; //!
    TBranch *b_triggerCell;        //!
    TBranch *b_scaler;             //!
    TBranch *b_waveform;           //!
    TBranch *b_time;               //!
    TBranch *b_adcSum;             //!

    DRS4Ana();
    virtual ~DRS4Ana();
    virtual Int_t Cut(Long64_t entry);
    virtual Int_t GetEntry(Long64_t entry);
    virtual Long64_t LoadTree(Long64_t entry);
    virtual void Init(TChain *tree_Event);
    virtual Int_t Init_BoardInfo(TChain *tree_Info);
    virtual Bool_t Notify();
    virtual void Show(Long64_t entry = -1);
    virtual void PlotADCSum(Int_t iBoard = 0, Int_t iCh = 0);
    virtual void PlotWave(Int_t iBoard = 0, Int_t iCh = 0, Int_t EventID = 1);
    virtual void PlotWaves(Int_t iBoard = 0, Int_t iCh = 0, Int_t EventID = 1, Int_t nEvent = 10);

    virtual void SetSignalPolarity(Int_t polarity) { fSignalPolarity = polarity; };
    virtual Int_t GetSignalPolarity() { return fSignalPolarity; };

    virtual void SetWaveRangeX(Double_t min, Double_t max);
    virtual void SetWaveRangeY(Double_t min, Double_t max);

    virtual void SetPedestalTimeRange(Double_t min, Double_t max);
    virtual Double_t GetPedestalTimeMin() { return fPedestalTmin; };
    virtual Double_t GetPedestalTimeMax() { return fPedestalTmax; };
    virtual void SetChargeIntegralTimeRange(Double_t min, Double_t max);
    virtual Double_t GetChargeIntegralTimeMin() { return fChargeIntegralTmin; };
    virtual Double_t GetChargeIntegralTimeMax() { return fChargeIntegralTmax; };

    virtual Double_t GetMinVoltage(Int_t iBoard = 0, Int_t iCh = 0);
    virtual Double_t GetMaxVoltage(Int_t iBoard = 0, Int_t iCh = 0);
    virtual Double_t GetPedestal(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0);
    //virtual Double_t GetPedestalMean(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0);
    virtual Double_t PlotPedestalMean(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0);
    virtual Double_t GetChargeIntegral(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0, Double_t TcutMin = 0, Double_t TcutMax = 1000);
    virtual Double_t PlotChargeIntegral(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0, Double_t xmin = 0.0, Double_t xmax = 5000.0);
    virtual Double_t PlotMaxVoltage(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0, Double_t xmin = 0.0, Double_t xmax = 5000.0);
    virtual Double_t GetAbsMaxVoltage(Int_t iBoard = 0, Int_t iCh = 0);
    virtual Double_t Output_chargeintegral(Int_t iCh = 0, Double_t Vcut = 20, Double_t xmin = 0.0, Double_t xmax = 50.0);
    virtual Double_t automated_peaksearch(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 20, Double_t xmin = 0.0, Double_t xmax = 50.0, Int_t numPeaks = 10, Double_t fitRange = 2.0);
    virtual void Output_EventTime(Int_t iCh = 0);
    virtual Double_t PlotTriggerRate(Int_t iCh = 0);
    virtual Double_t Overlay_PlotWaves(Int_t iBoard = 0, Int_t iCh = 0);
    virtual void DEBUG_timebin(Int_t iBoard = 0, Int_t iCh = 0);

    virtual void Plot_wave_two_boards(Int_t iCh_master = 0, Int_t iCh_slave = 0, Int_t EventID = 0, Int_t canvas_index = 0);
    virtual void Plot_waves_two_boards(Int_t event_num_initial = 0, Int_t iCh_master = 0, Int_t iCh_slave = 0);
    
    virtual Double_t Overlay_PlotWaves_discri(Int_t iBoard = 0, Int_t iCh = 0, Double_t threshold = 0.10);
    virtual Double_t GetTriggerTiming(Int_t iBoard = 0, Int_t iCh = 0, Double_t threshold = 0.10, Double_t trigger_voltage = -0.025);
    virtual Double_t Output_MaxVoltage(Int_t how_many_boards = 1, Int_t iCh = 0);
    virtual Double_t Plot_2Dhist_energy_btwn_PMTs(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1);
    virtual Double_t PlotEnergy(TString key, TString key_Crystal, Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax);
    virtual Double_t PlotSumEnergy(TString key = "0120", TString key_Crystal1 = "NaI", Int_t iBoard1 = 0, Int_t iCh1 = 0, TString key_Crystal2 = "NaI", Int_t iBoard2 = 0, Int_t iCh2 = 2, Double_t Vcut = 20, Double_t xmin = 0, Double_t xmax = 650);
    virtual Double_t PlotEnergy2(TString key, Int_t iBoard, Int_t iCh, Double_t xmin, Double_t xmax);
    virtual Double_t PlotWavesWithThreshold(Int_t iBoard, Int_t iCh);
    virtual Double_t automated_peaksearch_SCA_mode(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 20, Double_t xmin = 0.0, Double_t xmax = 50.0, Int_t numPeaks = 10, Double_t fitRange = 2.0);
    virtual Double_t GSO_peaksearch(Int_t iBoard = 0, Int_t iCh = 0, Double_t adcMin = 0, Double_t adcMax = 250.0, Int_t numPeaks = 10, Double_t fitRange = 3.0, Double_t spec_sigma = 5.0);
    virtual Double_t time_divided_spectrum(Int_t divOfTime = 10);
    virtual Double_t time_divided_adcSum(Int_t divOfTime = 10);
    virtual TString Makedir_Date();
    virtual Int_t IfFile_duplication(TString folderPath, TString &fileName);
    virtual Double_t Print_discriCell(Int_t iBoard = 0, Int_t iCh = 0);
    virtual Double_t NaI_peaksearch(Int_t iBoard = 0, Int_t iCh = 0, Double_t adcMin = 0, Double_t adcMax = 250.0, Int_t numPeaks = 10, Double_t fitRange = 3.0, Double_t spec_sigma = 5.0);
    virtual Double_t peak_divided(Int_t iBoard = 0, Int_t iCh = 0, Double_t adcMin = 0, Double_t adcMax = 150.0, Double_t fitXmin = 0.0, Double_t fitXmax = 0.0, Double_t adcTimeRange = 180.0);
    virtual void Load_EnergycalbData(TString key, Double_t p0[2][4], Double_t p0e[2][4], Double_t p1[2][4], Double_t p1e[2][4], Double_t p0_res[2][4], Double_t p0e_res[2][4]);
    virtual Double_t semi_automated_spectrum_fitting(TString key_crystal = "NaI", Int_t iBoard = 0, Int_t iCh = 0, Double_t adcMin = 0, Double_t adcMax = 100);
    virtual Double_t Plot_waveform_8ch();
    virtual Double_t Plot_TriggerTimeDist_8ch();
    virtual Double_t Plot_TriggerTimeDist_8ch_sato_cut();
    virtual Double_t Plot_TriggerTimeDist_8ch_difference();
    virtual Double_t PlotSumEnergy_with_cutting(TString key_energy_calib = "0120", Int_t cutting_option = 0, Int_t iBoard1 = 0, Int_t iCh1 = 0, Int_t iBoard2 = 1, Int_t iCh2 = 0, Double_t xmax = 1000);
    virtual Double_t Plot_2Dhist_energy_with_cut(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1);
   
    virtual Double_t EventSelection(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut =true, Double_t sigma = 3);
    virtual void NaI_waveform_ukai(Int_t x_iBoard = 0,Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1);
    virtual void osci_ukai(Int_t x_iBoard = 0,Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1);
    

    virtual Double_t Plot_2Dhist_energy_with_cut1(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1);
    virtual Double_t Plot_2Dhist_energy_with_cut2(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_sato = 4, Int_t nSigma_GSO = 4);
    virtual Double_t Plot_2Dhist_energy_with_cut2_1(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_GSO = 4);
    virtual Double_t Plot_2Dhist_energy_with_cut3(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2);
    virtual Double_t Plot_2Dhist_energy_with_cut3_t(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_NaI = 2, Int_t nSigma_y_GSO = 2);
    virtual Double_t Plot_2Dhist_energy_with_cut4(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2);
    virtual Double_t Plot_2Dhist_energy_with_cut5(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2);
    virtual Double_t Plot_2Dhist_energy_with_cut5_t(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2);
    virtual void PlotTrigger();
    virtual void PlotDiscriADC(Int_t iBoard, Int_t iCh);
    virtual Double_t DiscriAna(Int_t Scell = 100 , Int_t Fcell = 120);
    virtual void DiscriAna2(Int_t Scell = 100 , Int_t Fcell = 120);
    virtual void Discricut();
    virtual void waveform(Int_t nentry);
    virtual Double_t Plot_discriCell_each_chain(Int_t nentries = 10000);
    virtual Double_t Plot_2Dhist_energy_with_cut6(TString key = "0120", TString key_Crystal_x = "NaI", TString key_Crystal_y = "NaI", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Int_t nSigma_x_S2 = 1, Int_t nSigma_y_A2 = 2, Int_t nSigma_S1 = 2, Int_t nSigma_A1 = 2);
    virtual void Discricut2();
    virtual void Energy_fit(Int_t iBoard , Int_t ich , Int_t xMin=0, Int_t xMax=650, Int_t fitRangeMin=450 , Int_t fitRangeMax=580);
    virtual void PlotTrigger2();
    virtual void PlotdiscriTime_difference(Int_t iBoard1 = 0, Int_t iCh1 = 0, Int_t iBoard2 = 0, Int_t iCh2 = 2, Int_t fit_flag = 1, Double_t fit_min = -5.0, Double_t fit_max = 25.0, Double_t sigma = 2.0);
    virtual Double_t PlotDiscriCell_difference_NaIs(Int_t iBoard1 = 0, Int_t iCh1 = 0, Int_t iBoard2 = 0, Int_t iCh2 = 2, Int_t entry_flag = 0, Int_t figure_option = 0, Int_t xmin = -1050, Int_t xmax = 1050, Int_t ped_nega_lower = -130, Int_t ped_nega_upper = -30, Int_t ped_posi_lower = 30, Int_t ped_posi_upper = 130);
    virtual Double_t PlotdiscriCell_difference_S2A2(Int_t iBoard1 = 0, Int_t iCh1 = 0, Int_t iBoard2 = 0, Int_t iCh2 = 2, Int_t entry_flag = 0, Int_t figure_option = 0, Int_t xmin = -1050, Int_t xmax = 1050, Int_t ped_nega_lower = -130, Int_t ped_nega_upper = -30, Int_t ped_posi_lower = 30, Int_t ped_posi_upper = 130);

    virtual Double_t EventSelection2(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Double_t sigma4_S1 = 1.0, Double_t sigma5_S2 = 1.0, Double_t sigma6_GSO_lower = 1.0, Double_t sigma6_GSO_upper = 1.0, Double_t sigma7_sato_lower = 1.0, Double_t sigma7_sato_upper = 1.0, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut =true);
    virtual Double_t EventSelection2_eff(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Double_t sigma4_S1 = 1.0, Double_t sigma5_S2 = 1.0, Double_t sigma6_GSO_lower = 1.0, Double_t sigma6_GSO_upper = 1.0, Double_t sigma7_sato_lower = 1.0, Double_t sigma7_sato_upper = 1.0, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut =true);
    virtual Double_t EventSelection2_S1A1(TString key = "0204", Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1, Double_t sigma4_S1 = 1.0, Double_t sigma5_S2 = 1.0, Double_t sigma6_GSO_lower = 1.0, Double_t sigma6_GSO_upper = 1.0, Double_t sigma7_sato_lower = 1.0, Double_t sigma7_sato_upper = 1.0, bool applyTimeCut = true, bool applyScatterCut = true, bool applyEnergyCut =true);
    virtual Double_t PlotRunPeriods();


    TH2F *fH2Waveform = NULL;
    TH1F *fH1AdcSum = NULL;
    TH1F *fH1AdcPeak = NULL;
    TH1F *fH1Pedestal = NULL;
    TH1F *fH1ChargeIntegral = NULL;
    TH1F *fH1MaxVoltage = NULL;
    TH1F *fH1TriggerRate = NULL;
    TH2F *fH2Overlay_Waves = NULL;
    TH2F *fH2Filtered_Overlay_Waves = NULL;
    TH2F *fH2Waveform0 = NULL;
    TH2F *fH2Waveform1 = NULL;
    TH2F *fH2Energy_PMTs = NULL;
    TH2F *fH2Energy_PMTs2 = NULL;
    TH1F *fH1SumChargeIntegral = NULL;
    TH1F *fH1TriggerTime = NULL;
    TH1F *fH1Energy_PMTs = NULL;
    TH1F *fH1TriggerTimeDifference = NULL;
    TH1F *fH1TriggerCellDifference = NULL;

    

private:
    Double_t fTimeBinWidthInNanoSec;
    Int_t fSignalPolarity; // 1:Positive signal, -1:Negative signal

    Double_t fPedestalTmin;       // for the time range to caclurate pedestal (ns)
    Double_t fPedestalTmax;       // for the time range to caclurate pedestal (ns)
    Double_t fChargeIntegralTmin; // for the time range to charge integration (ns)
    Double_t fChargeIntegralTmax; // for the time range to charge integration (ns)

    // For axis range of histgrams
    Double_t fADCsumXmin;
    Double_t fADCsumXmax;
    Double_t fWaveformXmin;
    Double_t fWaveformXmax;
    Double_t fWaveformYmin;
    Double_t fWaveformYmax;

};

#endif

#ifdef DRS4Ana_cxx
DRS4Ana::DRS4Ana() : fChain(globalChain_Event)
{
    fRootFile += "_FILENAME_";
    // TFile *f = globalChain_Event->GetFile();
    // if (!f || !f->IsOpen())
    // {
    //     f = new TFile(fRootFile);
    // }

    //ボード情報の初期化
    fNumOfBoards=Init_BoardInfo(globalChain_Info);
    printf("\tconstructor || fNumOfBoards %d\n", fNumOfBoards);

    //イベント情報の初期化
    Init(globalChain_Event);
    
}

DRS4Ana::~DRS4Ana()
{
    if (!fChain)
        return;
    delete fChain->GetCurrentFile();
}

Int_t DRS4Ana::GetEntry(Long64_t entry)
{
    // Read contents of entry.
    if (!fChain)
        return 0;
    return fChain->GetEntry(entry);
}
Long64_t DRS4Ana::LoadTree(Long64_t entry)
{
    // Set the environment to read one entry
    if (!fChain)
        return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0)
        return centry;
    if (fChain->GetTreeNumber() != fCurrent)
    {
        fCurrent = fChain->GetTreeNumber();
        Notify();
    }
    return centry;
}

Int_t DRS4Ana::Init_BoardInfo(TChain *tree_Info){
    if(!tree_Info){
        return -1;
    }
    
    tree_Info->SetBranchAddress("numOfBoards", &fNumOfBoards);
    tree_Info->SetBranchAddress("serialNumber", &fSerialNumber);
    tree_Info->GetEntry(0);

    printf("\n\tDRS4Ana.h->Init_BoardInfo->fNumOfBoards = %d\n", fNumOfBoards);
    if(fNumOfBoards == 1){
        printf("\t\tiBoard 0 | searialNumber %d\n", fSerialNumber[0]);
    }
    else if(fNumOfBoards == 2){
        printf("\t\tiBoard 0 | searialNumber %d\n\t\tiBoard 1 | serialNumber %d\n", fSerialNumber[0], fSerialNumber[1]);
    }
    return(fNumOfBoards);
}
void DRS4Ana::Init(TChain *tree_Event)
{
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the branch addresses and branch
    // pointers of the tree will be set.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).

    // Set branch addresses and branch pointers
    printf("\n\tInit start\n");
    printf("\tentries : %lld", fChain->GetEntries());
    if (!tree_Event)
        return;
    fChain = tree_Event;
    printf("\tentries : %lld", fChain->GetEntries());
    fCurrent = -1;
    fChain->SetMakeClass(1);

    fChain->SetBranchAddress("fSec", &fEventTimeInSec, &b_eventTimeInSec);
    fChain->SetBranchAddress("fNanoSec", &fEventTimeInNanoSec, &b_eventTimeInNanoSec);
    fChain->SetBranchAddress("triggerCell", fTriggerCell, &b_triggerCell);
    fChain->SetBranchAddress("waveform", fWaveform, &b_waveform);
    fChain->SetBranchAddress("time", fTime, &b_time);
    fChain->SetBranchAddress("adcSum", fAdcSum, &b_adcSum);
    fChain->SetBranchAddress("discriCell", fDiscriCell);
    fChain->SetBranchAddress("adcSum_crystals", fAdcSum_crystals);
    printf("\tbranch address set\n");

    Notify();
    fChain->GetEntry(0);
    printf("\tGetEntry(0)\n");
    
    fTimeBinWidthInNanoSec = fTime[0][0][1023]/1024.0; //original fTime[0][0][1]
    fWaveformXmin = fTime[0][0][0];
    fWaveformXmax = fTime[0][0][1023];
    fWaveformYmin = -0.5;
    fWaveformYmax = 0.5;

    fADCsumXmin = 0.0;
    fADCsumXmax = 200.0;

    fPedestalTmin = fTime[0][0][0];
    fPedestalTmax = fTime[0][0][1023] / 40.0;
    fChargeIntegralTmin = fTime[0][0][0];
    fChargeIntegralTmax = fTime[0][0][1023];

    printf("\tparameters sets\n");

    // fSignalPolarity = 1; // positive signal
    fSignalPolarity = -1; // negative signal
}


Bool_t DRS4Ana::Notify()
{
    // The Notify() function is called when a new file is opened. This
    // can be either for a new TTree in a TChain or when when a new TTree
    // is started when using PROOF. It is normally not necessary to make changes
    // to the generated code, but the routine can be extended by the
    // user if needed. The return value is currently not used.

    return kTRUE;
}

void DRS4Ana::Show(Long64_t entry)
{
    // Print contents of entry.
    // If entry is not specified, print current entry
    if (!fChain)
        return;
    fChain->Show(entry);
}
Int_t DRS4Ana::Cut(Long64_t entry)
{
    // This function may be called from Loop.
    // returns  1 if entry is accepted.
    // returns -1 otherwise.

    return 1;
}
#endif // #ifdef DRS4Ana_cxx
