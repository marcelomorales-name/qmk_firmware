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
- [x] `marce.h`: `enum layers`, `LOWER`/`RAISE` macros, shared `marce_keycodes`
      (`ALTGR_MC`, `NUMGRID_1..0`, `NUM_GRD_F/G`, `DIG_TL..DIG_BR`), `TAP_SPC_ENT` enum.
      NUMGRID/NUM_GRD keycodes are declared here already (decided shared); crkbd just
      doesn't use them until phase 4b.
- [x] Board-specific keycodes: each keymap declares its own enum starting at a
      `MARCE_SAFE_RANGE` macro exported by `marce.h`.
- [x] Remove the duplicates from both `keymap.c`; keep `_NUMERIC` ordering invariant
      (must stay below LOWER/RAISE/ADJUST) and document it in `marce.h`.
- [x] Build both: crkbd identical; lily58 +8 B (keycode values shifted).
- [ ] Hardware check (user): flash both, sanity check layers/keys. Not done by Claude.
- [x] Commit.

## Phase 3 — Move self-contained subsystems (one commit each) — DONE
- [x] 3a. Digitizer (`digitizer_points[]`, `DIG_*` handler) + shared `process_record_user`
      dispatcher in marce.c that first calls weak `process_record_marce_kb` (pulled forward
      from phase 4).
- [x] 3b. Tap dance: QMK keymap introspection requires `tap_dance_actions[]` to be defined in
      keymap.c, so it is the `MARCE_TAP_DANCE_ACTIONS` macro in marce.h, expanded per keymap.
- [x] 3c. `hold_key_press` / `hold_key_was_tap` + interrupt bookkeeping in the dispatcher.
- [x] 3d. AltGr / Mac Unicode subsystem. crkbd now uses Lily58's table and the hand-rolled
      send_mac_unicode; `UNICODE_ENABLE` and `set_unicode_input_mode` removed from crkbd.
      `mac_altgr_mode` is a global (OLED reads it); weak `mac_altgr_mode_changed_marce_kb`
      lets crkbd recolor RGB on toggle.
- [x] 3e. Layer state: shared `layer_state_set_user` (tri-layer) + weak
      `layer_state_set_marce_kb`.

## Phase 4 — Single `process_record_user` dispatcher — DONE (via 3a)
- [x] Dispatcher: board hook first, then DIG, then AltGr. Hold-interrupt bookkeeping first of all.
- [x] Lily58 hook: `NUM_LAYER`, `NUM_GRD_F/G`, `NUMGRID_*` (moves to shared in 4b).
- [x] crkbd hook: `CW_CTL`, `NUM_MO`, `NUM_OS`, `TO(0)`.
- [x] `matrix_scan_user` (crkbd `CW_CTL` timer) stays in crkbd keymap.c: the shared code
      doesn't define it, so there is nothing to hook.

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

## Phase 5 — Config, rules, tooling — DONE
- [x] `users/marce/config.h`: `SPLIT_LAYER_STATE_ENABLE`, `OLED_TIMEOUT`. `SPLIT_USB_DETECT`
      stays in lily58 (Elite-C specific).
- [x] `users/marce/rules.mk`: DIGITIZER, LTO, MOUSEKEY, TAP_DANCE. NOT `CAPS_WORD_ENABLE`: only
      crkbd uses it (moving it to shared cost lily58 +380 B). Rule: only share a flag if
      both boards already set it, and compare sizes.
- [x] `gen_keymap_viz.py` (both): also reads `enum layers` from users/marce/marce.h.
      Regenerated HTML is unchanged. Merging the two scripts into one is NOT done (optional).
- [ ] Optional: drift-check script comparing layer semantics across boards.

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
- 2026-10-11: Phase 2 committed. lily58 18500/28672 B (+8), crkbd unchanged. Hardware check still owed by user. Next: Phase 3a.
- 2026-10-11: Phase 3 (3a-3e) and Phase 4 done, one commit per step. Sizes: lily58 18500/28672 B, crkbd text 50908 (-1.6 KB from the smaller AltGr table). No hardware testing done. Next: 4b needs a key-placement decision from the user; phases 5-6 can go ahead meanwhile.
- 2026-10-11: Phase 5 done. Gotcha: sharing CAPS_WORD_ENABLE grew lily58 by 380 B; keep it crkbd-only. Sizes back to lily58 18500, crkbd 50908.
