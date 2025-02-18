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
#include <TFile.h>
void DAQrate_transition(TString filepath){
    std::ifstream ifs(filepath);
    TGraph *graph = new TGraph();
    Double_t period,rate,dummy;
    Int_t count = 0;
    Double_t spent_time = 0;
    while(ifs >> dummy >> period >> rate){
        printf("\t%f %f\n", period, rate);
        graph->SetPoint(count, spent_time + (period/60.0/60.0/24.0)/2.0, rate);
        spent_time += period/60.0/60.0/24.0;
        count++;

    }
    graph->SetMarkerSize(2);
    graph->Draw("apl");
}