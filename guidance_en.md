# OBS Auto SF Fit — User Guide

This guide explains how to set up, configure, and use the **Auto SF Fit** plugin in OBS Studio. This filter automatically handles scaling, positioning, and animating a live/SDI video source to fit inside a transparent Squeeze Frame (SF) overlay.

---

## 1. Quick Start Setup
To configure a Squeeze Frame scene:
1. **Add your Sources**:
   * Add your live video source (e.g. SDI, capture card, media file) to your scene.
   * Add your Squeeze Frame (SF) overlay source (e.g. `.webm`/`.mov` video, static image, browser source).
2. **Apply the Filter**:
   * Right-click your **live video source** in the sources list and select **Filters**.
   * Click the `+` button in **Effect Filters** and add **Auto SF Fit**.
3. **Configure Settings**:
   * Select your Squeeze Frame source in the **SF Source** dropdown.
   * Check **Enable Timing (In/Hold/Out)** if you are using a transition.
   * Customize transition timings and fit modes as needed (explained below).

---

## 2. Setting Descriptions & Functions

### SF Source
* **What it does**: Selects the source that contains the transparent cutout.
* **Why it matters**: The plugin scans this source's alpha channel (transparency) to determine where the live video should be placed.

### Fit Mode
Controls how the live video is resized to fit the detected cutout area:
* **Contain**: Scales the video down so that it fits entirely inside the cutout. The aspect ratio is preserved (no distortion). If aspect ratios don't match, you will see thin margins (letterboxing/pillarboxing) inside the cutout.
* **Cover**: Scales the video up until it completely fills the cutout. Aspect ratio is preserved. If aspect ratios don't match, the outer edges of the video are cropped out to ensure **zero gaps**.
* **Stretch**: Stretches the width and height of the video independently to match the cutout dimensions exactly. The video will be stretched or squashed (aspect ratio distorted), but there is **no cropping** and **no gaps**.

### Alpha Threshold
* **Range**: `0` to `255` (default `16`).
* **What it does**: Sets the sensitivity for transparency detection. Any pixel with opacity below this threshold is counted as "transparent" and part of the cutout area.
* **Tip**: Increase this value slightly if compression noise in video files prevents a clean cutout scan.

### Padding
* **Range**: `0` to `200` pixels (default `0`).
* **What it does**: Adds a solid safety margin inside the detected cutout.
* **Tip**: Use `1` or `2` pixels of padding to overlap the live video slightly behind the border graphics to hide pixel-rounding lines.

### Recalculation Mode
Controls when the plugin scans the Squeeze Frame to calibrate the cutout:
* **Manual Refresh**: Performs the scan only when you click the **Recalculate Now** button. Best for static overlays.
* **On Activate**: Scans the cutout once when the scene becomes active or is switched to. Highly recommended for videos.
* **Periodic**: Repeatedly scans the cutout at a regular interval.

### Recalc Interval (ms)
* **What it does**: The frequency (in milliseconds) at which the plugin performs the scan when *Periodic* mode is active.

### Enable Timing (In/Hold/Out)
* **What it does**: Synchronizes the squeeze transition (in, hold, out) of the live source and the Squeeze Frame.
* **How it syncs**: 
  * If the SF source is a video file, it reads the video's decoded playback time (**Media-Sync**) for frame-perfect alignment.
  * If the SF source is static (an image, browser source, etc.), it falls back to the system clock.

### Transition Durations
* **In Duration (ms)**: The time it takes for the live video to squeeze down and the SF overlay to slide/scale in (default `1000ms`).
* **Hold Duration (ms)**: The time the video remains squeezed in the cutout (default `8000ms`).
* **Out Duration (ms)**: The time it takes for the live video to expand back to full screen and the SF to exit (default `1000ms`).

### SF Animation Mode
Controls how the Squeeze Frame overlay animates during the transition:
* **Static (No Animation)**: The Squeeze Frame is rendered at a constant 1:1 scale and 0 offset. It does not move. This is ideal if your Squeeze Frame video already has a transition animation built into the video file itself.
* **Slide Only**: The Squeeze Frame remains at a constant 1:1 scale (preventing any aspect ratio distortion) and slides on/off-screen in the direction specified by the **Slide Direction** setting.
* **Scale & Slide (Lockstep)**: The Squeeze Frame and the live video scale and shift in lockstep to guarantee zero edge gaps. Note that this mode will scale the Squeeze Frame graphics slightly.

### Slide Direction
* **Options**: Left, Right, Top, Bottom.
* **What it does**: Specifies the direction from which the Squeeze Frame slides onto the screen. Only active when *SF Animation Mode* is set to *Slide Only*.

### Show Debug Bounding Box
* **What it does**: Draws a green border around the detected cutout area.
* **Usage**: Turn this on temporarily during setup to verify that the cutout is scanned correctly, then turn it off for production.

### Overlay SF Source
* **What it does**: Draws the Squeeze Frame graphics directly on top of the live video.
* **Usage**: Leave this checked. It ensures that the frame overlays the video correctly and applies opacity animations.

### Recalculate Now
* **What it does**: Forces an immediate scan of the Squeeze Frame cutout. Useful for manual calibration.

---

## 3. Best Practices & Advanced Concepts

### Persistent Bounding Box Cache
The plugin automatically caches the bounding box coordinates inside the OBS scene collection. 
* This prevents visual jumping or "black frames" when switching scenes, since the plugin loads the correct coordinates immediately before the first frame renders.
* To clear the cache and force a fresh scan, click **Recalculate Now** or change the **SF Source** dropdown.

### Group Scaling (Option 2)
The plugin performs the transition by scaling the Squeeze Frame and the live video in lockstep.
* The Squeeze Frame scales down from a larger start size to $1.0$, while the live video squeezes from full-screen into the cutout.
* Because they scale and shift in lockstep, **no edge gaps or visual margins** can form between the video and the frame during the transition.

### Video Source Settings
For the smoothest results with `.mov`/`.webm` Squeeze Frame videos:
* Ensure **Loop** is **unchecked** in the Squeeze Frame's OBS source properties. The transition is designed to run once and end.
* Ensure **Restart playback when source becomes active** is **checked** in the Squeeze Frame's properties. This triggers the transition automatically whenever you switch scenes.
