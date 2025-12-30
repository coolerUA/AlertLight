# GitHub Pages Setup Guide for AlertLight Web Installer

This guide follows GitHub's official instructions to set up a project site for the AlertLight web-based firmware installer.

## Repository Information

- **Repository URL**: https://github.com/coolerUA/AlertLight
- **GitHub Pages URL**: https://coolerUA.github.io/AlertLight/
- **Web Installer URL**: https://coolerUA.github.io/AlertLight/install.html

## Prerequisites

✅ Repository must be **public** (required for GitHub Free)
✅ You must have **admin permissions** on the repository
✅ Your email must be **verified** on GitHub

## Step-by-Step Setup

### Step 1: Verify Entry Files

The following entry files are already created in the repository root:

- ✅ `index.html` - Landing page for the project
- ✅ `install.html` - Web firmware installer
- ✅ `manifest.json` - Firmware configuration
- ✅ `.nojekyll` - Disables Jekyll processing

### Step 2: Commit and Push Files

```bash
# Add all web installer files
git add index.html install.html manifest.json .nojekyll .github/workflows/

# Commit the changes
git commit -m "Add web-based firmware installer"

# Push to GitHub
git push origin main
```

### Step 3: Enable GitHub Pages

1. Go to your repository on GitHub: https://github.com/coolerUA/AlertLight

2. Click **Settings** (repository settings, not account settings)

3. In the left sidebar, under "Code and automation", click **Pages**

4. Under **Build and deployment** → **Source**:
   - Select: **"Deploy from a branch"**

