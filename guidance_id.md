# OBS Auto SF Fit — Panduan Pengguna

Panduan ini menjelaskan cara memasang, mengonfigurasi, dan menggunakan plugin **Auto SF Fit** di OBS Studio. Filter ini secara otomatis menangani penskalaan, pemosisian, dan animasi sumber video langsung (seperti input SDI/kartu penangkap) agar pas di dalam area transparan pada overlay Squeeze Frame (SF).

---

## 1. Pengaturan Mulai Cepat (Quick Start)
Untuk mengonfigurasi adegan (scene) Squeeze Frame:
1. **Tambahkan Sumber Anda (Sources)**:
   * Tambahkan sumber video langsung Anda (misalnya SDI, capture card, file media) ke dalam adegan Anda.
   * Tambahkan sumber overlay Squeeze Frame (SF) Anda (misalnya video `.webm`/`.mov`, gambar statis, atau browser source).
2. **Terapkan Filter**:
   * Klik kanan pada **sumber video langsung** Anda di daftar sumber, lalu pilih **Filters** (Filter).
   * Klik tombol `+` di bagian **Effect Filters** (Filter Efek) dan tambahkan **Auto SF Fit**.
3. **Konfigurasikan Pengaturan**:
   * Pilih sumber Squeeze Frame Anda pada menu dropdown **SF Source**.
   * Centang opsi **Enable Timing (In/Hold/Out)** jika Anda menggunakan transisi.
   * Sesuaikan durasi transisi dan mode kecocokan (fit mode) sesuai kebutuhan Anda (dijelaskan di bawah).

---

## 2. Penjelasan & Fungsi Pengaturan

### SF Source (Sumber SF)
* **Fungsi**: Memilih sumber di dalam adegan yang berisi area transparan (cutout) bermotif bingkai.
* **Pentingnya**: Plugin memindai saluran alfa (transparansi) dari sumber ini untuk mendeteksi koordinat area kosong tempat video langsung harus ditempatkan.

### Fit Mode (Mode Kecocokan)
Mengontrol bagaimana video langsung diubah ukurannya agar pas di dalam area transparan yang terdeteksi:
* **Contain (Muat)**: Memperkecil/skala video sehingga seluruhnya pas di dalam area transparan. Rasio aspek (aspect ratio) asli video dipertahankan sehingga tidak ada distorsi gambar. Jika rasio aspek tidak cocok dengan area transparan, Anda akan melihat margin kosong (letterboxing/pillarboxing) di bagian atas/bawah atau kiri/kanan di dalam bingkai.
* **Cover (Penuhi)**: Memperbesar/skala video hingga sepenuhnya mengisi seluruh area transparan. Rasio aspek asli dipertahankan. Jika rasio aspek tidak cocok, tepi luar video akan sedikit terpotong (crop) untuk memastikan **tidak ada celah kosong (zero gaps)** sama sekali.
* **Stretch (Regangkan)**: Meregangkan lebar dan tinggi video secara independen agar cocok dengan dimensi area transparan secara presisi. Gambar video akan mengalami distorsi rasio aspek (tampak mulur atau gepeng), tetapi **tidak ada pemotongan** dan **tidak ada celah**.

### Alpha Threshold (Ambang Batas Alfa)
* **Rentang**: `0` hingga `255` (default `16`).
* **Fungsi**: Mengatur sensitivitas deteksi transparansi. Setiap piksel dengan tingkat opasitas di bawah ambang ini akan dihitung sebagai "transparan" dan menjadi bagian dari area cutout.
* **Tips**: Tingkatkan nilai ini sedikit jika ada derau kompresi (compression noise) pada file video yang menghalangi deteksi tepi transparan yang bersih.

### Padding (Batas Pengisi)
* **Rentang**: `0` hingga `200` piksel (default `0`).
* **Fungsi**: Menambahkan margin pengaman solid ke arah dalam area transparan yang terdeteksi.
* **Tips**: Gunakan nilai `1` atau `2` piksel padding untuk membuat video langsung sedikit tumpang tindih di belakang grafis bingkai guna menyembunyikan garis pembulatan piksel (pixel-rounding lines) yang kasar.

### Recalculation Mode (Mode Pemindaian Ulang)
Mengontrol kapan plugin memindai ulang sumber Squeeze Frame untuk mengkalibrasi koordinat area kosong:
* **Manual Refresh**: Hanya memindai ketika Anda mengeklik tombol **Recalculate Now**. Sangat cocok untuk overlay gambar statis yang posisinya tidak berubah.
* **On Activate**: Memindai sekali ketika adegan (scene) menjadi aktif atau saat dipindahkan. Sangat direkomendasikan untuk transisi berbasis video.
* **Periodic**: Memindai ulang secara berulang pada selang waktu teratur.

### Recalc Interval (ms) (Interval Pemindaian Ulang)
* **Fungsi**: Selang waktu (dalam milidetik) bagi plugin untuk melakukan pemindaian ketika mode *Periodic* aktif.

