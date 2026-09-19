#!/usr/bin/env bash
# Compile and flash this keymap.
#
# Compiling needs avr-gcc/avr-libc, which only live in the archlinux-latest
# toolbox (with /usr/avr/{include,lib} symlinked to avr-libc's actual
# /usr/lib/avr/{include,lib} -- see CLAUDE.md history). Flashing must happen
# on the HOST: the toolbox's rootless user namespace can't see the real
# owner/group of /dev/ttyACM0 (it shows up as nobody:nobody), so avrdude
# inside the toolbox can never write to it.
set -euo pipefail

KEYBOARD="lily58/rev1"
KEYMAP="marce"
REPO_ROOT="/var/home/marcelo/ghq/github.com/marcelomorales-name/qmk_firmware"
HEX_FILE="${REPO_ROOT}/lily58_rev1_marce.hex"
DEVICE="/dev/ttyACM0"
TOOLBOX_CONTAINER="archlinux-latest"
WAIT_TIMEOUT=120 # seconds to wait for the device to appear

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
        qmk compile -kb '${KEYBOARD}' -km '${KEYMAP}' -e EXTRAFLAGS='-Wno-error=unused-but-set-variable'
    "
fi

if [ ! -f "$HEX_FILE" ]; then
    echo "Error: $HEX_FILE not found after compile." >&2
    exit 1
fi

echo "==> Waiting for ${DEVICE} (up to ${WAIT_TIMEOUT}s) -- reset the keyboard into the bootloader now"
echo "    (hold LOWER+RAISE and tap QK_BOOT, or double-tap the reset button)."

max_ticks=$((WAIT_TIMEOUT * 5))
ticks=0
until [ -e "$DEVICE" ]; do
    if [ "$ticks" -ge "$max_ticks" ]; then
        echo "Error: timed out waiting for ${DEVICE} to appear." >&2
        exit 1
    fi
    sleep 0.2
    ticks=$((ticks + 1))
    printf '.'
done
echo
echo "==> Device detected, flashing..."

sg dialout -c "avrdude -c avr109 -p atmega32u4 -P '${DEVICE}' -b 57600 -D -U flash:w:'${HEX_FILE}':i"
