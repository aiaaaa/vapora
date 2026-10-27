# Hundekraft browser preview — experimental source adapter kit

Build a playable, static browser preview from an OP-1 plugin's C source. Vapora
is the first production integration. This is a project-local prototype for a
future SDK contribution, not an official Hundekraft SDK release or an OP-1 emulator.

The same web host supports synth notes and stereo effect input. Its HTML, control
layout, keyboard, AudioWorklet, canvas renderer, patch files, and build command do
not depend on Vapora. Each plugin supplies a manifest and a small C source adapter.
New plugins do **not** need to target the OP-XY SDK.

## Build and play

Requirements: Python 3.10+, Clang with the wasm32 target and `wasm-ld`; Node 22+
for tests. On Ubuntu 24.04 install `clang-18 lld-18`; on macOS a portable
[WASI SDK](https://github.com/WebAssembly/wasi-sdk/releases) supplies both tools.
Pass its `bin/clang` with `--cc`. The output itself has no WASI imports.

```sh
python3 preview-sdk/test.py --cc clang-18 --native-cc clang-18
python3 -m http.server 8919 --bind 127.0.0.1 --directory site
```

Open `http://127.0.0.1:8919/`, click **Enable audio**, and play the keyboard or
**Play a chord**. Escape/Stop all, changing presets/octave, window blur, and hiding
the tab stop notes. Sound starts only after a user gesture. No device or firmware
file is needed. Use `--node /path/to/node` or `--native-cc /path/to/clang` in tests.

Build only:

```sh
python3 preview-sdk/build.py --cc /path/to/clang
```

Deploy the generated `site/` directory unchanged to HTTPS static hosting. Asset
URLs are relative so GitHub project paths work. On GitHub, select **Settings →
Pages → Source → GitHub Actions**. The Browser preview workflow tests both adapter
types before deploying. It also runs tests for pull requests without deploying.
No Node process, Python server, database, accounts, or device connection is needed
on the public site. The local server is only a development convenience.

## Add another plugin

1. Copy `examples/gain/plugin.json` and `adapter.c` into the new plugin's preview
   folder. For a synth, use `../preview/plugin.json` and its adapter as a worked
   example, retaining the original source files and headers.
2. Declare `kind` (`synth` or `effect`), parameters, knobs, optional alternate
   controls/buttons, screen size, presets, source paths, include paths and defines.
   Source/include paths are relative to the manifest. Parameter IDs are contiguous
   from zero, values normalized to [0,1], and IDs remain stable across patch files.
3. Implement the callbacks in `include/preview.h` by forwarding to the original
   engine and screen code. Replace device services with host equivalents; never
   dereference firmware addresses in WebAssembly. Read-only tables can compile
   unchanged. Unsupported services need explicit adapters, not silent stubs.
4. Build and test the result:

```sh
python3 preview-sdk/build.py --plugin path/to/plugin.json --out local-preview --cc clang-18
python3 -m http.server 8920 --bind 127.0.0.1 --directory local-preview
```

`examples/gain` is a small, original stereo gain effect. It proves the input path
and gives future effects a runnable starting point; it is not a native `.knl`
effect. The browser lets effect previews load and loop an audio file locally.
There is no microphone request or upload.

## ABI 1 and ownership

| Area | Contract |
|---|---|
| Audio | 44,100 Hz, blocks of 128, planar float stereo input/output; reject a mismatched audio context. |
| Synth | Six voices, integer pitch increment, Q15 velocity and audition envelope. Adapter calls the original per-voice render callback. Host applies the envelope once and sums mono output to both channels. |
| Effect | Full stereo input/output blocks. Adapter owns effect state and translates to its native interface. No synth note allocation or envelope is applied. |
| Parameters | IDs 0–63, finite normalized values. Adapter translates to native words. UI values update audio on the next block. |
| Actions | IDs 0–63 for preview screen controls (press/release, page and Shift). ABI 1 actions are visual only; audio controls use parameters. |
| Screen | Separate WASM instance on the UI thread, up to 4,096 commands/frame, Q10 line/circle/text commands, 80 bytes/command. Overflow is an error. |
| Telemetry | At most 1,024 bytes copied from audio to screen at about 30 Hz. Adapter owns its schema; synth audio and GUI share no mutable memory. |
| Memory | Fixed 4 MiB, no module imports or WASM allocation/growth. JavaScript copies telemetry at about 30 Hz. Drawing happens on the UI thread. |
| Patches | Versioned browser JSON, validated before application, max 16 KiB. These are not OP-1 preset/AIF files. |

The generic host applies a linear ADSR audition envelope; it does not reproduce
TE's T2 envelope. Vapora's preset durations come from its existing tested host
fixtures. Saved T3 effects are explicitly omitted. Preset octaves are applied.
Display geometry and animation execute the original C code. Canvas rasterization
and the substitute text font are not pixel-identical to the device OLED.

## What is supported now, and what comes next

- **Working:** Vapora synth, parameter/alternate controls, presets, notes, screen,
  telemetry, patch import/export, and local audio-file input for effect adapters.
- **Validated foundation:** stereo gain effect through the same build and worklet,
  proving that the shared host has an audio-input path independent of synth voices.
- **Future integrations:** native Hundekraft effect wrappers, SDK allocation/scratch
  services, reusable retained SVG nodes and richer text primitives as new plugins
  require them. Vapora currently has a small adapter for its two sun nodes.
- **Not covered:** arbitrary `.knl` loading, firmware emulation, stock filters/FX,
  LFO/sequence/transport plugins, sample loading, USB, or hardware CPU measurement.
  These need explicit capabilities and tests. Unknown manifest kinds fail builds.

Keep plugin behavior in source adapters and device code. Add host capabilities
when an actual second plugin needs them, with a fixture and parity tests; avoid
copying and modifying the shared web host for each instrument. Once the adapter
boundaries have been exercised by real synth and effect plugins, the kit can be
proposed for inclusion in Hundekraft's SDK.

## Validation

`test.py` compiles a native adapter and the browser WASM from the same sources and
compares 65,536 float output samples across note start, overlap, and release. It
then exercises all 15 presets with six voices, finite output/headroom, voice
stealing, panic/release, all 14 characters × seven suns, telemetry copies, patch
validation, independent stereo effect channels, and the actual worklet transport.
Results are written to `.preview-tests/results.json`. The stereo fixture uses a
known input/output relationship; the native comparison catches browser-port DSP
differences. Neither test predicts OP-1 hardware performance.

## License and provenance

New preview code follows this repository's GPL-3.0-or-later license with the
existing OP-1 firmware combination permission. `../preview/adapters/include/seq/seq_text.h`
is copied unchanged from the pinned Kennel SDK used by Vapora 1.2.4; it retains
ultralazr's copyright and license header. See `../NOTICE` and `../THIRD_PARTY_NOTICES.md`.

The Open-XY Machine Workbench inspired the preview workflow. This implementation
does not copy its code and does not require Open-XY. No TE fonts, firmware, keys,
or extracted proprietary assets are distributed. `site/build.json` records the
compiler and source/header hashes used for each browser module.
