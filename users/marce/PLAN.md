# users/marce — shared-userspace migration plan

Goal: keep `lily58/keymaps/marce` and `crkbd/keymaps/marce` in sync by moving all
layout-independent logic into `users/marce/`. Layout arrays (`keymaps[]`), board
specific keycodes, OLED and RGB stay in each keymap.

**Resuming:** read "Progress log" at the bottom, find the first unchecked box,
continue from there. Tick boxes and add a log line as you go. One commit per phase.

Paths (repo root = qmk_firmware):
- LILY = `keyboards/lily58/keymaps/marce`   (source of truth; newer)
- CRKBD = `keyboards/crkbd/keymaps/marce`   (older port; has drifted)
- Build (host, no toolbox needed for these): `qmk compile -kb lily58/rev1 -km marce`, `qmk compile -kb crkbd/rev4_1/standard -km marce`

## Design rules
- Lily58 is the source of truth for shared logic. crkbd gains its improvements.
- Shared code may only use features both boards enable, otherwise gate with `#ifdef`.
- Board-only behavior goes through weak hooks (`*_marce_kb` style) or `#ifdef`s, not copy-paste.
- Custom keycodes that exist on only one board (`NUM_LAYER`, `NUM_MO`, `NUM_OS`,
  `CW_CTL`, `NUMGRID_*`) must not shift the shared keycode values silently: shared
  enum first (starting at `SAFE_RANGE`), board-specific enum appended after it.
- Lily58 is AVR (28672 B limit) so it is the size constraint; crkbd is RP2040 (no practical flash limit). Measure both before and after every phase.

## Phase 0 — Baseline (no code changes)
- [x] Confirm crkbd target and MCU: `crkbd/rev4_1/standard`, RP2040 (from flash.sh). Lily58: `lily58/rev1`, AVR.
- [x] Build both boards cleanly on `feature/marce`; sizes in log.
- [x] Record which crkbd-only features exist: `CW_CTL`, `NUM_MO`/`NUM_OS`, repeat keys,
      RGB matrix (`update_rgb_matrix`), 32-bit AltGr table with full xkb set.
- [x] DECIDED: crkbd adopts Lily58's trimmed 16-bit AltGr table (direct chords + 5 dead keys).
      Full xkb table is dropped, not kept behind a flag.
- [x] DECIDED: crkbd adopts the `hold_key` helpers AND the `NUMGRID_*` F/G overlay.
      Consequence: crkbd's `_NUMERIC` stops using `KC_P0..P9` and becomes Lily58-style
      (number-row digits via `NUMGRID_1..9,0`, `NUM_GRD_F`/`NUM_GRD_G`). That is a layout
      change needing a key-placement design for 46 keys -> new Phase 4b below.

## Phase 1 — Skeleton userspace (builds identical firmware)
- [x] Create `users/marce/{rules.mk,marce.h,marce.c,config.h}`.
- [x] `users/marce/rules.mk`: `SRC += marce.c` only. Shared feature flags deferred to phase 5
      to keep this phase byte-identical.
- [x] Add `USER_NAME := marce` to both keymaps' `rules.mk` (own line: an inline `# comment`
      leaves trailing whitespace in the value and breaks the build with "users/marce: Is a directory").
- [x] Build both; sizes must match the Phase 0 baseline (empty userspace).
- [x] Commit: "users/marce: add empty shared userspace".

## Phase 2 — Shared definitions (header only)
- [ ] `marce.h`: `enum layers`, `LOWER`/`RAISE` macros, shared `custom_keycodes`
      (`ALTGR_MC`, `DIG_TL..DIG_BR`), `TAP_SPC_ENT` tap-dance enum.
- [ ] Board-specific keycodes: each keymap declares its own enum starting at a
      `MARCE_SAFE_RANGE` macro exported by `marce.h`.
- [ ] Remove the duplicates from both `keymap.c`; keep `_NUMERIC` ordering invariant
      (must stay below LOWER/RAISE/ADJUST) and document it in `marce.h`.
- [ ] Build both, sizes unchanged, flash lily58 and sanity check layers.
- [ ] Commit.

## Phase 3 — Move self-contained subsystems (one commit each)
For each: move code from LILY to `users/marce/`, delete from both keymaps, build both,
compare sizes, test on hardware.
- [ ] 3a. Digitizer: `digitizer_points[]` and the `DIG_*` handler.
- [ ] 3b. Tap dance: `tap_dance_actions[]`, `TAP_SPC_ENT` callbacks.
- [ ] 3c. `hold_key_*` helpers (`hold_key_press`, `hold_key_was_tap`).
- [ ] 3d. AltGr / Mac Unicode subsystem (`altgr_map`, `dead_*`, `send_mac_unicode`,
      Mac/PC mode state, `ALTGR_MC`). Largest change; crkbd switches to the 16-bit
      tables here (see Phase 0 decision). Keep `UNICODE_ENABLE` consistent (Lily58 = no,
      hand-rolled; crkbd = yes) — pick one, ideally the hand-rolled path.
