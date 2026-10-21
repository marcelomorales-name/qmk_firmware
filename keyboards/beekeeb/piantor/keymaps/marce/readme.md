# Piantor (marce)

A port of the [Corne `marce` keymap](../../../../crkbd/keymaps/marce/) onto the beekeeb Piantor
(`LAYOUT_split_3x6_3`, 42 keys). Same layers, tap dance, digitizer point-jump keys, numeric grid with F/G overlay and
Mac/PC AltGr switch; shared logic lives in `users/marce/`. See `CLAUDE.md` for what moved to fit without the Corne's
ex2 keys.

    qmk compile -kb beekeeb/piantor -km marce
    ./flash.sh            # compile in the toolbox and copy the .uf2 to the RPI-RP2 drive
