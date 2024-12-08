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

void triggered_waveform(const char* filename, Int_t initial_event = 0){
    TFile *rootFile = TFile::Open(filename);

    TTree *tree = (TTree*)rootFile->Get("treeDRS4BoardEvent");
    if(!tree){
        std::cerr << "Error in obtain TTree : treeDRSBoardEvent not found" << std::endl;
        return;
    }
    // std::cout << "DEBUG || DEBUG || DEBUG" << std::endl;

    Double_t waveform[2][4][1024];
    tree->SetBranchAddress("waveform", &waveform);

    TCanvas *c1 = new TCanvas("canvas", "waveforms of each event", 800, 600);
    c1->Divide(4,2);

    TH2F* H2Waveform[2][4] = {nullptr};

    Int_t nentries = tree->GetEntriesFast();
    
    // std::cout << "DEBUG || DEBUG || DEBUG" << std::endl;
    std::cout << "========================================" << std::endl <<"If you want to quit, press Ctrl+C and then type .q and then enter" << std::endl << "========================================" << std::endl;

    for(Int_t eventID=initial_event; eventID<nentries; eventID++){
        tree->GetEntry(eventID);
        // std::cout << "DEBUG || DEBUG || DEBUG" << std::endl;

        Int_t trigger_flag = 0;
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
                gPad->SetGrid();


                tree->Draw(Form("waveform[%d][%d]:Iteration$", iBoard, iCh), "", "lsame", 1 , eventID);
                if(iBoard*4+iCh != 0 && iBoard*4+iCh != 1 && iBoard*4+iCh != 2){
                    tree->GetEntry(eventID);
                    // std::cout << iBoard*4+iCh <<  " || " << tree->GetMinimum(Form("waveform[%d][%d]", iBoard, iCh)) << std::endl;
                    Double_t waveform_min = 1;
                    for(Int_t iCell=0; iCell<1024; iCell++){
                        if(waveform[iBoard][iCh][iCell] < waveform_min){
                            waveform_min = waveform[iBoard][iCh][iCell];
                        }
                    }
                    if(waveform_min < -0.050){
                        trigger_flag++;
                    }
                }
            }
        }
        if(trigger_flag == 1){
            c1->Update();

            c1->WaitPrimitive();
        }
        std::cout << eventID << std::endl;
        
    }
}