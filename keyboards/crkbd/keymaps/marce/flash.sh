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

echo "==> Waiting for the RPI-RP2 drive (up to ${WAIT_TIMEOUT}s) -- put the keyboard in bootloader mode now"
echo "    (hold Q while plugging in the left/master half, or P for the right half)."

max_ticks=$((WAIT_TIMEOUT * 5))
ticks=0
device=""
until [ -n "$device" ]; do
    device=$(lsblk -rno NAME,LABEL | awk '$2 == "RPI-RP2" { print "/dev/" $1; exit }')
    [ -n "$device" ] && break
    if [ "$ticks" -ge "$max_ticks" ]; then
        echo "Error: timed out waiting for the RPI-RP2 drive to appear." >&2
        exit 1
    fi
    sleep 0.2
    ticks=$((ticks + 1))
    printf '.'
done
echo
echo "==> Drive detected at ${device}"

# This system doesn't auto-mount the drive, so mount it ourselves. If a
# previous run already mounted it (and it wasn't ejected), reuse that.
existing_mount=$(lsblk -no MOUNTPOINT "$device")
if [ -n "$existing_mount" ]; then
    mount_path="$existing_mount"
else
    udisks_output=$(udisksctl mount -b "$device")
    mount_path=$(printf '%s' "$udisks_output" | sed -n 's/.* at //p')
    if [ -z "$mount_path" ]; then
        echo "Error: could not determine mount path from: $udisks_output" >&2
        exit 1
    fi
fi

echo "==> Mounted at ${mount_path}, flashing..."
cp "$UF2_FILE" "$mount_path/"
echo "==> Done -- the board will flash itself and reboot automatically."
