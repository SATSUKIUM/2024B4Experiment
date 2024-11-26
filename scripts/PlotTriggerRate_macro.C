#include <TROOT.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <TString.h>
#include <TApplication.h>
void PlotTriggerRate_macro(std::string folderpath = "../../data/dot_root/20241112/good/"){
    int argc = gApplication->Argc();
    char** argv = gApplication->Argv();
    std::cout << std::endl << "number of commandline arguments: " << argc << std::endl;
    std::cout << std::endl << "second commandline argument: " << argv[3] << std::endl;
    const char* str = argv[3];
    folderpath = str;
    gROOT->ProcessLine(".L DRS4Ana.C");
    gROOT->ProcessLine(Form("DRS4Ana obj(\"%s\")",folderpath.c_str()));
    gROOT->ProcessLine("obj.PlotTriggerRate()");


}