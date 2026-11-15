# RWD COLOR LAB — Arsitektur & Persyaratan Teknis (Phase 1)

*By ridwanard.id — Professional Color Transformation & Creative Looks*

Tanggal: 2026-11-13 · Status: **rancangan**. Belum ada kode plugin, belum ada yang dikompilasi, belum ada yang diuji di Resolve.

Legenda status pada tabel: **[V]** terverifikasi dari sumber (lihat bagian Sumber) · **[?]** perlu verifikasi sebelum dipakai · **[X]** tidak bisa dilakukan di lingkungan ini.

---

## 1. Hasil inspeksi

### 1.1 Proyek DCTL lama (RWD Color Tools)
- Tidak ada file proyek DCTL di workspace ini (pencarian file kosong; satu-satunya masukan adalah prompt pengembangan).
- Keputusan: **arsitektur OFX bersih**, sesuai aturan di prompt untuk kasus proyek lama tidak tersedia.
- Tidak ada yang diubah atau dihapus. Folder baru `RWD_COLOR_LAB/` terpisah total. **Simpan folder asli `RWD_COLOR_TOOLS_V1.0` milikmu sebagai backup**; aku tidak punya salinannya.
- DCTL tidak bisa dikonversi langsung ke OFX: DCTL adalah kernel per-piksel yang dijalankan Resolve, sedangkan OFX adalah plugin C++ terkompilasi dengan API host, deskriptor parameter, dan siklus render sendiri. Yang bisa dipakai ulang hanya **rumus matematisnya** (kurva, matriks, logika Skin/Tone) jika file DCTL diunggah; sintaksnya tidak.

### 1.2 Lingkungan sandbox ini

| Item | Hasil |
|---|---|
| OS | Linux x86_64 |
| CMake / Ninja | 3.28.3 / 1.11.1 |
| Kompiler | g++ 13.3, clang++ 18.1 |
| nvcc (CUDA) | tidak ada |
| MinGW / MSVC (Windows) | tidak ada |
| Xcode / Metal compiler (macOS) | tidak ada |
| Jaringan untuk Bash | mati (git clone SDK dan install paket ditolak) |
| DaVinci Resolve | tidak ada |

Konsekuensi jujur:
- **[X]** Binary Windows (.ofx) dan macOS tidak bisa dibuat di sini.
- **[X]** Tidak bisa memuat atau merender di Resolve dari sini.
- Yang realistis di sini: menulis dan menguji **mesin warna CPU** (C++ murni) di Linux, plus tes matematis (round-trip kurva log, keabu-abuan netral, NaN/Inf).
- Kompilasi plugin sungguhan harus di mesin Windows dan Mac milikmu, atau lewat CI (GitHub Actions: runner Windows dan macOS). Tes di Resolve tetap manual di mesinmu.

---

## 2. Verifikasi OpenFX SDK dan Resolve

