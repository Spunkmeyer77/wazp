#!/bin/bash
# Packs a Release build into dist/WAZP-<version>-Linux-x86_64.tar.gz (with an install script)
set -euo pipefail
cd "$(dirname "$0")/.."

VERSION=$(sed -nE 's/^project\(WAZP VERSION ([0-9.]+)\).*/\1/p' CMakeLists.txt)
ART="build/WAZP_artefacts/Release"
NAME="WAZP-$VERSION-Linux-$(uname -m)"
STAGE="dist/$NAME"

rm -rf "$STAGE" && mkdir -p "$STAGE"
cp -R "$ART/VST3/WAZP.vst3"   "$STAGE/"
cp -R "$ART/LV2/WAZP.lv2"     "$STAGE/"
cp    "$ART/Standalone/WAZP"  "$STAGE/"

cat > "$STAGE/install.sh" <<'EOS'
#!/bin/bash
# Installs WAZP for the current user (no root needed).
set -e
cd "$(dirname "$0")"
mkdir -p ~/.vst3 ~/.lv2 ~/.local/bin
rm -rf ~/.vst3/"WAZP.vst3" ~/.lv2/"WAZP.lv2"
cp -R "WAZP.vst3" ~/.vst3/
cp -R "WAZP.lv2"  ~/.lv2/
cp "WAZP" ~/.local/bin/WAZP && chmod +x ~/.local/bin/WAZP
echo "Installed: ~/.vst3/WAZP.vst3, ~/.lv2/WAZP.lv2, ~/.local/bin/WAZP"
echo "Rescan plug-ins in your DAW (Bitwig, Reaper, Ardour...)."
EOS

cat > "$STAGE/uninstall.sh" <<'EOS'
#!/bin/bash
rm -rf ~/.vst3/"WAZP.vst3" ~/.lv2/"WAZP.lv2" ~/.local/bin/WAZP
echo "WAZP removed."
EOS
chmod +x "$STAGE/install.sh" "$STAGE/uninstall.sh"

cat > "$STAGE/README.txt" <<EOS
WAZP $VERSION - Monophonic synthesizer - Transistor Labs 2026

Install:    ./install.sh      (VST3 -> ~/.vst3, LV2 -> ~/.lv2, app -> ~/.local/bin/WAZP)
Uninstall:  ./uninstall.sh
Needs: ALSA or JACK, X11, FreeType (standard on desktop distributions).
EOS

tar -C dist -czf "dist/$NAME.tar.gz" "$NAME"
rm -rf "$STAGE"
echo "==> Done: dist/$NAME.tar.gz"
