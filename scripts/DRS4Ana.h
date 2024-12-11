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

class DRS4Ana
{
public:
    TTree *fChain;  //! pointer to the analyzed TTree or TChain
    Int_t fCurrent; //! current Tree number in a TChain

    // Fixed size dimensions of array or collections stored in the TTree if any.

    // Declaration of leaf types
    Int_t fNumOfBoards;
    // TTimeStamp      *eventTime;
    Int_t fEventTimeInSec;
    Int_t fEventTimeInNanoSec;
    Int_t fTriggerCell[NUM_OF_BOARDS];          //[fNumOfBoards]
    UInt_t fScaler[NUM_OF_BOARDS][4];           //[fNumOfBoards]
    Double_t fWaveform[NUM_OF_BOARDS][4][1024]; //[fNumOfBoards]
    Double_t fTime[NUM_OF_BOARDS][4][1024];     //[fNumOfBoards]
    Double_t fAdcSum[NUM_OF_BOARDS][4];         //[fNumOfBoards]

    // List of branches
    TBranch *b_numOfBoards;        //!
    TBranch *b_eventTimeInSec;     //!
    TBranch *b_eventTimeInNanoSec; //!
    TBranch *b_triggerCell;        //!
    TBranch *b_scaler;             //!
    TBranch *b_waveform;           //!
    TBranch *b_time;               //!
    TBranch *b_adcSum;             //!

    DRS4Ana(const Char_t *fRootFile_par = "../data/run00010_22Na_1.65kV_MS.dat.root");
    virtual ~DRS4Ana();
    virtual Int_t Cut(Long64_t entry);
    virtual Int_t GetEntry(Long64_t entry);
    virtual Long64_t LoadTree(Long64_t entry);
    virtual void Init(TTree *tree);
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
    virtual Double_t GetPedestalMean(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0);
    virtual Double_t PlotPedestalMean(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0);
    virtual Double_t GetChargeIntegral(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0, Double_t TcutMin = 0, Double_t TcutMax = 1000);
    virtual Double_t PlotChargeIntegral(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0, Double_t xmin = 0.0, Double_t xmax = 5000.0);
    virtual Double_t PlotMaxVoltage(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 0.0, Double_t xmin = 0.0, Double_t xmax = 5000.0);
    virtual Double_t GetAbsMaxVoltage(Int_t iBoard = 0, Int_t iCh = 0);
    virtual Double_t Output_chargeintegral(Int_t iCh = 0, Double_t Vcut = 20, Double_t xmin = 0.0, Double_t xmax = 50.0);
    virtual Double_t automated_peaksearch(Int_t iBoard = 0, Int_t iCh = 0, Double_t Vcut = 20, Double_t xmin = 0.0, Double_t xmax = 50.0, Int_t numPeaks = 10);
    virtual void Output_EventTime(Int_t iCh = 0);
    virtual Double_t PlotTriggerRate(Int_t iCh = 0);
    virtual Double_t Overlay_PlotWaves(Int_t iCh = 0);
    virtual void DEBUG_timebin(Int_t iBoard = 0, Int_t iCh = 0);

    virtual void Plot_wave_two_boards(Int_t iCh_master = 0, Int_t iCh_slave = 0, Int_t EventID = 0, Int_t canvas_index = 0);
    virtual void Plot_waves_two_boards(Int_t event_num_initial = 0, Int_t iCh_master = 0, Int_t iCh_slave = 0);
    
    virtual Double_t Overlay_PlotWaves_discri(Int_t iCh = 0, Double_t threshold = 0.10);
    virtual Double_t GetTriggerTiming(Int_t iBoard = 0, Int_t iCh = 0, Double_t threshold = 0.10, Double_t trigger_voltage = -0.025);
    virtual Double_t Output_MaxVoltage(Int_t how_many_boards = 1, Int_t iCh = 0);
    virtual Double_t Plot_scatter_energy_btwn_PMTs(Int_t x_iBoard = 0, Int_t x_iCh = 0, Int_t y_iBoard = 0, Int_t y_iCh = 1);

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

    TString fRootFile;
};

#endif

#ifdef DRS4Ana_cxx
DRS4Ana::DRS4Ana(const Char_t *fRootFile_par) : fChain(0)
{

    fRootFile.Form("%s", fRootFile_par);
    TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject(fRootFile);
    if (!f || !f->IsOpen())
    {
        f = new TFile(fRootFile);
    }
    TTree *tree;
    f->GetObject("treeDRS4BoardEvent", tree);
    Init(tree);
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

void DRS4Ana::Init(TTree *tree)
{
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the branch addresses and branch
    // pointers of the tree will be set.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).

    // Set branch addresses and branch pointers
    if (!tree)
        return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);

    // fChain->SetBranchAddress("numOfBoards", &fNumOfBoards, &b_numOfBoards);
    fChain->SetBranchAddress("fSec", &fEventTimeInSec, &b_eventTimeInSec);
    fChain->SetBranchAddress("fNanoSec", &fEventTimeInNanoSec, &b_eventTimeInNanoSec);
    fChain->SetBranchAddress("triggerCell", fTriggerCell, &b_triggerCell);
    // fChain->SetBranchAddress("scaler", fScaler, &b_scaler);
    fChain->SetBranchAddress("waveform", fWaveform, &b_waveform);
    fChain->SetBranchAddress("time", fTime, &b_time);
    fChain->SetBranchAddress("adcSum", fAdcSum, &b_adcSum);

    Notify();

    fChain->GetEntry(1);
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