### Enable Timing (In/Hold/Out) (Aktifkan Transisi)
* **Fungsi**: Menyinkronkan transisi pemerasan (squeeze transition) (masuk, tahan, keluar) dari sumber langsung dan Squeeze Frame secara otomatis.
* **Sinkronisasi**: 
  * Jika sumber SF adalah file video, plugin membaca waktu pemutaran video yang didekodekan (**Media-Sync**) untuk penyelarasan transisi per frame secara sempurna.
  * Jika sumber SF bersifat statis (seperti gambar, browser source, dll.), plugin akan otomatis menggunakan waktu jam sistem sebagai cadangan.

### Transition Durations (Durasi Transisi)
* **In Duration (ms)**: Waktu yang dibutuhkan video langsung untuk menyusut (squeeze) dan overlay SF untuk bergerak masuk ke layar (default `1000ms`).
* **Hold Duration (ms)**: Waktu video tetap tertahan di dalam area bingkai transparan (default `8000ms`).
* **Out Duration (ms)**: Waktu yang dibutuhkan video langsung untuk melebar kembali ke layar penuh dan SF untuk bergerak keluar layar (default `1000ms`).

### SF Animation Mode (Mode Animasi SF)
Mengontrol bagaimana overlay Squeeze Frame dianimasikan selama transisi berlangsung:
* **Static (No Animation) / Statis**: Squeeze Frame dirender pada skala konstan 1:1 dan pergeseran 0. Frame tidak akan bergerak sama sekali. Mode ini sangat ideal jika video Squeeze Frame Anda sudah memiliki animasi transisi bawaan (baked-in) di dalam file video itu sendiri.
* **Slide Only / Geser Saja**: Squeeze Frame dipertahankan pada skala konstan 1:1 (mencegah terjadinya distorsi rasio aspek) dan bergerak menggeser masuk/keluar layar sesuai dengan arah yang ditentukan pada pengaturan **Slide Direction**.
* **Scale & Slide (Lockstep) / Skala & Geser**: Squeeze Frame dan video langsung menyusut/menggeser secara sinkron (lockstep) untuk memastikan tidak ada celah tepi. Perlu dicatat bahwa mode ini akan sedikit mengubah skala grafis dari Squeeze Frame selama transisi.

### Slide Direction (Arah Geser)
* **Pilihan**: Kiri (Left), Kanan (Right), Atas (Top), Bawah (Bottom).
* **Fungsi**: Menentukan arah asal masuknya overlay Squeeze Frame ke layar. Hanya aktif jika *SF Animation Mode* diatur ke *Slide Only*.

### Show Debug Bounding Box (Tampilkan Kotak Debug)
* **Fungsi**: Menggambar garis tepi berwarna hijau di sekitar area transparan terdeteksi.
* **Penggunaan**: Aktifkan opsi ini sementara selama pengaturan awal untuk memastikan area cutout terpindai dengan benar, lalu matikan untuk produksi siaran langsung.

### Overlay SF Source (Tumpuk Sumber SF)
* **Fungsi**: Menggambar grafis Squeeze Frame tepat di atas video langsung.
* **Penggunaan**: Biarkan opsi ini tercentang. Opsi ini memastikan bingkai menumpuk video dengan benar dan menerapkan animasi memudar (opacity) jika ada.

### Recalculate Now (Pindai Sekarang)
* **Fungsi**: Memaksa pemindaian instan terhadap cutout Squeeze Frame. Sangat berguna untuk kalibrasi manual setelah melakukan perubahan posisi sumber.

---

## 3. Praktik Terbaik & Konsep Lanjutan

### Tembolok Kotak Batas Persisten (Persistent Bounding Box Cache)
Plugin secara otomatis menyimpan koordinat kotak batas (bounding box) di dalam koleksi adegan (scene collection) OBS Anda.
* Ini mencegah terjadinya loncatan visual (visual jumping) atau tampilan frame hitam sesaat saat berpindah adegan, karena koordinat yang tepat langsung dimuat sesaat sebelum frame pertama dirender.
* Untuk menghapus tembolok dan memaksa pemindaian baru, klik **Recalculate Now** atau ubah pilihan dropdown **SF Source**.

### Penskalaan Grup (Group Scaling - Pilihan 2)
Plugin melakukan transisi dengan menyelaraskan skala Squeeze Frame dan video langsung secara sinkron (lockstep).
* Squeeze Frame memperkecil skalanya dari ukuran awal yang lebih besar menjadi $1.0$, sedangkan video langsung mengecil dari layar penuh ke dalam area cutout.
* Karena skala dan pergeserannya terjadi secara sinkron, **tidak akan ada celah tepi atau batas visual** yang terbentuk di antara video dan bingkai selama transisi berlangsung.

### Pengaturan Sumber Video (Video Source Settings)
Untuk hasil transisi paling mulus menggunakan file video `.mov`/`.webm` Squeeze Frame:
* Pastikan opsi **Loop** **tidak tercentang** pada properti sumber SF di OBS. Transisi ini dirancang untuk berjalan satu kali lalu selesai.
* Pastikan opsi **Restart playback when source becomes active** (Mulai ulang pemutaran ketika sumber aktif) **tercentang** pada properti properti sumber SF. Ini memicu transisi secara otomatis setiap kali Anda berpindah adegan.
