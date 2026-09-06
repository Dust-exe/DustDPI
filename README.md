# DustDPI — Selective Network Optimization & Traffic Management Service

DustDPI, Windows sistemlerde belirli platform ve istemcilere (Discord, Roblox vb.) giden ag trafigini secmeli olarak optimize eden, gelistirici araclari ve kurumsal baglantilara (Antigravity IDE, bulut servisleri, bankacilik, oyun sunuculari) hicbir mudahalede bulunmadan dogrudan (pass-through) gecis saglayan yuksek performansli bir ag servisidir.

[![Download Setup](https://img.shields.io/badge/Download-DustDPI__Setup.exe-blue?style=for-the-badge&logo=windows)](https://github.com/Dust-exe/DustDPI/releases/download/v1.0.0/DustDPI_Setup.exe)
[![Target](https://img.shields.io/badge/Target-Discord%20%7C%20Roblox-7c3aed?style=for-the-badge&logo=discord&logoColor=white)](https://dust-studio.com)
[![Engine](https://img.shields.io/badge/Engine-WinDivert%20v1.4-slate?style=for-the-badge)](https://dust-studio.com)
[![Status](https://img.shields.io/badge/Antigravity%20Compatible-Verified-10b981?style=for-the-badge)](https://dust-studio.com)

---

## Ozellikler ve Calisma Mantigi

Genel DPI duzenleme araclari calistirildiginda, sistemdeki tum giden baglantilari (port 80 ve 443) yakalayarak paket parcalama uygular. Bu durum Antigravity IDE, Google Cloud, Claude/Anthropic API, VPN oturumlari ve oyun baglantilarinda el sikisma (TLS Handshake) bozulmalarina ve oturum kopmalarina yol acabilir.

DustDPI, secmeli liste (`--blacklist`) ve entegre DNS yonlendirme mimarisiyle bu sorunu cozer:

* **Secici Filtreleme:** Yalnizca `blacklist.txt` icerisinde tanimlanan alan adlari optimize edilir.
* **Dogrudan Gecis (Pass-Through):** Listede yer almayan tum baglantilar (Antigravity IDE, API cagrilari, bankacilik, Steam vb.) sifir gecikmeyle dogrudan iletilir.
* **Masaustu Uygulama Uyumlulugu:** Hem tarayicilarda hem de Discord ve Roblox masaustu istemcilerinde baglanti kopmalarini onlemek icin dahili DNS ve TLS yonetimi saglar.
* **Masaustu Arayuzu (DustDPI.exe):** Koyu tema kontrol paneli, gercek zamanli servis durumu ve sistem tepsisi (System Tray) destegi.
* **Arka Plan Servis Modu:** Windows baslangicinda otomatik devreye girer, acik konsol pencerelerine ihtiyac duymaz.

---

## Dizin Yapisi

```
DustDPI/
├── DustDPI_Setup.exe        # Son kullanici tek tikla kurulum sihirbazi
├── DustDPI.exe              # Modern masaustu kontrol paneli (GUI)
├── DustDPI_Manager.bat      # Komut satiri yonetim menusu
├── blacklist.txt            # Hedef servisler ve alan adlari yapilandirma listesi
├── service_install.cmd      # Servisi otomatik baslatilacak sekilde kurar
├── service_remove.cmd       # Servisi ve suruculeri sistemden kaldirir
├── start_console.cmd        # Konsol test modunda baslatir
├── start_service.cmd        # Arka plan servisini calistirir
├── stop_service.cmd         # Arka plan servisini durdurur
├── status.cmd               # Servis calisma durumunu sorgular
├── x86_64/                  # 64-bit suruculer ve dust_engine motoru
└── x86/                     # 32-bit suruculer ve dust_engine motoru
```

---

## Kurulum ve Kullanim

### 1. Yontem: Kurulum Sihirbazi ile (Tavsiye Edilen)
1. [DustDPI_Setup.exe](https://github.com/Dust-exe/DustDPI/releases/download/v1.0.0/DustDPI_Setup.exe) dosyasini indirin ve calistirin.
2. Kurulum tamamlandiginda servis otomatik olarak baslatilacak ve masaustune `DustDPI` kontrol paneli kisayolu eklenecektir.

### 2. Yontem: Yonetim Arayuzu ile
1. `DustDPI.exe` uygulamasini acin.
2. Servis durumunu kontrol edip tek tikla baslatabilir, durdurabilir veya hedef listesini duzenleyebilirsiniz.

### 3. Yontem: Manuel Kurulum (Konsol)
* **Kurulum:** `service_install.cmd` dosyasina sag tiklayip **"Yonetici Olarak Calistir"** deyin.
* **Kaldirma:** `service_remove.cmd` dosyasini yonetici olarak calistirin.
* **Canli Test:** `start_console.cmd` dosyasini yonetici olarak calistirabilirsiniz.

---

## Lisans ve Gelistirici

* **Gelistirici:** [Dust Studio](https://dust-studio.com)
* **Destek & Iletisim:** contact@dust-studio.com
