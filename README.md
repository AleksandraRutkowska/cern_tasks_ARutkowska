# Symulacja pp @ 13.6 TeV (Pythia 8 + ROOT)

Autor: Aleksandra Rutkowska

Projekt generuje zderzenia proton-proton przy energii 13.6 TeV w generatorze Pythia 8
(tryb minimum bias, `SoftQCD:all = on`, `Beams:eCM = 13600`) i analizuje cząstki stanu końcowego
(`isFinal()`) w ROOT. Dla każdej kategorii cząstek powstają histogramy krotności (N),
pędu poprzecznego (pT), pseudorapidity (η) i kąta azymutalnego (φ).

## Zawartość repozytorium

| Plik | Opis |
|---|---|
| `simulate.cpp` | generacja zdarzeń w Pythii i zapis histogramów do `analysis.root` |
| `draw_plots.C` | makro ROOT: rysuje wykresy i zapisuje je jako PNG i PDF |
| `CMakeLists.txt` | budowanie programu `simulate` |
| `analysis.root` | gotowe histogramy (wynik symulacji, 100 000 zdarzeń, seed 12345) |
| `Prezentacja_Monte_Carlo_pp_13.6TeV.pdf` | prezentacja ze wszystkimi 32 wykresami |

## Kategorie i histogramy

Kategorie: `all`, `ch`, `pip` (π⁺), `pim` (π⁻), `Kp` (K⁺), `Km` (K⁻), `p`, `pbar` (p̄).

- `all` i `ch` oznaczają **wszystkie cząstki naładowane** stanu końcowego (ładunek ≠ 0),
  więc te dwa zestawy histogramów są identyczne (`ch` służy jako kontrola).
- Kody PDG: π ±211, K ±321, p ±2212.

Dla każdej kategorii zapisywane są histogramy `hN_*`, `hPt_*`, `hEta_*`, `hPhi_*`
(4 zmienne × 8 kategorii = 32 histogramy):

| Zmienna | Zakres | Biny |
|---|---|---|
| N | 0–400 | 400 |
| pT | 0–10 GeV | 100 |
| η | −5 do 5 | 100 |
| φ | 0 do π | 64 |

Uwaga: φ jest wypełniane wartością bezwzględną |φ|, żeby zmieścić się w wymaganym zakresie 0–π.

## Wymagania

- kompilator obsługujący C++17
- CMake ≥ 3.20
- ROOT 6
- Pythia 8 (z ustawioną zmienną `PYTHIA8DATA` wskazującą na `share/Pythia8/xmldoc`,
  jeśli Pythia nie znajdzie jej sama)

Jeśli Pythia 8 jest zainstalowana w niestandardowym miejscu, ustaw zmienną `PYTHIA8`
na katalog instalacji (zawierający `include/` i `lib/`), np.:

```bash
export PYTHIA8=/sciezka/do/pythia8
```

## Kompilacja

```bash
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build .
```

## Uruchomienie

1. Symulacja (w katalogu `cmake-build`):

   ```bash
   ./simulate            # domyślnie 100 000 zdarzeń, seed 12345
   ./simulate 100000 12345   # jawnie: liczba zdarzeń i seed
   ```

   Powstaje plik `analysis.root`. Ten sam seed daje te same wyniki (powtarzalność).

2. Wykresy (w tym samym katalogu):

   ```bash
   root -l -b -q ../draw_plots.C
   ```

   W folderze `plots/` pojawią się:
   - 32 wykresy (po jednym na histogram), każdy jako PNG i PDF (razem 64 pliki),
   - 12 wykresów porównawczych z legendą (π⁺/π⁻, K⁺/K⁻, p/p̄ dla N, pT, η, φ), też w PNG i PDF.