| Fakta | Status |
|---|---|
| SDK resmi: repositori OpenFX milik Academy Software Foundation, lisensi BSD 3-clause | [V] |
| OpenFX 1.5 dirilis akhir Agustus/awal September 2024; versi 1.4 dari 2015 | [V] |
| 1.5 memakai CMake dan Conan, menambah suite GPU resmi (Metal, CUDA, OpenCL), DrawSuite, parameter `StrChoice`, API color management | [V] |
| Tipe parameter standar: Integer, Double, RGB(A), Boolean, Choice, StrChoice, String, Custom, PushButton, Group, Page, Parametric | [V] |
| PushButton tidak menyimpan nilai; penekanan dikirim lewat `kOfxActionInstanceChanged` | [V] |
| Choice menyimpan indeks bulat 0..N-1; mengubah **urutan** opsi memerlukan `kOfxParamPropChoiceOrder` (1.5) | [V] |
| Suite Metal: dukungan buffer via `kOfxImageEffectPropMetalRenderSupported`, tekstur via properti terpisah; Metal tidak punya format tekstur 3-kanal | [V] |
| Bundle: `NAME.ofx.bundle/Contents/{Info.plist, Resources, MacOS atau Win64/NAME.ofx}`; semua biner berakhiran `.ofx` | [V] |
| Biner macOS sebaiknya universal, ditaruh di `MacOS/`; Resolve di Windows ARM saat ini Arm64EC | [V] |
| Folder instal: Windows `C:\Program Files\Common Files\OFX\Plugins`, macOS `/Library/OFX/Plugins` | [V] |
| Resolve menyimpan daftar plugin di `OFXPluginCacheV2.xml` (cache bisa basi dan membuat plugin baru tidak muncul) | [V] |
| Resolve menyertakan folder `Developer/OpenFX` dengan contoh `GainPlugin` (kernel CUDA, OpenCL, Metal) | [V] (dari forum Blackmagic, 2019) |
| Resolve tidak mengaktifkan render OpenGL untuk OFX; GPU lewat OpenCL/CUDA/Metal | [V] untuk Resolve lama (2017–2019); [?] untuk 19 dan 21 |
| Resolve punya ekstensi non-standar dan tidak mendukung semua suite OFX | [V] (pernyataan komunitas); detail per versi [?] |
| Plugin Windows pihak ketiga memakai OpenCL jika Resolve memberi buffer OpenCL, selain itu CPU | [V] (contoh: OpenEmulsion) |
| Pola dukungan GPU per OS di Resolve: Metal di Mac, OpenCL dan CUDA di Windows | [V] (contoh: Gyroflow) |
| PushButton, Group, Page, Choice dirender benar di Inspector Resolve 19 dan 21 | [?] harus dites |
| Color management OFX 1.5 dihormati oleh Resolve | [?] jangan diandalkan |
| Perbedaan perilaku OFX Resolve 19 vs 21 | [?] belum ada data |

Strategi SDK: kompilasi terhadap header OpenFX 1.5 + Support library, tetapi **hanya memakai API yang juga dipakai contoh Resolve**. Karena Resolve membawa sampel dan header sendiri di folder `Developer/OpenFX`, salinan dari Resolve Studio 21 milikmu adalah acuan paling andal untuk properti GPU yang sebenarnya.

---

## 3. Batasan antarmuka native

OFX tidak punya GUI kustom. Inspector Resolve yang menggambar widget parameter.

| Permintaan di prompt | Kenyataan | Implementasi terbaik |
|---|---|---|
| Antarmuka gelap sinematik ala ToneLab Chroma | Tampilan ditentukan Resolve; tidak ada skin, font, atau warna kustom | Group dan Page bernama jelas, urutan parameter rapi, branding lewat nama plugin dan judul grup |
| Dropdown dengan pencarian | Tidak ada di standar OFX | Choice datar, dikelompokkan lewat awalan label ("Sony — S-Log3 / S-Gamut3") |
| Pratinjau gambar LUT | Tidak didukung parameter standar | Tidak ada; nama look yang jelas |
| Tombol Previous/Next | PushButton ada di standar [V] | PushButton ×2 yang mengubah indeks Choice; tampilan di Resolve [?] dites |
| Nama LUT terpilih tampak | Choice menampilkan opsi terpilih | Choice itu sendiri (ditambah label String hanya-baca bila Resolve mendukung) |
| Pilih kategori | Opsi Choice statis; mengganti isi dropdown secara dinamis [?] | Satu daftar datar 47 look berawalan kategori. Kategori terpisah hanya jika perubahan opsi dinamis terbukti jalan di Resolve |

Aturan stabilitas proyek: urutan indeks Choice **tidak boleh berubah** setelah rilis (proyek tersimpan menyimpan indeks). Opsi baru ditambahkan di akhir; urutan tampilan diatur lewat `kOfxParamPropChoiceOrder`.

---

## 4. Arsitektur modul

