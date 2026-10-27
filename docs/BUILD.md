# Build VAPORA 1.2.4

Download the [matching Hundekraft 3.0.2 source and SDK](https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.2-source.tar.gz) and verify its [published checksum](https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.2-source.tar.gz.sha256).

The SDK archive used for this release has SHA-256 `35d481d4fd6e3d0e8699bf6fccd8e548c2929ca9155b82b909c175ef24de9700`; Kennel commit `7c790676ad555da2b8df7a7851826055dee32b2c`. Toolchain: Blackfin GCC 13.3.0, binutils 2.42.

Copy this repository’s `plugin/vapora` directory to the SDK’s `ext/kennel/synth_sdk/plugins/vapora`. From `ext/kennel`, pack the checked-in source and tables:

```sh
python3 tools/kennel/pack.py synth_sdk/plugins/vapora \
  -o build/vapora.knl --toolchain /path/to/toolchain
```

The toolchain directory contains `bfin-gcc/bin` and `bfin/bin`. Read the SDK’s toolchain setup instructions. No firmware image is needed to compile this plugin.

For generation and full host validation, use Python 3 with the dependencies in `synth_sdk/plugins/vapora/requirements.txt`, and Clang with ASan/UBSan support:

```sh
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install -r synth_sdk/plugins/vapora/requirements.txt
python3 synth_sdk/plugins/vapora/tools/build.py --toolchain /path/to/toolchain --skip-previews
```

Outputs are `build/vapora.knl` and `build/vapora-1.2.4-validation/`. The build script expects this SDK directory layout. Keep it inside `synth_sdk/plugins/vapora`, rather than invoking it from the standalone GitHub checkout.

Checks cover table generation, 15 preset round trips, production DSP and screen harnesses, 19 dry audio fixtures, wave bounds, all seven suns, control behavior, native imports and loader relocation against GNU ld at two addresses. Host rasterization and timing do not establish native display quality or hardware CPU limits.

Compiled release: 481,756-byte plugin; SHA-256 `2b096e3cea44ee4242567f39cce34e5b2141cf41eb3a327d9e579f38ff7e4911`.

The release build uses `--skip-previews`: all SVG drawing-command checks still
run, but no GIFs or other preview media are generated. Omit that option when
media exports are wanted. The user’s existing public images and GIFs are retained unchanged.

Additional checks cover default-hidden Shift toggling, conditional readout
backing, outward note/chord ripples, fourteen Character styles, ten Interval
fields/signatures, all 32768 Thickness values, and 16384 random side-geometry
samples. Small-star continuity is checked for 512 frames per Interval, plus sun occlusion and the 37.5% drift ratio. Wave geometry, DSP, tables, saved mappings and all presets are unchanged from 1.2.3. The loaded image, relocations, imports and SVG match the approved internal.4 build byte-for-byte; only the package version label changes. Host fixtures are dry and omit saved Reverb/Plate effects.

The output-level sweep is separate from the general build:

```sh
cd synth_sdk/plugins/vapora
clang -O2 -std=c11 -I . tests/output_level.c vapora_dsp.c -lm -o /tmp/vapora-output-level
/tmp/vapora-output-level
```

It checks 13,440 full-envelope six-voice cases covering all Characters/Intervals, Morph/Thickness positions, phase seeds, registers, Motion settings, chords, clusters and repeated-note unisons. The measured peak is 26,243/32,767, with no clipping. This is a host dry-sum check, not a native mixer/FX performance measurement or a guarantee for every possible phase combination. The three-minute sustained-chord check also passes.
