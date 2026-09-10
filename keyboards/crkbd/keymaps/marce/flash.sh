#!/usr/bin/env bash
# Compile and flash this keymap.
#
# Compiling needs the ARM toolchain, which only lives in the archlinux-latest
# toolbox. Flashing is simpler than Lily58's AVR board: this is RP2040, so
# putting it in bootloader mode makes it mount as a plain USB mass-storage
# drive (RPI-RP2) and "flashing" is just copying the .uf2 onto it -- no
# avrdude, no serial device permissions. It still has to happen on the HOST
# though: the toolbox has its own private /run/media (an empty tmpfs, not a
# bind mount of the host's), so it can never see the drive appear.
set -euo pipefail

KEYBOARD="crkbd/rev4_1/standard"
KEYMAP="marce"
REPO_ROOT="/var/home/marcelo/ghq/github.com/marcelomorales-name/qmk_firmware"
UF2_FILE="${REPO_ROOT}/crkbd_rev4_1_standard_marce.uf2"
TOOLBOX_CONTAINER="archlinux-latest"
WAIT_TIMEOUT=120 # seconds to wait for the drive to appear

skip_compile=0
for arg in "$@"; do
    case "$arg" in
        --flash-only) skip_compile=1 ;;
        *)
            echo "Usage: $0 [--flash-only]" >&2
            exit 1
            ;;
    esac
done

if [ "$skip_compile" -eq 0 ]; then
    echo "==> Compiling ${KEYBOARD}:${KEYMAP} in the ${TOOLBOX_CONTAINER} toolbox..."
    toolbox run -c "$TOOLBOX_CONTAINER" bash -c "
        cd '${REPO_ROOT}' &&
        qmk compile -kb '${KEYBOARD}' -km '${KEYMAP}'
    "
fi

if [ ! -f "$UF2_FILE" ]; then
    echo "Error: $UF2_FILE not found after compile." >&2
    exit 1
fi

echo "==> Waiting for the RPI-RP2 drive (up to ${WAIT_TIMEOUT}s) -- reset the keyboard into the bootloader now"
echo "    (hold LOWER+RAISE and tap QK_BOOT, or double-tap the reset button / hold BOOTSEL while plugging in)."

max_ticks=$((WAIT_TIMEOUT * 5))
ticks=0
mount_point=""
until [ -n "$mount_point" ]; do
    for candidate in /run/media/*/RPI-RP2 /media/*/RPI-RP2 /media/RPI-RP2; do
        if [ -d "$candidate" ]; then
            mount_point="$candidate"
            break
        fi
    done
    [ -n "$mount_point" ] && break
    if [ "$ticks" -ge "$max_ticks" ]; then
        echo "Error: timed out waiting for the RPI-RP2 drive to appear." >&2
        exit 1
    fi
    sleep 0.2
    ticks=$((ticks + 1))
    printf '.'
done
echo
echo "==> Drive detected at ${mount_point}, flashing..."

cp "$UF2_FILE" "$mount_point/"
echo "==> Done -- the board will flash itself and reboot automatically."