```
Plugin/src/
  host/       Deskriptor OFX, describe/describeInContext, render action, GPU/CPU dispatch
  params/     Definisi parameter, ID tetap, pemetaan indeks Choice → enum internal
  engine/
    camera/   Dekode transfer function + matriks gamut per kamera (data, bukan logika)
    working/  Ruang kerja, adaptasi white point, exposure, kontras, saturasi, WB, highlight/shadow
    output/   Matriks gamut keluar + encoding gamma
    looks/    Resep look (parametrik) → LUT 3D; impor/ekspor .cube
  render/
    cpu/      Pemroses CPU (acuan kebenaran), multithread
    gpu/      Kernel Metal, CUDA, OpenCL yang memakai konstanta yang sama
  resources/  Pemuat sumber daya (LUT eksternal, validasi, error)
Resources/    Camera_Profiles, Output_Profiles, Creative_LUTs/{Cinematic,Kodak,Fujifilm,Vintage,Fashion,Mood}
```

Prinsip:
1. **CPU adalah acuan kebenaran.** Setiap kernel GPU dibandingkan piksel-per-piksel dengan CPU (toleransi ditetapkan di Phase 7).
2. **Data terpisah dari logika.** Konstanta kamera (koefisien kurva, matriks) hidup di tabel data yang bisa diaudit satu per satu terhadap dokumen pabrikan.
3. **Transfer function dan gamut dipisah** sesuai prompt: kurva → linear → matriks gamut → ruang kerja.
4. **Tidak ada nilai buatan.** Konstanta yang belum diverifikasi diberi penanda `UNVERIFIED` di tabel data dan tampil sebagai "perlu verifikasi" di dokumentasi, bukan disamarkan.

---

## 5. Pipeline pemrosesan (rencana)

1. Input dari Resolve (float RGBA).
2. Dekode transfer function kamera → linear scene.
3. Matriks gamut kamera → ruang kerja (+ adaptasi white point bila perlu).
4. Exposure (kali 2^stop), white balance, kontras, highlight/shadow, saturasi, di ruang kerja.
5. Creative look (LUT 3D lewat shaper log), blend sesuai **Creative Intensity** di linear.
6. Matriks ruang kerja → gamut output.
7. Encoding gamma output.
8. **Master Amount** dan **Bypass** (lihat pertanyaan terbuka 1).

Aturan umum: tidak ada clamp di tengah pipeline (nilai negatif dari gamut lebar diproses dengan pelindung, bukan dipotong), pelindung NaN/Inf di setiap tahap, blending selalu di domain linear kecuali dinyatakan lain.

Asumsi penting tentang Resolve: plugin menganggap klip yang masuk masih **nilai log kamera mentah** (tanpa CST/IDT sebelum plugin), dan pilihan Conversion Output harus **cocok dengan ruang timeline/monitor**. Jika tidak cocok, hasilnya akan salah dan itu bukan bug plugin. Ini akan masuk README.

---

## 6. Keputusan desain (provisional)

**ADR-1 Ruang kerja: linear Rec.2020 (D65)**, provisional. Alasan: primaries terdokumentasi luas, gamut cukup lebar untuk S-Gamut3/V-Gamut tanpa banyak nilai negatif, tanpa adaptasi chromatic untuk sumber D65. Alternatif: ACEScg (AP1, D60) atau DaVinci Wide Gamut (primaries harus diambil dari white paper Blackmagic, belum diverifikasi). Putusan final di Phase 3 setelah semua matriks kamera diverifikasi dan diuji pada diagram netral dan kulit.

**ADR-2 Look = resep parametrik yang dibake ke LUT 3D.** Resep (lift/gamma/gain, kurva tone, split-tone, kurva saturasi per hue) dijalankan di CPU saat parameter berubah untuk menghasilkan LUT, lalu satu kernel GPU memakai LUT untuk semua 47 look. Keuntungan: satu jalur GPU, mudah ekspor `.cube`, dukungan impor `.cube` pengguna kelak. Risiko: galat interpolasi dan banding pada footage 8-bit; mitigasi: LUT 65³ (≈3,1 MiB), interpolasi tetrahedral, shaper log, tes banding di Phase 5. Putusan final setelah prototipe.

**ADR-3 CPU dulu, GPU kemudian.** Phase 2–6 di CPU dengan kebenaran terukur; GPU di Phase 7 dengan tes paritas.

**ADR-4 Dropdown Choice bertipe integer**, bukan `StrChoice`. `StrChoice` baru ada di OFX 1.5 dan dukungannya di Resolve 19/21 belum terverifikasi [?].

