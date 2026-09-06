# 🛡️ DustDPI — Selective DPI Circumvention Service

> **Zero Collateral Damage, High-Performance Selective Windows DPI Circumvention Engine for Turkey Blocked Platforms (Discord, Roblox & More)**

[![Download Setup](https://img.shields.io/badge/Download-DustDPI__Setup.exe-brightgreen?style=for-the-badge&logo=windows)|(https://github.com/Dust-exe/DustDPI/releases/download/v1.0.0/DustDPI_Setup.exe)
[![Target](https://img.shields.io/badge/Target-Discord%20%7C%20Roblox-7c3aed?style=for-the-badge&logo=discord&logoColor=white)](https://dust-studio.com)
[![Engine](https://img.shields.io/badge/Engine-WinDivert%20v1.4-a855f7?style=for-the-badge)](https://dust-studio.com)
[![Status](https://img.shields.io/badge/Antigravity%20Compatible-Verified-10b981?style=for-the-badge)](https://dust-studio.com)
[![License](https://img.shields.io/badge/License-MIT%20%2F%20Apache-blue.svg?style=for-the-badge)](https://dust-studio.com)

---

## ⚡ Neden DustDPI?

Standart DPI atlatma araçları çalıştırıldığında sistemdeki **tüm giden 443 (HTTPS) ve 80 (HTTP) trafiğini körü körüne yakalar**, araya sahte (fake) TTL enjekte eder ve paketleri parçalar. 

Bu durum; **Antigravity IDE**, Google Cloud, Claude/Anthropic, iş VPN'leri, Steam ve bankacılık gibi katı TLS kuralları uygulayan modern platformların el sıkışmasını (TLS Handshake) bozarak oturumların sürekli düşmesine veya bağlantı hatalarına (`ECONNRESET`) yol açar.

**DustDPI**, `--blacklist` mimarisini devreye sokarak bu sorunu kökten çözer:

* 🎯 **Yalnızca Hedef Servisler:** Sadece `blacklist.txt` içindeki engelli alan adlarına (Discord, Roblox, Wattpad vb.) desync uygular.
* 🚀 **Sıfır Yan Etki (Pass-Through):** Antigravity IDE, Google servisleri, oyunlar ve bankalar bu listede olmadığı için paketlerine **kılını bile kıpırdatmaz**, %100 doğrudan ve orijinal hızında geçer.
* 🖥️ **Modern Masaüstü Arayüzü (`DustDPI.exe`):** Şık dark mode arayüzü, sistem tepsisi (system tray) desteği ve tek tıkla servis başlatma/durdurma.
* ⚙️ **Windows Servis Mimarisi:** Bilgisayar her açıldığında otomatik olarak arka planda sessizce başlar. Ekranda açık kalan CMD pencerelerine ihtiyaç duymaz.

---

## 📁 Proje Yapısı

```
DustDPI/
├── DustDPI_Setup.exe        # Son kullanıcı tek tıkla Windows Kurulum Sihirbazı
├── DustDPI.exe              # Modern C++ Dark Mode Masaüstü Grafik Arayüzü (GUI)
├── DustDPI_Manager.bat      # Renkli Konsol Yönetim Menüsü
├── blacklist.txt            # Hedef alan adları listesi (İstediğini ekleyebilirsin)
├── service_install.cmd      # DustDPI'ı otomatik başlayan Windows Servisi yapar
├── service_remove.cmd       # Servisi ve WinDivert sürücülerini temizce kaldırır
├── start_console.cmd        # Canlı test / konsol modunda başlatır
├── start_service.cmd        # Arka plan servisini başlatır
├── stop_service.cmd         # Arka plan servisini durdurur
├── status.cmd               # Servis çalışma durumunu sorgular
├── x86_64/                  # 64-bit sürücüler ve dust_engine motoru
└── x86/                     # 32-bit sürücüler ve dust_engine motoru
```

---

## 🚀 Hızlı Kurulum & Kullanım

### 1. Yöntem: Yönetim Menüsü ile (Tavsiye Edilen)
1. `DustDPI_Manager.bat` dosyasına **Sağ Tıklayın** ve **"Yönetici Olarak Çalıştır"** deyin.
2. Açılan menüden **[1]** tuşuna basarak servisi kurun.
3. Artık sistem arka planda hazırdır! Bilgisayarınızı yeniden başlatsanız bile servis otomatik devrede kalır.

### 2. Yöntem: Manuel Kurulum
* **Kurmak İçin:** `service_install.cmd` dosyasına sağ tıklayıp **"Yönetici Olarak Çalıştır"** deyin.
* **Kaldırmak İçin:** `service_remove.cmd` dosyasını yönetici olarak çalıştırın.
* **Test Etmek İçin:** Servis kurmadan denemek isterseniz `start_console.cmd` dosyasını yönetici olarak çalıştırabilirsiniz.

---

## 🌐 Kritik Tavsiye: Güvenli DNS (DoH) Ayarı

Türkiye'deki bazı servis sağlayıcılar (İSS) Discord IP'lerini doğrudan DNS seviyesinde engellemektedir. Tüm sistemin DNS'ini rastgele tünellere zorlamak yerine, Windows veya tarayıcınızda **Şifreli DNS (DNS-over-HTTPS)** açmanız önerilir:

1. **Windows 11 Ayarları:**
   * `Ayarlar` ──► `Ağ ve İnternet` ──► `Wi-Fi veya Ethernet (Bağlantı Özellikleri)`
   * `DNS Sunucusu Ataması` ──► `Düzenle` ──► `El ile (Manual)`
   * IPv4 Tercih Edilen DNS: `1.1.1.1` (Cloudflare) veya `8.8.8.8` (Google)
   * **DNS Şifrelemesi:** `Yalnızca Şifrelenmiş (HTTPS üzerinden DNS)` olarak seçin.
2. **Alternatif (Chrome / Brave / Edge):**
   * Tarayıcı Ayarları ──► `Gizlilik ve Güvenlik` ──► `Güvenli DNS Kullan (DoH)` ──► `Cloudflare (1.1.1.1)` seçin.

Bu sayede DNS sorgularınız şifreli çözülür, DustDPI ise TCP paketlerini sansürden kurtarır. **Antigravity IDE ve tüm iş ortamınız sıfır kesintiyle çalışmaya devam eder.**