- [ ] 3e. Layer state: `layer_state_set_user` tri-layer logic, with a weak
      `layer_state_set_marce_kb(state)` hook so crkbd can call `update_rgb_matrix`.

## Phase 4 — Single `process_record_user` dispatcher
- [ ] `marce.c` `process_record_user` handles shared keycodes, then calls weak
      `bool process_record_marce_kb(uint16_t, keyrecord_t *)`.
- [ ] Lily58 keymap implements it for `NUM_LAYER`, `NUM_GRD_F/G`, `NUMGRID_*`.
- [ ] crkbd keymap implements it for `CW_CTL`, `NUM_MO`, `NUM_OS`.
- [ ] Preserve ordering semantics: Lily58's `hold_interrupted` bookkeeping must run
      for every keypress, before board-specific handling.
- [ ] Same for `matrix_scan_user` (crkbd `CW_CTL` timer) via a weak hook.

## Phase 4b — Port the NUMGRID F/G overlay to crkbd (layout work)
- [ ] Design crkbd `_NUMERIC` placement: 3x3 grid on the right hand (rows map 1:1 to
      Corne rows), `NUMGRID_0` on a thumb or bottom-row key, `NUM_GRD_F`/`NUM_GRD_G` on
      left-hand home-row keys (as on Lily58: F/G positions). Agree on it with the user
      BEFORE editing; it replaces `KC_P7..P0`, `KC_PDOT`, `KC_PEQL` etc.
- [ ] Keep crkbd's entry keys (`NUM_MO`/`NUM_OS`, `TO(0)`) working; decide whether
      `NUM_LAYER`'s tap-one-shot/hold-momentary (Lily58) replaces `NUM_MO`/`NUM_OS` or both stay.
- [ ] Move `NUMGRID_*`, `NUM_GRD_F/G`, `numeric_grid_mode`, `numgrid_sent[]` and their handling
      into `users/marce/` (now shared, no longer Lily58-only), gated by nothing.
- [ ] Update `gen_keymap_viz.py` labels for the new keys; regenerate `keymap-viz.html`.
- [ ] Build both, test on hardware, commit.

## Phase 5 — Config, rules, tooling
- [ ] Move shared `config.h` defines (`SPLIT_LAYER_STATE_ENABLE`, `OLED_TIMEOUT`,
      `SPLIT_USB_DETECT` if appropriate) into `users/marce/config.h`.
- [ ] Trim each keymap's `rules.mk` to board-only overrides, with comments saying why.
- [ ] Update `gen_keymap_viz.py` (both copies) if it parses things that moved; ideally
      de-duplicate into one script in `users/marce/` with a per-board config.
- [ ] Optional: drift check script that compares layer *semantics* across boards
      (which keys exist on each layer) and flags unintended divergence.

## Phase 6 — Docs
- [ ] `users/marce/README.md`: what lives where, how to add a feature, weak hook list.
- [ ] Update `lily58/keymaps/marce/CLAUDE.md` architecture section; add a matching
      `crkbd/keymaps/marce/CLAUDE.md`.
- [ ] Move the "Deliberate changes" comment block from crkbd `keymap.c` into the crkbd
      CLAUDE.md as a Lily58→Corne mapping table (rows, absent keys, replaced keys).

## Verification checklist (run after every phase)
- [ ] `qmk compile` for lily58/rev1 and crkbd/rev4_1/standard build with no warnings
- [ ] Flash size recorded in log, no regressions beyond expectation
- [ ] On hardware (at least after 3a, 3d, 4): layer switching, tri-layer ADJUST,
      Space/Enter tap dance, a digitizer point, AltGr chords (á, ñ, ¿), dead key, Mac mode toggle

## Risks
- Flash overflow on Lily58 (AVR) when shared code lands; abort and revert the phase if the size check fails. crkbd (RP2040) is not size-constrained.
- Keycode enum reordering changes `SAFE_RANGE` offsets; harmless unless Vial/VIA
  remaps are stored, but note it.
- `UNICODE_ENABLE` mismatch between boards (see 3d).
- Corne's numpad muscle memory changes (KC_P* -> number-row digits; Num Lock no longer matters).
- Lily58 `NUMGRID_0..9` contiguity requirement must survive any enum move.

## Progress log
- 2026-10-11: plan written, nothing started.
- 2026-10-11: Phase 0 baseline done (branch feature/marce @ 399b0abf61, clean apart from this file).
  - lily58/rev1 (AVR): builds OK, no errors. 18492/28672 B (64%), 10180 B free. Hex 52025 B.
  - crkbd/rev4_1/standard (RP2040): builds OK. ARM size text=52512, bss=264992. UF2 105472 B.
  - Both targets compile on the host with `qmk compile` (avr-gcc + arm-none-eabi-gcc present).
  - Decisions made: crkbd adopts Lily58 AltGr table, hold_key helpers and NUMGRID F/G overlay.
  - Phase 0 complete. Next: Phase 1.
- 2026-10-11: Phase 1 done. Skeleton builds; sizes identical to baseline (lily58 18492 B, crkbd text 52512). Next: Phase 2.
