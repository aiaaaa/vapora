# Hundekraft handoff — prepared, not submitted


Reviewed 20 October 2026 against the [Workshop repository](https://github.com/Hundekraft/op1-workshop) and the `PLUGIN_GUIDE.md` in the [3.0.2 SDK bundle](https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.2-source.tar.gz).


The SDK guide describes sharing independent plugins; no separate formal submission process was found in the Workshop repository pages reviewed. Catalog inclusion is a maintainer decision.


## Materials requested by the SDK guide


- Compiled `.knl`, corresponding source and build instructions.
- Required core/SDK version, controls and resource claims.
- License and third-party notices, artwork/preset credits and a short changelog.
- Separate reporting of compilation, host/emulator checks and hardware checks.
- Stable unique plugin ID; no firmware images or device keys.


VAPORA supplies these materials. Its stable ID is `vapora`, manifest `order` is 0, and resource claims are empty. It is a standalone synth with no companion plugin or core patches. The optional Reverb setting on Deep Pressure requires that effect to be installed.


## Prepared description


VAPORA 1.2 is a six-voice PADsynth instrument for the original OP-1 / Hundekraft
3.0.2, with two detuned playback heads per voice, 14 sound Characters and 15
cinematic presets. Character selects fourteen wave styles; Morph moves them.
Notes make temporary outward ripples. Interval controls an added star field
and signature; Thickness rotates and grows the signature. Motion/Speed control
the sun's white-to-red-orange pulse. Shift toggles a default-hidden readout.
The original star layer, sound engine and installed preset bank are preserved.

The package includes a 480,652-byte `.knl`, corresponding source,
deterministic generators, preset recipes, controls, licenses and validation
records. Native Blackfin build, package/import/relocation checks and sanitizer
host tests pass. The author previously evaluated sounds and selected Solid and
Lines on hardware. The final 1.2 installation and combined performance remain
untested. The public GIF is retained from 1.1.4 for a separate manual update;
it uses a host renderer and substitute font. The static screen is updated.


## Before a maintainer handoff


Confirm the final 1.2 installation and seven-design cycle on the OP-1 and record remaining hardware checks honestly. The SDK also recommends checking sustained chords and heavy effects, save/reload/restart, replacement, removal and reinstall. Do not present unperformed checks as passed.


No maintainer message, issue or pull request has been submitted. The author handles any final submission.


## Manual contribution


The [Manual section in the README](../README.md#manual) is the single maintained manual entry. It covers loading, controls, sound characters, presets and saving, following the Workshop synth-entry format. This is prepared entry content, not an edit to Hundekraft’s website.
