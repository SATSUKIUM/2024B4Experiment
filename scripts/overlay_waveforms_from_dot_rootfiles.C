#include <TROOT.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <TString.h>
void overlay_waveforms_from_dot_rootfiles(std::string folderpath = "dot_root/"){
    for(const auto& entry : std::filesystem::directory_iterator(folderpath)){
        
    }

    gROOT->ProcessLine(".L DRS4Ana.C");
    int numFiles = 0;
    for(const auto& entry : std::filesystem::directory_iterator(folderpath)){
        TString line1 = Form("DRS4Ana obj(\"%s\")", entry.path().c_str());
        gROOT->ProcessLine(line1);
        gROOT->ProcessLine("obj.Output_chargeintegral(0,20,0,100)");
        gROOT->ProcessLine("delete obj;");
        numFiles++;
        std::cout << "numFiles : " << numFiles << std::endl;
    }
    gROOT->ProcessLine(".q");
}