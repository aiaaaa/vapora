# VAPORA 1.1.4

The sound and 15-preset set are the approved 1.0 baseline. Version 1.1.4 updates
only the approved sun designs and order; do not reinstall presets for this update.

Native six-voice PADsynth-based instrument for the original OP-1 / Hundekraft
3.0.2. Two independently phased, detuned playback heads per voice. The OP-1
supplies the T2 envelope and optional T3 effect.

## Suns

01 Solid (default) → 02 Lines → 03 Halo → 04 Retro → 05 Bands → 06 Saturn → 07 Triple.
Solid is the approved trial 10; Lines is the approved trial 21 at 4px thickness.
Push white to cycle; turn white to move/open the sun with Brightness.
The Holy Dust showcase is an idle ten-second down/up sweep with no notes played or held.

## Sound and CPU

VAPORA has used two playback heads since R1. No oscillator was removed to obtain
the R3 CPU improvement. The major changes were half-rate synthesis (R2), then
one guarded table level, one shared transition fade, block-rate coefficient
ramps, constant-pitch preparation, inlined table reads, symmetric reconstruction
and staggered screen telemetry (R3). EVA and Oxbow use related half-rate and
block-preparation techniques. `CPU_AUDIT.md` records the actual source evidence.

PADsynth defines offline spectral broadening, random phases and IFFT construction
of periodic waveforms; it does not require a particular playback oscillator
count. R6 added many short, sparse spectra for variety. R7 returns more of that
space to broad ensemble textures: **six new 8,192-sample spectra**, with seven
detuned Gaussian clouds per partial. Their loop lasts about 0.489 s at C4,
0.979 s at C3. The original two 16,384-sample WARM endpoints remain unchanged.
There are 20 endpoints and 448,800 sample bytes in total.

The production DSP source, two heads, six voices, gain, filtering and release
processing are unchanged from R6. All 15 preset release words remain 12500.
The eight older sounds are checked against R6 at three pitches and three
Morph positions using identical seeds: exact dry PCM. Deep Pressure adds a
saved effect, so its final wet output intentionally changes.

The earlier user report that CPU improved is device evidence, but we do not
have a controlled OP-1 timing comparison attributing the improvement to each
optimization. Host timing is only a regression check; it cannot establish a
Blackfin deadline or a device CPU percentage. Reverb adds its own processing
cost. No new claim of measured device CPU is made for R7.

## Approved 15-preset core set

Current names: `a fifth tide` replaces `fifth tide`; `XFIN` replaces `end titles`.
Both are name-only changes. The final bank has 15 unique names and is already
installed. Preserve it for this plugin-only update.

This is the set to preserve and develop going forward. Keep the existing Character
assignments stable; `data/core_set.json` locks the names and Character order.

Older sounds: Asteroid Swarm, Breath Choir, Cavern Flute, Deep Pressure, Pink Surf,
Porcelain Orbit, Sub Lantern and A Fifth Tide.

Selected cinematic sounds: Holy Dust, XFIN, Ice Chapel, Last Light,
Moon Choir, Stone Hall and Tidal Arc. The other 13 R7 evaluation presets are
retired from the active bank. Archived deliveries remain unchanged.

The audio engine and all 14 Character sources are unchanged from R7. The core
bank is compatible with R7. The seven retained R6 sounds use corrected R7
Character words; A Fifth Tide keeps the original WARM assignment. Do not overwrite
these files with the R6 versions: their old Character words select other sounds.

The October 20 disk inspection found one file per preset name, but the seven
older keeper files still carried R6 Character words. Browser doubles were
reported despite no duplicate files or hidden trash on the exported disk. Their
internal cause is not proven. Process removal in a separate disk synchronization
before importing the corrected bank; verify the browser on the instrument.

Presets save one T3 effect with its settings, not a multi-effect chain or the
master effect. **Deep Pressure saves Reverb on** (time 70%, tone 46%, diffusion
73%, mix 31%). It needs Hundekraft's `reverb` plugin installed. The other retained
sounds and the seven selected cinematic pads are dry, so their source character can be evaluated.
T3 can be disabled to compare Deep Pressure dry.

