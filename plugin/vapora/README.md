# VAPORA 1.2.4

Native six-voice PADsynth instrument for the original OP-1 / Hundekraft 3.0.2.
Two independently phased, detuned playback heads per voice; 14 sound Characters
and 15 factory presets. The OP-1 supplies velocity, T2 envelope and optional T3
processing. This release refines the star field and green signature designs.
Small/tiny stars drift at 37.5% of the medium/large stars' speed. The blue wave,
audio, all 15 presets and saved parameter mappings are unchanged from 1.2.3.

## Controls and visuals

| Encoder | Turn | Press, then turn |
|---|---|---|
| Blue | Morph: sound blend and wave movement | Character: sound family and wave style |
| Green | Thickness: detune/fullness; signature rotation and size | Interval: second-head pitch and added star field |
| White | Brightness: filter and sun height/opening | Each press cycles the seven suns |
| Orange | Motion: sound modulation and sun pulse depth | Speed: modulation and sun pulse rate |

Blue, green and orange presses toggle their main/secondary control. White turns
always return to Brightness after the next redraw. White and orange share
secondary word 7 in this core: wait for the sun change before turning white.
A turn before redraw can reach Speed; clicks within one redraw can coalesce.
The firmware's stock knob scrolling and saved-word encoding are retained.

Shift toggles the complete bottom control readout and dial, initially hidden.
It stays in the chosen state across screens and slots during the session;
visibility is not saved in sounds or across a fresh plugin load. Knob edits do
not reveal a hidden readout. Its black backing appears only when shown. The
underlying waves always use the full available area, clipped at y=151 instead
of the former y=126 boundary. Hiding the readout exposes those same waves.

Shift reads the pinned Hundekraft 3.0.2 core's read-only state byte. No firmware
patch or companion is required. A Shift tap entirely between redraws can be
missed; a Shift already held on screen re-entry does not toggle the readout.

## Character waves

Character selects the following visual styles in existing audio Character order.
The readout names the sound Character (Warm, Swarm, Breath, etc.), not its
wave style. Morph changes wave movement. The pointer follows one 270-degree sweep over the
raw encoder word while the names retain the firmware's usual cycling.

| Character | Wave style | Existing sound |
|---|---|---|
| 01 | Original | Warm |
| 02 | Ripples | Swarm |
| 03 | Roll | Breath |
| 04 | Fold | Flute |
| 05 | Orbit | Pressure |
| 06 | Rings | Surf |
| 07 | Rain | Porcelain |
| 08 | Pulse | Sub |
| 09 | Tide | Velvet |
| 10 | Ribbons | Cathedral |
| 11 | Weave | Nebula |
| 12 | Dunes | Aurora |
| 13 | Echo | Oceanic |
| 14 | Jelly | Glassveil |


Each key or grouped chord creates a sharp hit at a random interior location.
One ripple moves outward about 4.5 grid cells as its amplitude falls to zero;
a small trailing wake is one eighth of the main pulse's strength. It settles in
about 1.4 seconds after a tap. Holding a participating note slows the fade
slightly and lets the same front travel farther, capped at about 1.75 seconds.
Holding also broadens the trailing return up to twice its width and softens
the rebound, while preserving the sharp incoming edge and the 1.75s cap.
Release stops earning extension; an unrelated held note or reused voice cannot
sustain an older hit. Even an indefinitely held chord settles completely. Held
notes never produce continuous audio-trace bending. Overlapping hits share an
8px bound, with local-spacing limits and absorption near the rim.

Lower side joints target the full wave displacement, up from 25%, subject to
neighboring triangle limits. Bottom joints can travel into the former readout
strip. The right outline is inset 8px to keep the pointed corners visible.
Thickness, Interval, Motion and Speed have no direct wave-geometry effect.

## Interval stars and Thickness

The base field has 48 cool stars (8 large, 16 medium, 24 small). Interval adds
a distinct seeded field; changing Interval immediately replaces its placement.
Small/tiny stars use a separate clock at 37.5% of the larger stars' speed.
All small stars follow smooth subpixel paths and fade near the sun and screen
wrap. They can overlap instead of jumping sideways to avoid one another.

| Interval | Added distribution (before sun occlusion), stock size | Color |
|---|---|---|
| Octave below | 8 large, 4 medium | Cool |
| Fifth below | 8 large, 4 medium | Green |
| Unison | 8 large, 8 medium, 16 small | Cool |
| Minor third | 8 large, 8 medium, 16 small | Green |
| Major third | 18 medium, 6 small | Cool |
| Fifth | 18 medium, 6 small | Green |
| Major sixth | 8 medium, 32 small | Cool |
| Minor seventh | 8 medium, 32 small | Green |
| Major seventh | 48 tiny | Cool |
| Octave above | 48 tiny | Green |


