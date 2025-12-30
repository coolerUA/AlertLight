# Web Installer Setup Guide

This guide explains how to set up the web-based firmware installer for AlertLight using GitHub Pages.

## Overview

The web installer uses **ESP Web Tools** and Chrome's **Web Serial API** to flash firmware directly from the browser without requiring Arduino IDE or other software installation.

## Architecture

```
GitHub Release (created by CI/CD)
    ├── firmware/
    │   ├── bootloader.bin
    │   ├── partitions.bin
    │   ├── boot_app0.bin
    │   └── AlertLight.ino.bin
    ├── manifest.json (describes firmware parts)
    └── install.html (web flasher UI)
           ↓
    GitHub Pages hosts install.html
           ↓
    User visits https://coolerUA.github.io/AlertLight/install.html
           ↓
    ESP Web Tools loads firmware from manifest.json
           ↓
    User flashes device via Web Serial API
```

## Setup Steps

### 1. Enable GitHub Pages

1. Go to your repository on GitHub
2. Click **Settings** → **Pages**
3. Under **Source**, select:
   - Branch: `main` (or `master`)
   - Folder: `/ (root)`
4. Click **Save**
5. GitHub will provide a URL like: `https://coolerUA.github.io/AlertLight/`

### 2. Configure DNS (Optional - Custom Domain)

If you want to use a custom domain:

1. In **Settings** → **Pages** → **Custom domain**
2. Enter your domain (e.g., `install.alertlight.com`)
3. Add DNS records at your domain provider:
   ```
   Type: CNAME
   Name: install (or @)
   Value: coolerUA.github.io
   ```

### 3. Create a Release to Build Firmware

The GitHub Actions workflow automatically builds firmware when you create a release:

```bash
# Tag the current commit
git tag -a v1.0.0 -m "Release version 1.0.0"

# Push the tag to GitHub
git push origin v1.0.0
```

This triggers the `build-firmware.yml` workflow which:
1. Compiles the firmware for ESP32-S3
2. Extracts bootloader and partition files
3. Creates a GitHub Release with all .bin files
4. Uploads `manifest.json` and `install.html`

### 4. Update Manifest for GitHub Pages

The `manifest.json` needs to point to the firmware files from the GitHub Release:

```json
{
  "name": "AlertLight",
  "version": "1.0.0",
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        {
          "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.0/bootloader.bin",
          "offset": 0
        },
        {
          "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.0/partitions.bin",
          "offset": 32768
        },
        {
          "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.0/boot_app0.bin",
          "offset": 57344
        },
        {
          "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.0/AlertLight.ino.bin",
          "offset": 65536
        }
      ]
    }
  ]
}
```

**Note**: Update the version number in URLs when creating new releases.

### 5. Test the Installer

1. Visit: `https://coolerUA.github.io/AlertLight/install.html`
2. Connect an ESP32-S3 device via USB
3. Click "Install AlertLight"
4. Chrome will prompt to select a serial port
5. Firmware will be flashed automatically

## Alternative: Self-Hosted Option

If you prefer to host the installer on your own server:

1. Build firmware locally or download from GitHub Releases
2. Upload all files to your web server:
   ```
   your-domain.com/
   ├── install.html
   ├── manifest.json
   └── firmware/
       ├── bootloader.bin
       ├── partitions.bin
       ├── boot_app0.bin
       └── AlertLight.ino.bin
   ```

3. Update `manifest.json` paths to be relative:
   ```json
   "path": "firmware/bootloader.bin"
   ```

4. Visit `https://your-domain.com/install.html`

## Troubleshooting

### "Web Serial API not supported"
- Use Chrome 89+, Edge 89+, or Opera 76+
- Web Serial is not available in Firefox or Safari

### "Failed to open serial port"
- Close Arduino IDE, PlatformIO, or other serial monitors
- Check USB cable connection
- Try a different USB port
- On Linux, ensure user is in `dialout` group:
  ```bash
  sudo usermod -a -G dialout $USER
  ```

### "Failed to download firmware"
- Check internet connection
- Verify manifest.json URLs are correct
- Ensure GitHub Release files are public

### Flash verification failed
- Try erasing flash first (checkbox in installer)
- Check power supply (use USB 2.0 ports, not USB 3.0)
- Verify device is ESP32-S3 (not ESP32 or ESP32-C3)

## Files Reference

### `manifest.json`
Describes firmware parts and their flash offsets. Used by ESP Web Tools.

### `install.html`
Web UI for the installer. Loads ESP Web Tools library and provides user interface.

### `.github/workflows/build-firmware.yml`
GitHub Actions workflow that automatically builds firmware on release tags.

### Firmware Files
- **bootloader.bin**: ESP32-S3 bootloader (offset 0x0000)
- **partitions.bin**: Partition table (offset 0x8000)
- **boot_app0.bin**: Boot configuration (offset 0xe000)
- **AlertLight.ino.bin**: Main application (offset 0x10000)

## Security Notes

- Web Serial API requires HTTPS (GitHub Pages provides this)
- Users must explicitly grant serial port access
- Firmware is downloaded from GitHub (CDN cached, secure)
- All communication is encrypted over HTTPS

## Resources

- [ESP Web Tools Documentation](https://esphome.github.io/esp-web-tools/)
- [Web Serial API Specification](https://wicg.github.io/serial/)
- [GitHub Pages Documentation](https://docs.github.com/en/pages)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)

---

Made with ❤️ for Ukraine 🇺🇦
