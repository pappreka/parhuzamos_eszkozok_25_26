# Julia-halmaz generálása C és OpenCL segítségével

## 1. A feladat ismertetése

A projekt célja egy Julia-halmazt előállító alkalmazás megvalósítása C nyelven két különböző módon:

- soros CPU-alapú implementációval,
- OpenCL segítségével párhuzamosított változattal.

A program a komplex sík pontjaira alkalmazott iteráció segítségével fraktálképet generál, majd összehasonlítja a CPU és a GPU (OpenCL) futási idejét különböző felbontások és iterációszámok mellett. Az eredmények CSV fájlba kerülnek, így azok könnyen kiértékelhetők.

---

## 2. Matematikai háttér

A Julia-halmaz az alábbi iteráció alapján definiálható:

z(n+1) = z(n)^2 + c

ahol:

- z komplex szám,
- c egy rögzített komplex konstans.

A képpontokhoz tartozó komplex számokra alkalmazzuk az iterációt, és azt vizsgáljuk, hogy a sorozat kilép-e a |z| > 2 tartományból. Az iterációk száma határozza meg a pixel színét.

---

## 3. Rövid irodalomkutatás

A Julia-halmazok nevüket Gaston Julia után kapták [1], aki a 20. század elején a komplex függvények iterációját vizsgálta. Munkássága alapvető volt a komplex dinamikai rendszerek fejlődésében.

A Julia-halmazokkal párhuzamosan Pierre Fatou [2] is jelentős eredményeket ért el, és a két matematikus együtt tekinthető a terület megalapozójának.

A fraktálgeometria későbbi fejlődésében kulcsszerepet játszott Benoît Mandelbrot [3], aki a fraktálok fogalmát széles körben ismertté tette, különösen a *The Fractal Geometry of Nature* (1982) című művével.

A Julia-halmazok matematikai leírása megtalálható a szakirodalomban [4].

---

## 4. Párhuzamosítás

A feladat kiválóan párhuzamosítható, mivel minden pixel számítása egymástól független.

Az OpenCL megvalósításban:

- minden pixel egy külön work-item-ben kerül feldolgozásra,
- a GPU több ezer szálon végzi a számítást,
- nincs szükség szinkronizációra.

Ez a probléma tipikus adatpárhuzamos (data-parallel) feladat.

---

## 5. Program felépítése

A projekt fő komponensei:

- `main.c` – programindítás, paraméterek kezelése, mérés  
- `julia_cpu.c` – soros implementáció  
- `julia_opencl.c` – OpenCL inicializálás és futtatás  
- `kernel.cl` – párhuzamos kernel  
- `image.c` – kép mentése  
- `timer.c` – időmérés és CSV export  

---

## 6. Futtatás

```bash
./julia width height max_iter c_real c_imag output results.csv
```

---

## 7. Mérési módszer

A mérések célja a soros CPU és az OpenCL-alapú párhuzamos megvalósítás teljesítményének összehasonlítása volt különböző paraméterek mellett.

A vizsgálatok során két fő tényezőt elemeztem:

- a felbontás hatását fix iterációszám mellett,
- az iterációszám hatását fix felbontás mellett.

### Vizsgált paraméterek

- Felbontások:
  - 800×600
  - 1280×720
  - 1920×1080
  - 2560×1440
  - 3840×2160

- Iterációszámok:
  - 100
  - 300
  - 500
  - 1000

- Julia-konstans minden esetben:
  - c = -0.7 + 0.27015i

### Mérési eljárás

- Minden egyes paraméterkombinációt 10 alkalommal futtattam.
- A futási időket a program `gettimeofday()` alapú időméréssel határozta meg.
- A mérés tartalmazta:
  - a CPU számítás idejét,
  - az OpenCL teljes futási idejét (inicializálás + kernel futás + adatmásolás).
- Az egyes mérések eredményei CSV fájlba kerültek.
- A végső értékeket az ismételt futások átlagaként határoztam meg.

### Megjegyzések

- Az OpenCL futási idő tartalmazza az inicializációs és memória-kezelési költségeket is, ezért kis problémaméretnél ez torzíthatja az eredményeket.
- A GPU előnye főként nagyobb felbontások és nagyobb iterációszámok esetén jelentkezik.
- Az ismételt futtatások célja a mérési zaj csökkentése és stabilabb eredmények elérése volt.

---

## 8. Eredmények

### 8.1 Felbontás hatása (max_iter = 500)

| Felbontás | CPU (ms) | OpenCL (ms) | Gyorsulás |
|----------|---------|------------|----------|
| 800×600  | 540.48  | 958.24     | 0.57×    |
| 1280×720 | 1055.79 | 930.05     | 1.14×    |
| 1920×1080| 2330.53 | 975.42     | 2.41×    |
| 2560×1440| 3663.96 | 931.71     | 3.95×    |
| 3840×2160| 7601.30 | 1030.67    | 7.39×    |

