# OBS Auto SF Fit (Ads Insertion Plugin)

A native C++ filter plugin for OBS Studio on Windows that automatically fits a live video or SDI source into a transparent cutout (Squeeze Frame / SF overlay) to deliver frame-perfect picture-in-picture broadcast sequences.

---

## Project M.I.D.A.S.
* **Acronym**: Media Insertion & Dynamic Auto-scaling System.
* **Philosophy**: King Midas is known for his ability to turn everything he touches into gold. This name is perfect for the context of "Ads Insertion" or advertisements, which are essentially money-making (gold-making) engines for broadcasting. Additionally, the term "Auto-scaling" represents its core feature of automatically resizing the Squeeze Frame.

---

## 🚀 Key Features

* **Alpha-Based Cutout Scan**: Automatically scans the Squeeze Frame source's alpha transparency channel on the fly to detect the target placement box.
* **Unified Group Scaling**: Scales and offsets both the Squeeze Frame graphics and live source in lockstep. This guarantees **zero gaps, margins, or black borders** during transitions.
* **Media-Synced Timing**: Synchronizes the transition phases (In, Hold, Out) frame-by-frame with the media player's decoded timeline for videos, with a system-clock fallback for static images/web overlays.
* **Three Fit Modes**:
  * **Contain**: Fits the video entirely within the cutout while maintaining aspect ratio (adds letterboxing/pillarboxing if needed).
  * **Cover**: Fills the cutout completely, cropping overflow to ensure no border gaps.
  * **Stretch**: Stretches the source to match the cutout dimensions exactly (distorts aspect ratio, but no crop and no gaps).
* **Smart Bounding Box Caching**: Stores the bounding box coordinates inside the OBS scene collection. This prevents visual jumps or black frame flickers when switching active scenes.
* **Visual Debugging**: Toggles a green outline around the detected cutout to simplify initial configuration.

---

## 🛠️ Build & Installation (Windows)

This plugin is built using **CMake** and **Visual Studio 2022**.

### 1. Build the Plugin
Open your terminal in the repository root and run:
```powershell
# Configure and build the project in RelWithDebInfo configuration
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build_x64 --config RelWithDebInfo
```

### 2. Copy to OBS Studio Installation
Run an elevated PowerShell window to deploy the built assets:
```powershell
# Create the locale directory
New-Item -ItemType Directory -Force -Path "C:\Program Files\obs-studio\data\obs-plugins\obs-auto-sf-fit\locale"

# Copy DLL binary
Copy-Item -Path ".\build_x64\RelWithDebInfo\obs-auto-sf-fit.dll" -Destination "C:\Program Files\obs-studio\obs-plugins\64bit\obs-auto-sf-fit.dll" -Force

# Copy English Locale
Copy-Item -Path ".\data\locale\en-US.ini" -Destination "C:\Program Files\obs-studio\data\obs-plugins\obs-auto-sf-fit\locale\en-US.ini" -Force
```

---

## 📖 Detailed User Manuals

For quick setup, description of all UI settings, and broadcast-grade configuration tips, refer to our detailed user guides:

* 🇬🇧 [**English User Guide** (guidance_en.md)](guidance_en.md)
* 🇮🇩 [**Panduan Pengguna Bahasa Indonesia** (guidance_id.md)](guidance_id.md)

---

## ⚙️ Filter Settings Overview

| Setting | Type | Description |
|---|---|---|
| **SF Source** | Dropdown | The reference source (video/image) containing the transparent cutout. |
| **Fit Mode** | List | How the live source fits the cutout: *Contain*, *Cover*, or *Stretch*. |
| **Alpha Threshold** | Slider (0-255) | Sensitivity threshold for transparency scanning. |
| **Padding** | Slider (0-200px) | Safety border overlap padding (prevents rounding lines). |
| **Recalculation Mode** | List | Scans for cutout: *Manual*, *On Activate* (recommended), or *Periodic*. |
| **Enable Timing** | Checkbox | Toggles transition timing sync. |
| **In/Hold/Out Durations** | Inputs (ms) | Timings for scale-in, hold-state, and scale-out. |
| **Overlay SF Source** | Checkbox | Renders the overlay graphics on top of the live video. |
| **Show Debug Bounding Box** | Checkbox | Renders a green visual frame around the detected cutout. |
| **Recalculate Now** | Button | Forces a manual scan of the Squeeze Frame's transparent area. |

---

## ⚖️ License

Distributed under the GNU General Public License v2.0 or later. See `LICENSE` for details.
