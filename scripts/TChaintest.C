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
#include <TChain.h>

void TChaintest(const Char_t *f1 = "../data/20250124/huruno2/22Na.dat.root", const Char_t *f2 = "../data/20250124/huruno2/152Eu.dat.root"){
    TChain *fChain = new TChain("treeDRS4BoardEvent");
    // fChain->Add("../data/20250124/huruno2/22Na.dat.root/treeDRS4BoardEvent;23");
    printf("\t%s\n", f1);
    printf("\t%s\n", f2);
    fChain->Add(f1);
    fChain->Add(f2);
    printf("%d\n",fChain->GetNtrees());
    printf("%lld\n",fChain->GetEntries());
    fChain->GetEntry(0);
    printf("%lld\n",fChain->GetEntriesFast());
    Int_t triggerCell[1][4];
    fChain->SetBranchAddress("triggerCell",&triggerCell);
    for(Int_t jentry=0; jentry<fChain->GetEntries(); jentry++){
        fChain->GetEntry(jentry);
        printf("%d\n",triggerCell[0][0]);
    }
}