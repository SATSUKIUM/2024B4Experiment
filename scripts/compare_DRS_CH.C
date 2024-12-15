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
#include <iostream>
#include <TSystem.h>

#include <TSystemDirectory.h>
#include <TFile.h>

Double_t extract_Vpp(const TString filePath){
    TString fileName = filePath(filePath.Last('/')+1, filePath.Length()-filePath.Last('/'));

    Int_t firstUnderscorePos = fileName.Index('_');
    Int_t seconUnderscorePos = fileName.Index('_', firstUnderscorePos+1);
    Int_t thirdUnderscorePos = fileName.Index('_', seconUnderscorePos+1);
    Int_t fourthUnderscorePos = fileName.Index('_', thirdUnderscorePos+1);

    TString VppStr = fileName(thirdUnderscorePos+1, fourthUnderscorePos);
    Double_t VppNum = VppStr.Atoi();

    return (VppNum);

}

Double_t compare_DRS_CH(TString DIR_PATH = "../data/20241215/master/CH1_2/", Int_t iCh_1 = 0, Int_t iCh_2 = 1){
    TCanvas *c1 = new TCanvas("c1", "compare measured voltage between CHs of DRS4 Board", 800, 600);
    c1->SetGrid();
    gStyle->SetOptStat(0);
    TGraph *graph = new TGraph;
    graph->SetMarkerStyle(20);
    graph->SetTitle(";Function generator Vpp [mV];measured Vmax [mV]");
    graph->Draw("AP");

    //.rootファイルを探す
    TSystemDirectory Dir("dir", DIR_PATH);
    TList* Filelist = Dir.GetListOfFiles();

    std::vector<TString> fileNames;
    TIter next(Filelist);
    TObject *fileObj;

    Int_t graph_index = 0;

    while((fileObj = next())){
        TSystemFile* file = (TSystemFile*)fileObj;
        TString fileName = file->GetName();
        if(fileName.EndsWith(".root")){
            fileNames.push_back(fileName);

            std::cout << fileName << std::endl;
        }
        else{
            continue;
        }

        Double_t VppSet = extract_Vpp(fileName);
        TString filePath = DIR_PATH + "/" + fileName;
        TFile *f = TFile::Open(filePath);
        if(f && !f->IsZombie()){
            TTree *tree = (TTree*)f->Get("treeDRS4BoardEvent");
            TTree *tree_info = (TTree*)f->Get("treeDRS4BoardInfo");

            Int_t numOfBoards;
            tree_info->SetBranchAddress("numOfBoards", &numOfBoards);
            tree_info->GetEntry(0);

            Double_t waveform[numOfBoards][4][1024];
            tree->SetBranchAddress("waveform", waveform);

            Int_t nentries = tree->GetEntriesFast();
            Double_t avgMaxVoltage = 0;
            Double_t avgMinVoltage = 0;
            for(Int_t eventID=0; eventID<nentries; eventID++){
                tree->GetEntry(eventID);
                Double_t maxVoltage = -0.5;
                Double_t minVoltage = 0.5;
                Double_t voltage_Buf;
                for(Int_t iCell=0; iCell<1024; iCell++){
                    voltage_Buf = waveform[0][iCh_1][iCell];
                    if(voltage_Buf > maxVoltage){
                        maxVoltage = voltage_Buf;
                    }
                    if(voltage_Buf < minVoltage){
                        minVoltage = voltage_Buf;
                    }
                }
                avgMaxVoltage += maxVoltage;
                avgMinVoltage += minVoltage;
            }
            avgMaxVoltage = avgMaxVoltage/nentries;
            avgMinVoltage = avgMinVoltage/nentries;
            
            graph->SetPoint(graph_index, VppSet, (avgMaxVoltage-avgMinVoltage)*1000/2);
            graph_index++;

            for(Int_t eventID=0; eventID<nentries; eventID++){
                tree->GetEntry(eventID);
                Double_t maxVoltage = -0.5;
                Double_t minVoltage = 0.5;
                Double_t voltage_Buf;
                for(Int_t iCell=0; iCell<1024; iCell++){
                    voltage_Buf = waveform[0][iCh_2][iCell];
                    if(voltage_Buf > maxVoltage){
                        maxVoltage = voltage_Buf;
                    }
                    if(voltage_Buf < minVoltage){
                        minVoltage = voltage_Buf;
                    }
                }
                avgMaxVoltage += maxVoltage;
                avgMinVoltage += minVoltage;
            }
            avgMaxVoltage = avgMaxVoltage/nentries;
            avgMinVoltage = avgMinVoltage/nentries;
            
            graph->SetPoint(graph_index, VppSet, (avgMaxVoltage-avgMinVoltage)*1000/2);
            graph_index++;
        }
    }
    graph->Draw("AP");
    return graph_index;
}