Each Interval also adds a green signature: five retained outline stars alternate
with a capped compass plus, forked diagonal cross, six-ray asterisk, eight-ray
asterisk and short-diagonal cross. Thickness rotates each through a full turn
and grows it up to 60%. Signatures cross the sky and pass behind the sun.

## Sun

01 Solid (default) → 02 Lines → 03 Halo → 04 Retro → 05 Bands → 06 Saturn →
07 Triple, then Solid. Solid uses the approved native cubic disk/cover; Lines
uses seven horizontal strokes with 4px widths and 2px gaps. Other approved
shapes remain, including the removed lowest Retro stripe and enlarged Triple.

Brightness raises the center from y=56 to y=36, a 25% shorter rise than before,
and controls opening. Motion zero keeps the sun white. Increasing Motion sets
pulse depth toward red-orange #ff423f; Speed sets about 0.02–2 Hz. This visual
LFO is free-running, not a phase-accurate meter of individual audio voices.
The DSP still applies Motion to morph, detune and brightness. The ocean uses its
former default ambient Motion/Speed independently of the orange controls.

Blue readouts use #698eff; green readouts and added green stars use #00ed95.
Orange readout, dial and pulse endpoint share #ff423f. These are reference
software colors from the OP-1 Design Kit, not calibrated OLED measurements.
Wave colors, base stars and the other dial paints remain unchanged.

## Presets and installation

Update `vapora.knl` through Workshop **Your plugins / Update OP-1**. Existing
users upgrading from 1.2.3 should update only the plugin. Older installations may need to replace only
`synth/vapora/stone hall.aif` for the Plate binding. New users can
install the separate factory bank into `synth/vapora` in normal disk mode.
Do not replace current presets with older R6 evaluation files: their Character
words select different sources. `data/core_set.json` locks the final names and
Character assignments; `PRESET_CATALOG.csv` and `presets/recipes.json` document
this bank. Save edits using the OP-1's usual sound-key gesture.

Deep Pressure saves the separate Hundekraft Reverb effect (time 70%, tone 46%,
diffusion 73%, mix 31%). Stone Hall saves Plate enabled with readouts
Decay 99, Pre 99, Tone 00 and Mix 33, and starts at octave -3. Each requires its respective effect to be
installed. The other thirteen presets are dry. Host fixtures do not include
native T2 or T3 effects.

## Sound and implementation

The two-head, six-voice DSP has a fixed 3x output boost relative to 1.2.2.
There is no compressor or automatic level adjustment. Offline PADsynth tables contain
broadened spectral components; these are not extra runtime oscillators. There
are 20 endpoints and 448,800 sample bytes, including the original two long Warm
tables and six 8,192-sample cinematic spectra. Half-rate synthesis, block-rate
preparation, guarded table transitions and staggered telemetry remain as
recorded in `CPU_AUDIT.md`. No device CPU percentage is inferred from host tests.

Animation state is not stored in hidden sound parameters. The existing secret
remains: from normal pages, press blue → green → orange → white, then wait seven
seconds without changes. The three bouncing labels exit on a knob action.

## Rebuild and validation

Pinned Kennel `7c790676ad555da2b8df7a7851826055dee32b2c`, Hundekraft 3.0.2,
Blackfin GCC 13.3.0 / binutils 2.42. Place this directory at
`ext/kennel/synth_sdk/plugins/vapora` inside the matching SDK. From this directory:

```sh
python3 tools/build.py --toolchain /path/to/toolchain --skip-previews
```

Use the dependencies in `requirements.txt`. Omit `--skip-previews` to render
host media; the 1.2.4 build generates no GIFs. No media generation is required
to build the plugin.

Checks cover deterministic tables, 15 AIF round trips, production DSP and screen
ASan/UBSan tests, three-minute six-voice state, 19 dry audio fixtures, overlapping
note impacts, Shift toggling, wave bounds/side-triangle continuity, all ten star
fields, all seven suns, native imports and relocation against GNU ld at two
addresses. Native build and host validation pass. Controlled hardware CPU/GUI
timing and Stone Hall Plate save/reload verification remain pending.

Additional dry headroom sweep (13,440 six-voice cases):

```sh
clang -O2 -std=c11 -I . tests/output_level.c vapora_dsp.c -lm -o /tmp/vapora-level-check
/tmp/vapora-level-check
```

The sweep tests actual independently phased voices, including repeated-note
unisons. It does not guarantee arbitrary phase-aligned sums or downstream FX
headroom. Compare on hardware before publication.

See `HARDWARE_CHECKLIST.md`, `CPU_AUDIT.md`, `TOOLCHAIN.md`, `CHANGELOG.md` and
retained license/third-party notices. PADsynth algorithm by Nasca Octavian Paul.
