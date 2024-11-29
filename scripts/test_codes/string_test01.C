#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <TStyle.h>
#include <filesystem>
#include <TString.h>
#include <iostream>
#include <algorithm>
#include <sstream>

double extractDistance_ver2(const TString& filepath){
    TString filename = filepath(filepath.Last('/')+1, filepath.Length()-filepath.Last('/')); //ファイルパスからファイル名だけ抜き出した

    //以下では"22Na_-2_-1cm_dot_..."のような文字列から数字だけ抜き出す
    Ssiz_t firstUnderscorePos = filename.Index('_');
    std::cout << firstUnderscorePos << std::endl;
    Ssiz_t secondUnderscorePos = filename.Index('_',firstUnderscorePos+1);
    std::cout << secondUnderscorePos << std::endl;
    Ssiz_t thirdUnderscorePos = filename.Index('_', secondUnderscorePos+1);
    std::cout << thirdUnderscorePos << std::endl;

    TString firstNumberStr = filename(firstUnderscorePos+1, secondUnderscorePos-firstUnderscorePos-1);
    TString secondNumberStr = filename(secondUnderscorePos+1, thirdUnderscorePos-secondUnderscorePos-1);
    std::cout<< firstNumberStr << " || " << secondNumberStr << std::endl;
    int firstNum = firstNumberStr.Atoi();
    int secondNum = secondNumberStr.Atoi();

    return (firstNum+secondNum)/2.0;
}

Double_t string_test01(){
    TString filename = "../../data/22Na_5_6_cm.dat.root";
    return extractDistance_ver2(filename);
}

