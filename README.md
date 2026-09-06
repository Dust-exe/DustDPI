# DustDPI

[![Official Website](https://img.shields.io/badge/Official%20Site-dust--studio.com%2Fdustdpi-7c3aed?style=for-the-badge)](https://dust-studio.com/dustdpi)
[![Author](https://img.shields.io/badge/Author-dust.exe-blueviolet?style=for-the-badge)](https://dust-studio.com)
[![Discord Community](https://img.shields.io/badge/Discord-Dust%20Studio-5865F2?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/E8K6rczdb2)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%2F%2011-0078D6?style=for-the-badge&logo=windows)](https://github.com/Dust-exe/DustDPI/releases)

**Next-Generation Internet Freedom & Selective Traffic Optimization Engine**

DustDPI is an open-source, driver-level network optimization service engineered to deliver unrestricted, ultra-low latency web browsing and application connectivity. Official landing page & link profile: [https://dust-studio.com/dustdpi](https://dust-studio.com/dustdpi). By operating selectively at the packet level, DustDPI eliminates routing interference and packet inspection delays without the bandwidth degradation or latency penalties associated with traditional VPNs.

---

## Disclaimer / Sorumluluk Reddi

This software is developed strictly for educational, research, and network analysis purposes. 
The developers assume no liability and are not responsible for any misuse, damage, 
or violation of local laws, telecommunication regulations, or ISP terms of service 
caused by using this tool. Users are solely responsible for complying with all applicable laws 
in their jurisdiction.

---

## Key Features

- **Selective Packet Routing**: Only traffic destined for hostnames listed in your target filter is processed. General browsing, high-speed gaming, streaming, and local network traffic bypass the engine entirely at native line speed.
- **Interactive Target Filter Manager**: Add, remove, and manage custom domains and web services directly through the native GUI dashboard.
- **Zero Latency Penalty**: Operates locally on your system using driver-level packet header transformation. No remote tunnel servers, no third-party proxies, and zero routing lag.
- **DNS Redirection & Resiliency**: Built-in local DNS port redirection (Port 1253) safeguards against DNS poisoning and resolution failures.
- **Background Windows Service**: Runs autonomously as a native Windows service (`DustDPI`), starting automatically with your system with minimal resource consumption (<10 MB RAM).
- **Modern Cyber Dashboard**: Clean dark-mode Win32 interface with real-time status monitoring, system tray minimization, and one-click network diagnostics.

---

## How It Works

DustDPI intercepts outbound TCP/UDP handshakes for specified target hostnames and applies non-standard packet fragmentation and header sequencing. Intermediate inspection equipment cannot accurately match the signatures of restricted or throttled connections, enabling direct, unhindered peer-to-peer data flow to destination servers.

```
+-------------------+       +-----------------------+       +-------------------+
|  Web Browser / App| ----> |  DustDPI Driver Hook  | ----> |  Global Internet  |
+-------------------+       +-----------------------+       +-------------------+
                                        |
                             [ Is Domain in Filter? ]
                              /                  \
                            YES                   NO
                            /                      \
             [Apply Packet Shaping]          [Direct Pass-Through]
```

---

## Installation & Quick Start

### Option 1: Automated Installer (Recommended)

1. Download **`DustDPI_Setup.exe`** from the latest Release.
2. Run the installer as Administrator.
3. The installer automatically registers the kernel driver, installs the `DustDPI` service, creates desktop shortcuts, and launches the dashboard.

### Option 2: Portable Archive

1. Download and extract **`DustDPI.zip`**.
2. Right-click **`DustDPI.exe`** and select **Run as administrator**.
3. Use the interface to start the service or customize your domain filters.

---

## Managing Your Filter Targets

DustDPI gives you complete control over which websites and applications are routed through the optimization engine:

1. Open the **DustDPI Dashboard**.
2. Click **Manage Target Filter...**
3. Type the domain you wish to add (e.g., `example.com`) and click **+ Add Domain**.
4. To remove an entry, select it from the list and click **- Remove Selected**.
5. Click **Save & Apply Filter Changes**. The service will automatically reload and begin routing your updated targets immediately.

You can also directly edit `blacklist.txt` in any text editor. Ensure each domain is on a new line without protocol prefixes (`http://` or `https://`).

---

## Operating Modes

- **Selective Mode (Recommended)**: Only applies packet optimization to domains specified in `blacklist.txt`. Preserves full native network throughput for all other traffic.
- **Full Optimization Mode**: Applies packet optimization across all outbound HTTPS/TLS connections globally. Accessible via the *Mode: Selective / Full* toggle in the dashboard.

---

## Architecture & Project Structure

```
DustDPI/
├── DustDPI.exe                       # Native Win32 control dashboard
├── DustDPI_Setup.exe                 # Solid NSIS installer package
├── app.ico                           # Multi-resolution icon asset
├── blacklist.txt                     # Target domain filter list
├── network_diagnostics_and_repair.cmd# Quick diagnostic and socket reset utility
├── service_install.cmd               # Manual service registration script
├── service_remove.cmd                # Complete service & driver uninstaller
├── src/
│   ├── DustDPI_GUI.cpp               # GUI source code
│   ├── resource.rc                   # Windows resource definition
│   └── resource.res                  # Compiled binary resource
├── x86_64/
│   ├── dust_engine.exe               # 64-bit core packet processing engine
│   ├── WinDivert.dll                 # Packet capture interface library
│   └── WinDivert64.sys               # Signed kernel-mode filtering driver
└── x86/
    ├── dust_engine.exe               # 32-bit core packet processing engine
    ├── WinDivert.dll                 # 32-bit interface library
    ├── WinDivert32.sys               # 32-bit driver
    └── WinDivert64.sys               # 64-bit driver for WOW64
```

---

## Building from Source

### Prerequisites

- GCC / MinGW-w64 (`x86_64-w64-mingw32-g++` and `x86_64-w64-mingw32-windres`)
- NSIS (`makensis`) for building the setup installer

### Compilation

```bash
# 1. Compile Windows Resource
x86_64-w64-mingw32-windres src/resource.rc -O coff -o src/resource.res

# 2. Compile Dashboard Executable
x86_64-w64-mingw32-g++ -mwindows -municode -O2 -static \
    -o DustDPI.exe src/DustDPI_GUI.cpp src/resource.res \
    -lcomctl32 -ldwmapi

# 3. Compile Setup Installer
makensis installer.nsi
```

---

## Legal Notice & Disclaimer

- **Educational & Diagnostic Purpose**: DustDPI is published strictly as a network research, traffic diagnostic, and latency optimization utility.
- **Compliance with Laws**: Users are solely responsible for ensuring compliance with their local, national, and international laws, telecommunications regulations, and service provider terms. Dust Studio assumes no liability for user configuration or misuse.
- **Trademarks**: All product names, logos, and brands are property of their respective owners. All company, product, and service names used in this document and software are for identification purposes only.
- **Kernel-Level Liability**: DustDPI modifies raw network packet flags and TCP/IP handshakes at the driver level. Dust Studio shall not be held liable for any network downtime, system instability, packet loss, or regulatory penalties incurred by the user. End users bear full responsibility for actions performed using this software.
- Detailed terms: See [DISCLAIMER.md](DISCLAIMER.md) and [LICENSE](LICENSE).

---

## License & Third-Party Attributions
 
- **DustDPI**: Distributed under the [Apache License 2.0](LICENSE). Copyright (C) 2026 Dust Studio.
- **WinDivert**: WinDivert kernel driver and user-mode library are licensed under GNU LGPLv3. Source code and license details can be obtained from the [WinDivert Official Repository](https://github.com/basil00/WinDivert). For complete licensing texts and attributions, see [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
