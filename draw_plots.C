// Autor: Aleksandra Rutkowska
// Makro ROOT: rysuje histogramy z analysis.root i zapisuje je jako PNG i PDF
//
// Uruchomienie:  root -l -b -q draw_plots.C


#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
// te od roota
#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TSystem.h"

using namespace std;

struct VarInfo {
    string key;
    string title;
    string xTitle;
    string yTitle;
    bool logY; // prawda dla N i pT bo w nich chcemy os Y w sklai logarytmicznej
};

void setupCanvas(TCanvas* c, bool logY) {
    c->SetLeftMargin(0.18);
    c->SetRightMargin(0.05);
    if (logY) c->SetLogy();
}

void styleHist(TH1D* h, const VarInfo& v, const string& label, int color) {
    h->SetTitle((v.title + " - " + label + ";" + v.xTitle + ";" + v.yTitle).c_str());
    // zeby ladniej wygladalo i bylo widac wszystkie podpisy :)
    h->GetXaxis()->SetTitleSize(0.045);
    h->GetYaxis()->SetTitleSize(0.045);
    h->GetXaxis()->SetLabelSize(0.04);
    h->GetYaxis()->SetLabelSize(0.04);
    h->GetXaxis()->SetTitleOffset(1.1);
    h->GetYaxis()->SetTitleOffset(1.7);
    h->SetLineColor(color);
    h->SetLineWidth(2);
    if (!v.logY) h->SetMinimum(0); // wazne bo log z 0 nie istnieje
}

void saveCanvas(TCanvas* c, const string& name) {
    c->SaveAs(("plots/" + name + ".png").c_str());
    c->SaveAs(("plots/" + name + ".pdf").c_str());
}
// wykres porownawczy 2 kategorii na 1 canvasie
void drawOverlay(TFile* f, const VarInfo& v, const string& a, const string& b,
                 const string& labelA, const string& labelB) {
    TH1D* h1 = (TH1D*)f->Get(("h" + v.key + "_" + a).c_str());
    TH1D* h2 = (TH1D*)f->Get(("h" + v.key + "_" + b).c_str());
    if (!h1 || !h2) return;

    TH1D* c1 = (TH1D*)h1->Clone(("cl1_h" + v.key + "_" + a).c_str());
    TH1D* c2 = (TH1D*)h2->Clone(("cl2_h" + v.key + "_" + b).c_str());
    c1->SetDirectory(nullptr);
    c2->SetDirectory(nullptr);

    string name = "cmp_h" + v.key + "_" + a + "_" + b;
    TCanvas* c = new TCanvas(("c_" + name).c_str(), name.c_str(), 800, 600);
    setupCanvas(c, v.logY);

    styleHist(c1, v, labelA + " vs " + labelB, kBlue + 2);
    styleHist(c2, v, labelA + " vs " + labelB, kRed + 1);

    double mx = max(c1->GetMaximum(), c2->GetMaximum());
    c1->SetMaximum(v.logY ? 5.0 * mx : 1.3 * mx); // zapas nad wykresem
    if (v.logY) c1->SetMinimum(0.5);

    c1->Draw("HIST");
    c2->Draw("HIST SAME");

    TLegend* leg = new TLegend(0.68, 0.76, 0.93, 0.90);
    leg->SetBorderSize(0);
    leg->AddEntry(c1, labelA.c_str(), "l");
    leg->AddEntry(c2, labelB.c_str(), "l");
    leg->Draw();

    saveCanvas(c, name);
    delete c;
    delete c1;
    delete c2;
}

void draw_plots() {
    gROOT->SetBatch(kTRUE); // tryb bez okienek

    TFile* f = nullptr;
    for (const char* path : {"analysis.root", "../analysis.root", "cmake-build/analysis.root"}) {
        f = TFile::Open(path); //probuje otworzyc
        if (f && !f->IsZombie()) break; //udalo sie
        f = nullptr; // nie udalo sie - zapominamy wynik
    }
    if (!f) {
        cerr << "Brak pliku analysis.root! Uruchom najpierw ./simulate." << endl;
        return;
    }

    gSystem->mkdir("plots", true);

    const vector<string> cats = {"all", "ch", "pip", "pim", "Kp", "Km", "p", "pbar"};
    const vector<VarInfo> vars = {
        {"N",   "Krotnosc",           "Multiplicity N", "Liczba zdarzen", true}, //N:log bo szybki spadek
        {"Pt",  "Ped poprzeczny",     "p_{T} [GeV]",    "Liczba czastek", true}, // pT:log bo stromy spadek
        {"Eta", "Pseudorapidity",     "#eta",           "Liczba czastek", false}, //eta:skala liniowa
        {"Phi", "Kat azymutalny",     "#phi [rad]",     "Liczba czastek", false}, //phi: skala liniowa
    };

    gStyle->SetOptStat(1111);
    gStyle->SetHistLineWidth(2);
    for (const string& cat : cats) { // dla kazdej z 8 kategorii
        for (const VarInfo& v : vars) { // dla kazdej z 4 zmiennych
            string hName = "h" + v.key + "_" + cat;
            TH1D* h = (TH1D*)f->Get(hName.c_str());
            if (!h) {
                cerr << "Brak histogramu!!! " << hName << endl;
                continue;
            }
            TCanvas* c = new TCanvas(("c_" + hName).c_str(), hName.c_str(), 800, 600);
            setupCanvas(c, v.logY);
            styleHist(h, v, cat, kBlue + 2);
            h->Draw("HIST");
            saveCanvas(c, hName);
            delete c;
        }
    }

    gStyle->SetOptStat(0);
    for (const VarInfo& v : vars) {
        drawOverlay(f, v, "pip", "pim", "#pi^{+}", "#pi^{-}"); //piony
        drawOverlay(f, v, "Kp", "Km", "K^{+}", "K^{-}"); // kaon
        drawOverlay(f, v, "p", "pbar", "p", "#bar{p}"); // proton i antyproton
    }

    f->Close();
    cout << ">>> Gotowe! Wykresy są w folderze 'plots/'." << endl;
}