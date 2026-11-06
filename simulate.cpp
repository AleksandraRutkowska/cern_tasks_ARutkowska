// Autor: Aleksandra Rutkowska
// Symulacja zderzen pp @ 13.6 TeV (Pythia 8, SoftQCD / minimum bias)
// i zapis histogramow (N, pT, eta, phi) do pliku analysis.root.
//
// Uzycie:  ./simulate [liczba_zdarzen] [seed]
//          domyslnie: 100000 zdarzen, seed = 12345 (wynik powtarzalny)

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Pythia8/Pythia.h"
#include "TFile.h"
#include "TH1D.h"

using namespace std;
using namespace Pythia8;

struct CategoryHists {
    TH1D* hN   = nullptr;
    TH1D* hPt  = nullptr;
    TH1D* hEta = nullptr;
    TH1D* hPhi = nullptr;
    int eventCount = 0;  // liczba cząstek danej kategorii w biezacym zdarzeniu
};

void wstawDane(CategoryHists& ch, double pt, double eta, double phi) {
    // dodajemy po 1 do binu
    ch.hPt->Fill(pt);
    ch.hEta->Fill(eta);
    ch.hPhi->Fill(phi);
    ch.eventCount++;
}

CategoryHists stworzHistogramy(const string& cat) {
    CategoryHists ch; // puste
    // #eta i p_{T} to zapis LaTeX-owy ROOTa, dzieki temu rysuje sie z tego grecka litera i indeks dolny
    // zakresy z polecenia
    ch.hN = new TH1D(("hN_" + cat).c_str(),
                       ("Krotnosc - " + cat + ";Multiplicity N;Liczba zdarzen").c_str(), 400, 0, 400);
    ch.hPt = new TH1D(("hPt_" + cat).c_str(),
                       ("Ped poprzeczny - " + cat + ";p_{T} [GeV];Liczba czastek").c_str(), 100, 0.0, 10.0);
    ch.hEta = new TH1D(("hEta_" + cat).c_str(),
                       ("Pseudorapidity - " + cat + ";#eta;Liczba czastek").c_str(), 100, -5.0, 5.0);
    ch.hPhi = new TH1D(("hPhi_" + cat).c_str(),
                       ("Kat azymutalny - " + cat + ";#phi [rad];Liczba czastek").c_str(), 64, 0.0, M_PI);
    return ch;
}

int main(int argc, char* argv[]) {
    int nEvents = 100000;
    int seed = 12345;
    if (argc > 1) nEvents = atoi(argv[1]);
    if (argc > 2) seed = atoi(argv[2]);
    // zabiezpieczamy przed bledna wartoscia
    if (nEvents <= 0 || seed < 1) {
        cerr << "Uzycie: ./simulate [liczba_zdarzen > 0] [seed >= 1]" << endl;
        return 1;
    }
    cout << ">>> Start symulacji: " << nEvents << " zdarzen, seed = " << seed << endl;

    Pythia pythia;
    pythia.readString("Beams:eCM = 13600."); //energia w ukladzie srodka masy
    pythia.readString("SoftQCD:all = on");  // minimum bias
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = " + to_string(seed));
    pythia.readString("Next:numberShowInfo = 0");
    pythia.readString("Next:numberShowProcess = 0");
    pythia.readString("Next:numberShowEvent = 0");

    // uruchamiamy generator
    if (!pythia.init()) {
        cerr << "Blad startu Pythia 8!" << endl;
        return 1;
    }
    // tworzenie lub nadpisanie pliku
    TFile* outFile = TFile::Open("analysis.root", "RECREATE");
    if (!outFile || outFile->IsZombie()) {
        cerr << "Nie mozna utworzyc pliku analysis.root." << endl;
        return 1;
    }

    const vector<string> categories = {"all", "ch", "pip", "pim", "Kp", "Km", "p", "pbar"};
    map<string, CategoryHists> histMap; // slownik: nazwa kategorii, jej 4 histogramy i licznik
    for (const string& cat : categories) {
        histMap[cat] = stworzHistogramy(cat);
    }

    int nFailed = 0;
    for (int iEvent = 0; iEvent < nEvents; ++iEvent) {
        // generujemy 1 zdarzenie
        if (!pythia.next()) {
            ++nFailed; //zapamietaj porazke
            continue;
        }
        //przechodzimy po slowniku
        for (auto& pair : histMap) {
            pair.second.eventCount = 0;
        }

        for (int i = 0; i < pythia.event.size(); ++i) {
            const Particle& p = pythia.event[i];
            // chcemy zostawic tylko czastki ktore faktycznie dolecialy do detektora
            if (!p.isFinal()) continue;

            const double pt  = p.pT();
            const double eta = p.eta();
            const double phi = fabs(p.phi());
            const int id = p.id();
            // jesli ladunek nie 0 = prawda
            if (p.isCharged()) {
                wstawDane(histMap["all"], pt, eta, phi);
                wstawDane(histMap["ch"], pt, eta, phi);
            }
            // tu tylko sortujemy czastki wg kodu
            if (id == 211)   wstawDane(histMap["pip"], pt, eta, phi);
            if (id == -211)  wstawDane(histMap["pim"], pt, eta, phi);
            if (id == 321)   wstawDane(histMap["Kp"], pt, eta, phi);
            if (id == -321)  wstawDane(histMap["Km"], pt, eta, phi);
            if (id == 2212)  wstawDane(histMap["p"], pt, eta, phi);
            if (id == -2212) wstawDane(histMap["pbar"], pt, eta, phi);
        }
        // chcemy tylko 1 wpis na zdarzenie na kategorie
        for (auto& pair : histMap) {
            pair.second.hN->Fill(pair.second.eventCount);
        }
        // co 10 tys wypisujemy postep
        if ((iEvent + 1) % 10000 == 0) {
            cout << "Postep: " << (iEvent + 1) << " / " << nEvents << endl;
        }
    }

    outFile->cd();
    for (const string& cat : categories) {
        CategoryHists& ch = histMap[cat];
        ch.hN->Write();
        ch.hPt->Write();
        ch.hEta->Write();
        ch.hPhi->Write();
    }
    outFile->Close();

    pythia.stat(); // wypisuje podsumowanie generatora
    if (nFailed > 0) cout << "Uwaga: pominieto " << nFailed << " nieudanych zdarzen." << endl;
    cout << ">>> Gotowe! Zapisano plik analysis.root." << endl;
    return 0;
}