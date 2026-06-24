# Changelog

All notable changes to **Project M.I.D.A.S.** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.1] - 2026-06-18
### Added
- Configurable **Squeeze Frame Animation Modes** in settings UI:
  - **Static (No Animation)**: Perfect for videos with baked-in animations.
  - **Slide**: Slides the Squeeze Frame in a specified direction.
  - **Scale & Slide (Lockstep)**: Scales and slides both the Squeeze Frame and parent source in lockstep.
- Customizable **Slide Direction** (Left, Right, Top, Bottom) for Slide mode.
- Local PowerShell packaging and deployment script (`package.ps1`).
- Inno Setup installer compilation support in GitHub Actions.

### Changed
- **Robust Cutout Scanner**: Rewrote the transparency bounding box scanner in `alpha-analyzer.cpp` to use a column/row projection profile algorithm. This prevents edge transparent margins (like the 3px border in the Soccer Edition video) from incorrectly expanding the cutout box.
- Updated user documentation guides with new animation settings.

### Fixed
- Fixed Pascal compiler declaration order bug (`Unknown identifier 'IsModuleLoaded'`) in `installer.iss`.

---

## [1.0.0] - 2026-06-17
### Added
- Initial Release of the **Auto SF Fit** OBS filter plugin.
- **Unified Group Transform**: Locks the parent source scale and translation to the Squeeze Frame overlay.
- **Media-Synced Timing**: Transition timelines are driven frame-perfectly by media player timestamps.
- **Bounding Box Caching**: Stores detected cutouts in OBS settings to prevent visual jumps.
- **Visual Debugging**: Renders a green bounding box around the detected cutout.
- **Inno Setup Installer Script**: Bundles DLL and localizations into an elevated `.exe` installation wizard.
