#include <TH1D.h>
#include <TGraph.h>
#include <TF1.h>
#include <cmath>
#include <TCanvas.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TLegend.h>

double f(double x, double mu){
    double r = 0.5 * sqrt(511 / (2 - cos(mu))) / (2 * sqrt(2 * log(2))); //count rate
    
    return pow(2 * M_PI * pow(r, 2), -0.5) * exp(-0.5 * pow((x - mu) / r, 2));
}

void test8(){
    int points = 1;
    int total_bin = 1000;
    double xValue[points], yValue[points], nd[points], areaValue[points];
    double xhist[total_bin], sumhist[total_bin];
    double width = 0.5;
    double sum = 0.0;
    TCanvas* c1 = new TCanvas("c1", "c1");
    // TH1D* h1 = new TH1D("h1", "h1", 10, -10, 10);
    // h1->GetYaxis()->SetRangeUser(0, 0.16);
    // h1->Draw();

    double min = -5.0;
    double max = 10.0;
    
    for(int i = 0; i < points; i++){
        xValue[i] = 1 + i * width;
        yValue[i] = xValue[i];
        areaValue[i] = yValue[i] * width;
        double area = areaValue[i];
        double mu = xValue[i];
        // TF1* func0 = new TF1("f0", "0", -10, 10);
        // func0->Draw();
        // if(i == 0){
        // TF1* func = new TF1("f1", "[0] * f(x, [1])", -10, 10);
        // func->SetParameters(area, mu);
        // func->Draw("same");
        // }
        // else{
        //     TF1* func = new TF1("f1", "[0] * f(x, [1])", -10, 10);
        //     func->SetParameters(area, mu);
        //     func->Draw("same");
        // }
        TH1D* hist = new TH1D("hist", "hist", total_bin, min, max);
        for(int j = 0; j < total_bin; j++){
                xhist[j] = min + (max - min) / total_bin * j;
                hist->Fill(xhist[j], area * f(xhist[j], mu));
                // hist->Fill(xhist[j], 5);
                sumhist[j] = (max - min) / total_bin * area * f(xhist[j], mu);
                sum += sumhist[j];
            }
            
            
            
        
        hist->Draw();
        std::cout << area << std::endl;
        std::cout << sum << std::endl;
    }
}

Double_t gauss_func(Double_t x_difference, Double_t volume, Double_t x_mean, Double_t sigma_x){
    // std::cout << x_difference << std::endl;
    return(volume*(1/sqrt(2*M_PI*pow(sigma_x, 2)))*exp(-(pow(x_difference,2)/(2*pow(sigma_x,2)))));
}
void test8_1(){
    //ここでは、ある範囲のxに対して分布が存在し、その分布の各ビンがあるxの標準偏差を持ってバラけた場合にどうなるか考察してみる。
    Int_t xmin = 0;
    Int_t xmax = 10;
    Int_t nBins = (xmax-xmin)*100;
    std::cout << "xmin : " << xmin << " || xmax : " << xmax << " || nBins : " << nBins << std::endl;

    // TCanvas* canvas = new TCanvas("canvas", "canvas", 800, 600);
    // canvas->Divide(2,1);
    TH1F* h1 = new TH1F("h1", "ideal distribution(, and real distribution)", nBins, xmin-xmax, xmax+xmax);
    gStyle->SetOptStat(0);
    
    Int_t numFilledBins = 0;
    for(Int_t bin_Index=1; bin_Index<=nBins; bin_Index++){
        if(h1->GetBinCenter(bin_Index) > xmin && h1->GetBinCenter(bin_Index) < xmax)
        // h1->SetBinContent(bin_Index, 1000+100*(xmax-xmin)*bin_Index/nBins);
        // h1->SetBinContent(bin_Index, 1e6*(bin_Index/static_cast<Double_t>(nBins)));
        // h1->SetBinContent(bin_Index, 1e6*(cos(bin_Index/360.0*2*M_PI)));
        h1->SetBinContent(bin_Index, 1e6*(sin(bin_Index/500.0)));

        numFilledBins++;
    }
    // canvas->cd(1);
    h1->SetLineColor(kRed);
    h1->Draw();

    TH1F* h2 = new TH1F("h2", "real distibution", nBins, xmin-xmax, xmax+xmax);

    Int_t div = (xmax-xmin)*1e2;
    std::cout << "div : " << div << std::endl;
    for(Int_t bin_Index=1; bin_Index<=nBins; bin_Index++){
        if(h1->GetBinContent(bin_Index) != 0){
            Double_t delta_x = (xmax-xmin)/static_cast<double_t>(div); //used for obtain where to fill
            Double_t x_mean = h1->GetBinCenter(bin_Index);
            std::cout << "x_mean : " << x_mean << std::endl;
            Double_t numContents = h1->GetBinContent(bin_Index); //the volume of gaus distribution
            std::cout << "numContents : " << numContents << std::endl;
            for(Int_t x_difference_Index=0; x_difference_Index < div; x_difference_Index++){
                Double_t nToFill = gauss_func(x_difference_Index*delta_x, numContents/numFilledBins*(xmax-xmin), 0, 1);

                Int_t nToFill_Int = static_cast<Int_t>(nToFill);
                // std::cout << "nToFill : " << nToFill <<std::endl;
                // std::cout << "nToFill_Int : " << nToFill_Int <<std::endl;
                if(x_difference_Index == 0){
                    for(Int_t i=0; i<nToFill_Int; i++){
                        h2->Fill(x_mean);
                    }
                }
                else{
                    for(Int_t i=0; i<nToFill_Int; i++){
                        h2->Fill(x_mean + delta_x*x_difference_Index);
                        h2->Fill(x_mean - delta_x*x_difference_Index);
                    }
                }
            }
        }
    }
    h2->SetLineColor(kBlue);
    h2->Draw("same");

    auto legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->AddEntry(h1, "ideal distribution", "l");
    legend->AddEntry(h2, "real dist", "l");
    legend->Draw();

}
    