5. Under **Branch**:
   - Select branch: **`main`** (or `master` if that's your default branch)
   - Select folder: **`/ (root)`**
   - Click **Save**

6. Wait for deployment (usually 1-3 minutes)

### Step 4: Verify GitHub Pages Deployment

1. Refresh the Pages settings page

2. You should see a message at the top:
   ```
   Your site is live at https://coolerUA.github.io/AlertLight/
   ```

3. Click **Visit site** to see your landing page

4. Test the web installer:
   - Navigate to: https://coolerUA.github.io/AlertLight/install.html
   - You should see the firmware installer page

### Step 5: Create First Release (Build Firmware)

The web installer needs firmware files from a GitHub Release. Create one:

```bash
# Tag the current commit
git tag -a v1.0.0 -m "Release v1.0.0 with web installer"

# Push the tag to GitHub
git push origin v1.0.0
```

This triggers the GitHub Actions workflow that:
- Compiles the firmware
- Extracts bootloader and partitions
- Creates a Release with all .bin files

### Step 6: Wait for GitHub Actions Build

1. Go to the **Actions** tab in your repository

2. Watch the "Build Firmware" workflow

3. Wait for it to complete (~5 minutes)

4. Go to **Releases** tab

5. Verify that v1.0.0 release contains:
   - `bootloader.bin`
   - `partitions.bin`
   - `boot_app0.bin`
   - `AlertLight.ino.bin`

### Step 7: Test the Complete Installation

1. Visit: https://coolerUA.github.io/AlertLight/install.html

2. Connect an ESP32-S3 device via USB

3. Click **"Install AlertLight"** button

4. Chrome will prompt you to select a serial port

5. Select the correct port (usually `/dev/ttyUSB0` or `COM3`)

6. Wait ~2 minutes for firmware to flash

7. Device will restart with AlertLight firmware! 🎉

## Updating for Future Releases

When you create a new release (e.g., v1.0.1), update `manifest.json`:

```json
{
  "name": "AlertLight",
  "version": "1.0.1",
  "builds": [{
    "chipFamily": "ESP32-S3",
    "parts": [
      {
        "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.1/bootloader.bin",
        "offset": 0
      },
      {
        "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.1/partitions.bin",
        "offset": 32768
      },
      {
        "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.1/boot_app0.bin",
        "offset": 57344
      },
      {
        "path": "https://github.com/coolerUA/AlertLight/releases/download/v1.0.1/AlertLight.ino.bin",
        "offset": 65536
      }
    ]
  }]
}
```

Then commit and push the updated manifest:

```bash
git add manifest.json
git commit -m "Update web installer to v1.0.1"
git push origin main

# Create new release tag
git tag -a v1.0.1 -m "Release v1.0.1"
git push origin v1.0.1
```

## Troubleshooting

### Site not appearing after 10+ minutes

**Check these:**
1. ✅ Repository is **public**
2. ✅ Your email is **verified** (GitHub account settings)
3. ✅ Someone with **admin permissions** pushed to the branch
4. ✅ `index.html` exists in the **root** of the branch
5. ✅ Pages is configured to deploy from **main** branch, **root** folder

**Force rebuild:**
- Make a small change to index.html
- Commit and push
- Wait 2-3 minutes

### 404 Error on install.html

**Verify:**
- File is in the root of the repository
- File is committed and pushed to main branch
- GitHub Pages has rebuilt (check Actions tab)

### Web Installer Shows "Failed to download firmware"

**Causes:**
1. Release doesn't exist yet → Create release with `git tag v1.0.0 && git push origin v1.0.0`
2. Release doesn't have .bin files → Wait for GitHub Actions workflow to complete
3. Wrong version in manifest.json → Check URLs match the release tag

**Fix:**
- Go to Releases tab
- Verify v1.0.0 (or your version) exists
- Verify it has 4 .bin files
- Check manifest.json URLs match the release tag

### "Web Serial API not supported"

**Browser Requirements:**
- ✅ Chrome 89+
- ✅ Edge 89+
- ✅ Opera 76+
- ❌ Firefox (not supported)
- ❌ Safari (not supported)

### Serial Port Access Denied

**Fixes:**
- Close Arduino IDE / PlatformIO / Serial Monitor
- Try a different USB port
- On Linux, add user to dialout group:
  ```bash
  sudo usermod -a -G dialout $USER
  # Log out and back in
  ```

## Optional: Custom Domain

If you want to use a custom domain (e.g., `install.alertlight.com`):

1. In Pages settings, under **Custom domain**, enter your domain

2. Add DNS records at your domain provider:
   ```
   Type: CNAME
   Name: install (or @)
   Value: coolerUA.github.io
   ```

3. Wait for DNS propagation (can take up to 24 hours)

4. GitHub will automatically provision an SSL certificate

## File Structure

Your repository should have these files for GitHub Pages:

```
AlertLight/
├── index.html              # Landing page (entry file)
├── install.html            # Web installer UI
├── manifest.json           # Firmware configuration
├── .nojekyll              # Disable Jekyll
├── .github/
│   └── workflows/
│       └── build-firmware.yml  # Auto-build on releases
├── README.md              # Project documentation
└── (all other project files)
```

## Verification Checklist

Before going live, verify:

- [ ] Repository is public
- [ ] index.html exists in root
- [ ] GitHub Pages is enabled (Settings → Pages)
- [ ] Source is set to "main" branch, root folder
- [ ] Site appears at https://coolerUA.github.io/AlertLight/
- [ ] install.html loads correctly
- [ ] At least one release (v1.0.0) exists
- [ ] Release contains all 4 .bin files
- [ ] manifest.json points to correct release URLs
- [ ] Web installer can flash a test device

## Success!

Once setup is complete:

✅ Users can visit: **https://coolerUA.github.io/AlertLight/**
✅ Click "Install Firmware via Browser"
✅ Flash firmware in ~2 minutes with zero setup!

No Arduino IDE, no drivers, no command line required! 🎉

---

**Questions?**
- 📖 [GitHub Pages Documentation](https://docs.github.com/en/pages)
- 🐛 [Report Issues](https://github.com/coolerUA/AlertLight/issues)

Made with ❤️ for Ukraine 🇺🇦
