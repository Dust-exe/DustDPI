# Third-Party Notices and Licenses

DustDPI incorporates or dynamically links to the following third-party software components:

---

## 1. WinDivert (Windows Packet Divert)

- **Author / Copyright**: Copyright (c) 2018 basil00
- **Source Code Repository**: https://github.com/basil00/Divert
- **License**: GNU Lesser General Public License (LGPL) version 3.0 / GNU General Public License (GPL) version 3.0

DustDPI dynamically links to `WinDivert.dll` via the standard Windows `LoadLibrary` API and does not statically incorporate or modify WinDivert driver source code. Users remain completely free to replace `WinDivert.dll`, `WinDivert32.sys`, or `WinDivert64.sys` with custom or newly compiled versions under the terms of the GNU LGPL v3.

### GNU Lesser General Public License (LGPL) v3 Notice:

```
WinDivert is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

WinDivert is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with WinDivert. If not, see <http://www.gnu.org/licenses/>.
```

---

## 2. Windows SDK & MinGW-w64 Runtime

- Component: C/C++ Runtime Libraries & Windows API Headers
- Usage: Standard system API linkages (`kernel32`, `user32`, `gdi32`, `comctl32`, `dwmapi`, `shell32`, `advapi32`).
- Terms: Subject to Microsoft Windows SDK and MinGW-w64 runtime licensing terms.