## Visual changes

The cycle starts at **01 SOLID**, followed by **02 LINES, 03 HALO, 04 RETRO,
05 BANDS, 06 SATURN, 07 TRIPLE**. Solid is the approved native cubic disk/cover
from trial 10; Lines is the approved seven-stroke trial 21, with fixed 4px
width and 2px gaps. Both use a 20px circle envelope and open/close with Brightness.
The remaining five designs preserve their approved geometry; Retro's lowest
stripe stays removed and Triple remains enlarged.

The two selected methods were approved by the user on the OP-1 in comparison
builds. Their geometry is checked unchanged in this combined build. Final
installation/order checks for 1.1.4 remain separate from those approvals.

The circled upper mountains are raised symmetrically: corner peaks 3 px,
shoulders 4 px, next inner top vertices 3 px. Top shoulder movement grows from
4% to 16%, the next row blends at 8%, and top corner tips drop to 2% response.
The next inner top vertices gain one-third more motion. Other facets retain
the prior geometry. The wave clipping / readout blackout boundary drops 5 px
to y=126; the dial and text stay aligned. Daytime particle speed is unchanged.

## Install and evaluate

Update only `vapora.knl` through Workshop **Your plugins / Update OP-1**.
Let the instrument complete installation and restart. The 15 approved presets,
including `a fifth tide` and `XFIN`, are already final: do not copy the bank again.
This release does not alter sound words, Character assignments, DSP or tables.

Check Solid starts by default, then Lines and the remaining designs. Sweep
Brightness at minimum, maximum and slowly between them. Inspect Halo,
Bands, Saturn and Triple. Check that circles are round and their edges do not
notch or flicker. Compare the device against the host preview knowing that the
preview uses a substitute font and a different rasterizer. No physical OP-1
rendering or CPU measurement has been performed for this release.

The archive's individual dry WAVs are validation fixtures, with approximate
host envelopes. Deep Pressure's saved Reverb is heard only on the instrument.

## Controls and existing input limitation

| Encoder | Turn | Press, then turn |
|---|---|---|
| Blue | Morph | Character: 14 choices, one step apart |
| Green | Thickness | Interval: ten choices |
| White | Brightness | Press cycles seven suns |
| Orange | Motion | Speed: 0.02–2 Hz |

The most recently edited control appears in the single readout row. Secondary
zero means the 8192 default. The host owns velocity and T2 amplitude envelopes.
Animation state is not stored in hidden sound parameters.

White and orange share secondary word 7 in this core. The plugin clears the
white toggle at redraw: wait for the sun change before turning white. A turn
before that redraw can reach Speed; clicks within one redraw can coalesce.
The standalone API cannot expose continuous button holds. The approved secret
remains: normal pages, blue → green → orange → white clicks, then seven seconds
without changes. The three bouncing labels exit on a knob action.

## Rebuild and validation

Pinned Kennel `7c790676ad555da2b8df7a7851826055dee32b2c`, Hundekraft 3.0.2,
Blackfin GCC 13.3.0 / binutils 2.42. From this source directory:

```sh
python3 tools/build.py --toolchain /path/to/toolchain
```

The full build checks deterministic tables, quantization/band limits, 15 AIF
round trips, ASan/UBSan production DSP and GUI harnesses, three-minute six-voice
state, 19 dry WAVs, geometry bounds, folded-triangle prevention, sun translation
and eclipse progression, the secret trigger, native compilation, imports and
loader relocation against GNU ld at two addresses. Exact sizes and hashes are
in `validation/package.json` and `build-record.json`.

Additional repeatable comparisons:

```sh
python3 tools/check_retained.py /path/to/r6/source/vapora /path/to/validation
python3 tools/compare_host_cpu.py /path/to/r6/source/vapora /path/to/validation/host-cpu-comparison.json
```

See `HARDWARE_CHECKLIST.md`, `CPU_AUDIT.md`, the revision record and licenses.
