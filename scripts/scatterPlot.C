/*
DRSの個体値の調査のためのプロット
*/
#include <iostream>
#include <fstream>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TStyle.h> //gStyleのところ
#include <TString.h>
#include <TCanvas.h>
using namespace std;
void scatterPlot(TString input_Folder = "./analysis/"){
    TString input_Filepath = Form("%sdata.txt",input_Folder.Data());
    ifstream ifs(input_Filepath);
    Double_t max[2], min[2], vpp, p2p[2];

    TCanvas* canvas = new TCanvas();
    TGraph* graph = new TGraph;
    int index_data = 0;
    while(ifs >> vpp >> max[0] >> min[0] >> p2p[0] >> max[1] >> min[1] >> p2p[1]){
        // graph->SetPoint(index_data, vpp, max[0]*1000);
        // index_data++;
        graph->SetPoint(index_data, vpp, max[1]*1000);
        index_data++;
        // graph->SetPoint(index_data, vpp, min[0]*1000);
        // index_data++;
        // graph->SetPoint(index_data, vpp, min[1]*1000);
        // index_data++;
        // graph->SetPoint(index_data, vpp, p2p[0]*1000);
        // index_data++;
        // graph->SetPoint(index_data, vpp, p2p[1]*1000);
        // index_data++;
    }
    ifs.close();
    // graph->SetTitle(";Function generator Vpp [mV];measured Vpp [mV]");
    graph->SetTitle(";Function generator Vpp [mV];measured Vmax [mV]");
    graph->SetTitle(";Function generator Vpp [mV];measured Vmin [mV]");
    graph->SetMarkerStyle(20);
    gStyle->SetOptFit();
    graph->Fit("pol1");
    graph->Draw("ap"); //axisとpointを描画する

    canvas->SaveAs("./figure/energy_calb.pdf");
}