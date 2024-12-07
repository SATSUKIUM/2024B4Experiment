#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TString.h>
#include <iostream>
#include <TCanvas.h>
#include <TF1.h>
#include <TH2.h>
#include <TString.h>
#include <TStyle.h>

void overview_waveform(const char* filename){
    TFile *rootFile = TFile::Open(filename);

    TTree *tree = (TTree*)rootFile->Get("treeDRS4BoardEvent");
    if(!tree){
        std::cerr << "Error in obtain TTree : treeDRSBoardEvent not found" << std::endl;
        return;
    }
    // std::cout << "DEBUG || DEBUG || DEBUG" << std::endl;

    // Double_t waveform[2][4][1024];
    // tree->SetBranchAddress("waveform", &waveform);

    TCanvas *c1 = new TCanvas("canvas", "waveforms of each event", 800, 600);
    c1->Divide(4,2);

    TH2F* H2Waveform[2][4] = {nullptr};

    Int_t nentries = tree->GetEntriesFast();
    
    // std::cout << "DEBUG || DEBUG || DEBUG" << std::endl;
    std::cout << "========================================" << std::endl <<"If you want to quit, press Ctrl+C and then type .q and then enter" << std::endl << "========================================" << std::endl;

    for(Int_t eventID=0; eventID<nentries; eventID++){
        tree->GetEntry(eventID);
        // std::cout << "DEBUG || DEBUG || DEBUG" << std::endl;

        

        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                // std::cout << "DEBUG || DEBUG || DEBUG" << std::endl;
                c1->cd(iBoard*4+iCh + 1);

                if(H2Waveform[iBoard][iCh] != NULL){
                    delete H2Waveform[iBoard][iCh];
                    H2Waveform[iBoard][iCh] = nullptr;
                }
                H2Waveform[iBoard][iCh] = new TH2F(Form("hist || iBoard: %d || iCh %d", iBoard, iCh), Form("EventID : %d || iBoard: %d || iCh %d", eventID, iBoard, iCh), 512, 0, 1023, 512, -0.55, 0.05);

                H2Waveform[iBoard][iCh]->SetXTitle("Iteration");
                H2Waveform[iBoard][iCh]->SetYTitle("Waveform");
                H2Waveform[iBoard][iCh]->Draw();
                gStyle->SetOptStat(0);

                tree->Draw(Form("waveform[%d][%d]:Iteration$", iBoard, iCh), "", "lsame", 1 , eventID);
            }
        }
        c1->Update();

        c1->WaitPrimitive();
    }
}