---

## 7. Pustaka kamera (15 profil)

Tidak ada koefisien yang kutulis di fase ini. Semuanya diisi di Phase 3 dari dokumen resmi pabrikan, satu per satu.

| # | Profil | Parameter tambahan | Status data |
|---|---|---|---|
| 1 | Sony S-Log2 / S-Gamut | – | [?] verifikasi di Phase 3 |
| 2 | Sony S-Log2 / S-Gamut3.Cine | – | [?] |
| 3 | Sony S-Log3 / S-Gamut3 | – | [?] |
| 4 | Sony S-Log3 / S-Gamut3.Cine | – | [?] |
| 5 | DJI D-Log | gamut/generasi kamera | [?] |
| 6 | DJI D-Log M | gamut/generasi kamera; bukan alias D-Log | [?] spesifikasi publik perlu dicari |
| 7 | Canon C-Log | pilihan gamut | [?] |
| 8 | Canon C-Log2 | pilihan gamut | [?] |
| 9 | Canon C-Log3 | pilihan gamut | [?] |
| 10 | Panasonic V-Log / V-Gamut | – | [?] |
| 11 | Fujifilm F-Log | gamut (opsi menunggu dokumen) | [?] |
| 12 | Fujifilm F-Log2 | gamut (opsi menunggu dokumen) | [?] |
| 13 | Nikon N-Log | gamut | [?] |
| 14 | Blackmagic Film Gen 5 | – | [?] |
| 15 | GoPro GP-Log | – | [?] risiko: spesifikasi resmi mungkin tidak terbuka |

Aturan: jika dokumen resmi sebuah profil tidak ditemukan, profil itu **tidak** diimplementasikan diam-diam; ia diberi label "perkiraan, belum terverifikasi" atau ditunda.

Sony A6400 (prioritas): pasangan kurva/gamut yang benar per picture profile dan perilaku 8-bit (batas banding, EI/ISO dasar) harus dikonfirmasi dari manual dan dokumen teknis Sony, bukan diasumsikan. Koreksi tambahan untuk 8-bit dibuat **lembut** (tanpa kurva agresif) dan dinilai pada footage nyata milikmu.

---

## 8. Output

| Opsi | Catatan |
|---|---|
| Rec.709 Gamma 2.4 | gamut Rec.709, gamma pangkat 2,4 murni |
| Rec.709 Gamma 2.2 | gamut Rec.709, gamma pangkat 2,2 murni |
| Rec.709-A / Apple-compatible | **[?] definisi persis harus diverifikasi** sebelum implementasi; tidak boleh ditebak |
| sRGB | gamut Rec.709, kurva sRGB piecewise |
| Rec.2020 | gamut Rec.2020; fungsi transfer (gamma 2,4 atau lainnya) ditetapkan di Phase 4 dan harus jelas dilabeli |

Perbedaan keempat Rec.709 variant akan didokumentasikan di `COLOR_SCIENCE.md` di Phase 4.

---

## 9. Creative looks (47)

Cinematic 10 · Kodak-inspired 7 · Fujifilm-inspired 6 · Vintage 6 · Commercial/Fashion 8 · Mood 10 = **47 look**, ditambah entri "Off". Semuanya orisinal ("terinspirasi", bukan emulasi stok film). Syarat tiap look: karakter warna berbeda, diuji pada kulit dan pakaian; tidak boleh ada dua look yang hasilnya hampir sama.

---

## 10. Strategi GPU

- Metal (Mac), CUDA (Windows NVIDIA), OpenCL (Windows AMD/Intel dan fallback). Tiga kernel, satu spesifikasi, satu set konstanta.
- Pola mengikuti contoh `GainPlugin` Resolve; properti sebenarnya diambil dari header dan sampel di Resolve milikmu.
- Metal tidak punya tekstur RGB 3-kanal; pakai buffer RGBA.
- CPU fallback tetap ada untuk host/GPU yang tidak memberi buffer GPU.
- Target performa ditetapkan setelah ada angka nyata.

---

## 11. Build dan distribusi

