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
#include <TFile.h>

#include <TLegend.h>

#include <fstream>

#include <TApplication.h>
#include <TChainElement.h>
#include <TObjArray.h>

#define DEBUG 0

void adcSum_crystals(TString filepath = "../data/PhysicsRun/ROOT_FILES2/Run_004.dat"){
    // 入力ファイルを開く
    TFile *file = TFile::Open(filepath, "UPDATE");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    // TTree を取得
    TTree *tree = (TTree*)file->Get("treeDRS4BoardEvent");
    if (!tree) {
        std::cerr << "Error: TTree not found!" << std::endl;
        file->Close();
        return;
    }

    // 既存のブランチを取得
    Double_t fWaveform[2][4][1024], fTime[2][4][1024];
    Int_t fDiscriCell[2][4];
    tree->SetBranchAddress("waveform", fWaveform);
    tree->SetBranchAddress("time", fTime);
    tree->SetBranchAddress("discriCell", fDiscriCell);

    // 新しいブランチの変数を定義
    Double_t adcSum_crystals[2][4];
    TBranch *newBranch = tree->Branch("adcSum_crystals", adcSum_crystals, "adcSum_crystals[2][4]/D");


    Double_t fPedestalTmin, fPedestalTmax;
    fPedestalTmin = fTime[0][0][0];
    fPedestalTmax = fTime[0][0][1023] / 40.0;

    Double_t pedeslta_sum;
    // イベントループ
    Long64_t nentries = tree->GetEntries();
    Int_t counter;
    Double_t discriTime;
    for (Long64_t i = 0; i < nentries; i++) {
        tree->GetEntry(i);

        for(Int_t iBoard=0; iBoard<2; iBoard++){
            for(Int_t iCh=0; iCh<4; iCh++){
                pedeslta_sum = 0;
                counter = 0;
                adcSum_crystals[iBoard][iCh] = 0.0;
                for(Int_t iCell=0; iCell<1024; iCell++){
                    if(fTime[0][0][iCell] > fPedestalTmax){
                    break;
                    }
                    pedeslta_sum += fWaveform[iBoard][iCh][iCell];
                    counter++;
                }
                pedeslta_sum = pedeslta_sum/counter;
                if(iBoard == 0){
                    if(iCh == 1){
                        discriTime = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
                        for(Int_t iCell = fDiscriCell[iBoard][iCh];;iCell++){
                            if(fTime[iBoard][iCh][iCell] > discriTime - 50.0 && fTime[iBoard][iCh][iCell] < discriTime + 180.0){
                                adcSum_crystals[iBoard][iCh] += fWaveform[iBoard][iCh][iCell] - pedeslta_sum;
                            }
                            else if(fTime[iBoard][iCh][iCell] > discriTime + 180.0){
                                break;
                            }
                        }
                        if(DEBUG){
                                    std::cout << Form("adcSum_crystals[%d][%d] : %f", iBoard, iCh, adcSum_crystals[iBoard][iCh]) << std::endl;
                        }
                    }
                    else{
                        discriTime = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
                        for(Int_t iCell = fDiscriCell[iBoard][iCh];;iCell++){
                            if(fTime[iBoard][iCh][iCell] > discriTime - 50.0 && fTime[iBoard][iCh][iCell] < discriTime + 600.0){
                                adcSum_crystals[iBoard][iCh] += fWaveform[iBoard][iCh][iCell] - pedeslta_sum;
                            }
                            else if(fTime[iBoard][iCh][iCell] > discriTime + 180.0){
                                break;
                            }
                        }
                        if(DEBUG){
                                    std::cout << Form("adcSum_crystals[%d][%d] : %f", iBoard, iCh, adcSum_crystals[iBoard][iCh]) << std::endl;
                        }
                    }
                }
                else{
                    discriTime = fTime[iBoard][iCh][fDiscriCell[iBoard][iCh]];
                    for(Int_t iCell = fDiscriCell[iBoard][iCh];;iCell++){
                        if(fTime[iBoard][iCh][iCell] > discriTime - 50.0 && fTime[iBoard][iCh][iCell] < discriTime + 180.0){
                            adcSum_crystals[iBoard][iCh] += fWaveform[iBoard][iCh][iCell] - pedeslta_sum;
                        }
                        else if(fTime[iBoard][iCh][iCell] > discriTime + 180.0){
                            break;
                        }
                    }
                }
            }
        }
        newBranch->Fill();
        if(i % 10000 == 0){
            printf("\tevent processed... %lld\n",i);
            for(Int_t iBoard=0; iBoard<2; iBoard++){
                for(Int_t iCh=0; iCh<4; iCh++){
                    if(1){
                    std::cout << Form("adcSum_crystals[%d][%d] : %f", iBoard, iCh, adcSum_crystals[iBoard][iCh]) << std::endl;
                    }
                }
            }
        }
    }

    // 既存のツリーを削除して新しいツリーを保存
    // ツリーをファイルに保存
    tree->Fill();
    tree->Print();
    tree->Write("", TObject::kOverwrite);

    file->Close();
}