---

### 8.2 Iteráció hatása (1920×1080)

| Iteráció | CPU (ms) | OpenCL (ms) | Gyorsulás |
|---------|---------|------------|----------|
| 100     | 1097.39 | 1000.40    | 1.10×    |
| 300     | 1880.80 | 901.21     | 2.09×    |
| 500     | 2330.53 | 975.42     | 2.41×    |
| 1000    | 2221.85 | 921.01     | 2.43×    |

---

### 8.3 Iteráció hatása (3840×2160)

| Iteráció | CPU (ms) | OpenCL (ms) | Gyorsulás |
|---------|---------|------------|----------|
| 100     | 3519.17 | 954.74     | 3.68×    |
| 300     | 6618.19 | 995.81     | 6.73×    |
| 500     | 7601.30 | 1030.67    | 7.39×    |
| 1000    | 8220.63 | 1003.64    | 8.20×    |

---

## 9. Eredmények értékelése

A mérések alapján az alábbi megállapítások tehetők:

- kis felbontásnál az OpenCL lassabb (overhead miatt),
- közepes méreteknél már gyorsabb,
- nagy felbontásnál jelentős gyorsulás figyelhető meg,
- 4K esetén akár 8× gyorsulás is elérhető.

### Legjobb mérés

- ~8.9× gyorsulás (3840×2160, 1000 iteráció)

### Legrosszabb mérés

- ~0.18× gyorsulás (kis felbontás, alacsony iterációszám)

---

## 10. Következtetések

A projekt eredményei alapján az alábbi következtetések vonhatók le:

- **A Julia-halmaz kiválóan párhuzamosítható feladat.**  
  Ennek oka, hogy minden pixel számítása egymástól teljesen független. Az iteráció során nincs szükség más pixelek eredményére, így a számítás könnyen felosztható sok kisebb részfeladatra. Ez ideálissá teszi a problémát adatpárhuzamos (data-parallel) feldolgozásra.

- **GPU használatával jelentős gyorsulás érhető el.**  
  A mérések alapján látható, hogy nagyobb felbontásoknál az OpenCL-alapú megoldás többszörös gyorsulást biztosít a CPU-hoz képest. A GPU képes több ezer szálon egyszerre végrehajtani a számításokat, ami különösen hatékony az ilyen típusú feladatoknál.

- **A párhuzamosítás csak nagy adatmennyiségnél hatékony.**  
  Kis felbontás és alacsony iterációszám esetén az OpenCL implementáció gyakran lassabb, mivel a futási idő jelentős részét az inicializálás, a memóriafoglalás és az adatmásolás teszi ki. Nagyobb problémaméreteknél azonban ez az overhead elhanyagolhatóvá válik, és a párhuzamos végrehajtás előnye dominál.

- **A projekt jól demonstrálja a CPU és GPU közötti különbségeket.**  
  A CPU kevesebb, de erősebb maggal rendelkezik, ezért kisebb feladatoknál hatékonyabb lehet. Ezzel szemben a GPU sok egyszerű magot tartalmaz, amelyek párhuzamosan dolgoznak, így nagy mennyiségű, egymástól független számítás esetén jelentős teljesítménynövekedést biztosít.

---

## 11. Összefoglalás

A projekt sikeresen demonstrálja a Julia-halmaz generálását és a párhuzamos számítás gyakorlati alkalmazását. A feladat során egy soros CPU-alapú és egy OpenCL-alapú párhuzamos megoldás került megvalósításra, amelyek összehasonlítása jól szemlélteti a különböző számítási modellek közötti eltéréseket.

A mérések alapján egyértelműen megfigyelhető, hogy az OpenCL alapú megoldás nagyobb problémaméretek esetén jelentős teljesítménynövekedést eredményez. Míg kisebb feladatoknál az inicializációs költségek miatt a CPU hatékonyabb lehet, addig nagyobb felbontás és iterációszám esetén a GPU párhuzamos feldolgozása egyre nagyobb előnyt biztosít.

A projekt így nemcsak a Julia-halmaz matematikai és vizuális tulajdonságait mutatja be, hanem jól illusztrálja a párhuzamos programozás előnyeit és korlátait is, különös tekintettel a CPU és GPU közötti különbségekre.

---

## 12. Hivatkozások

[1] MacTutor History of Mathematics – Gaston Julia  
https://mathshistory.st-andrews.ac.uk/Biographies/Julia/

[2] MacTutor History of Mathematics – Pierre Fatou  
https://mathshistory.st-andrews.ac.uk/Biographies/Fatou/

[3] Encyclopaedia Britannica – Benoît Mandelbrot  
https://www.britannica.com/biography/Benoit-Mandelbrot

[4] Wolfram MathWorld – Julia Set  
https://mathworld.wolfram.com/JuliaSet.html