| | Windows | macOS |
|---|---|---|
| Toolchain | Visual Studio 2022 (MSVC x64), CMake, Ninja/VS generator | Xcode (clang), CMake |
| Arsitektur | x64 (Arm64EC opsional kelak) | universal arm64 + x86_64 |
| Bundle | `RWD_COLOR_LAB.ofx.bundle/Contents/Win64/RWD_COLOR_LAB.ofx` | `.../Contents/MacOS/RWD_COLOR_LAB.ofx` + `Info.plist` |
| Instal | `C:\Program Files\Common Files\OFX\Plugins` | `/Library/OFX/Plugins` |
| GPU | CUDA toolkit (opsional), OpenCL SDK | Metal (bawaan Xcode) |
| Signing | tidak wajib | code signing; notarization bila dibagikan ke orang lain |

Mesin Mac Apple Silicon (M2 Pro) paling logis sebagai tempat build dan tes utama; Windows lewat mesin Windows atau CI. Setelah instal, hapus cache `OFXPluginCacheV2.xml` bila plugin tidak muncul.

---

## 12. Risiko dan pertanyaan terbuka

1. **Master Amount vs Bypass.** Input aslinya adalah log kamera, outputnya display-referred. Blend langsung keduanya terlihat seperti dissolve, bukan "jumlah efek". Pilihan:
   - (a) Master Amount mencampur hasil akhir dengan **input asli** (sesuai prompt; Bypass = input asli persis).
   - (b) Master Amount menskala **hanya grade** (kontrol global + creative), konversi kamera dan output tetap aktif.
   Menunggu keputusanmu. Default sementara: (a).
2. Spesifikasi publik D-Log M dan GP-Log mungkin tidak lengkap.
3. Perilaku Inspector Resolve (PushButton, Group, Choice panjang) pada versi 19 dan 21 belum diuji.
4. Perbedaan perilaku GPU OFX antara Resolve 19 dan 21 belum ada datanya.
5. Tanpa SDK dan Resolve di sandbox ini, tidak ada klaim compile/load/render sampai kamu mengujinya.

---

## 13. Rencana Phase 2 (fondasi OFX + pemrosesan piksel dasar)

Butuh salah satu: (a) header OpenFX 1.5 (clone GitHub; mengaktifkan jaringan Bash di pengaturan, berlaku di chat baru), atau (b) unggah ZIP SDK / folder `Developer/OpenFX` dari Resolve milikmu. Keluaran: `CMakeLists.txt`, deskriptor plugin, satu parameter Exposure yang benar-benar mengubah piksel di CPU, tes unit mesin warna di Linux, instruksi build untuk Windows dan Mac. Status kompilasi dan muat di Resolve dilaporkan terpisah sesuai `COMPATIBILITY.md`.

---

## Sumber

- OpenFX 1.5 release notes — https://openfx.readthedocs.io/en/main/ReleaseNotes/relnotes-1.5.html
- OpenFX Parameters — https://openfx.readthedocs.io/en/main/Reference/ofxParameter.html
- OpenFX Metal render suite — https://openfx.readthedocs.io/en/main/Reference/api/group/group__MetalRender.html
- OpenFX packaging — https://openfx.readthedocs.io/en/main/Reference/ofxPackaging.html
- CG Channel, OpenFX 1.5 — https://www.cgchannel.com/2024/09/openfx-1-5-has-been-released/
- Lokasi plugin OFX di Resolve — https://davinciresolveclub.com/davinci-resolve-openfx-plugins-folder/
- Forum Blackmagic, OFX Plugin Examples — https://forum.blackmagicdesign.com/viewtopic.php?p=536611
- Forum Blackmagic, OpenGL di OFX Resolve — https://forum.blackmagicdesign.com/viewtopic.php?p=384511
- Forum Blackmagic, folder Developer/OpenFX — https://forum.blackmagicdesign.com/viewtopic.php?p=756331
- Gyroflow OFX (dukungan GPU per OS) — https://docs.gyroflow.xyz/app/video-editor-plugins/davinci-resolve-openfx
- OpenEmulsion (OpenCL dengan fallback CPU) — https://github.com/technomancer702/openemulsion/releases
