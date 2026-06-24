# Development Agents

This project, **Project M.I.D.A.S.** (Media Insertion & Dynamic Auto-scaling System), was co-developed by the USER and **Antigravity**, an agentic AI coding assistant designed by the Google DeepMind team.

## Our Partnership
Throughout the development cycle, the USER acted as the lead architect and product manager, defining requirements and verifying behavior in OBS, while the Antigravity agent handled:
- Native C++ logic implementation.
- HLSL Shader programming for image scaling and overlay compositing.
- Automated packaging and deployment scripts.
- CI/CD integration.

## Key Milestones Achieved
1. **Dynamic Scaling Engine**: Co-designed the lockstep group scaling math that anchors the parent source to the Squeeze Frame cutout.
2. **Media-Synced Transitions**: Bound transitions directly to OBS Media Source frame times, eliminating clock drift.
3. **Robust Cutout Scanner**: Developed the column/row projection profile scanner to ignore video edge transparency noise.
4. **Inno Setup Installer**: Automated the delivery of built DLLs and localized assets to user desktops.
