# Hundekraft handoff — prepared, not submitted


Reviewed 20 October 2026 against the [Workshop repository](https://github.com/Hundekraft/op1-workshop) and the `PLUGIN_GUIDE.md` in the [3.0.2 SDK bundle](https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.2-source.tar.gz).


The SDK guide describes sharing independent plugins; no separate formal submission process was found in the Workshop repository pages reviewed. Catalog inclusion is a maintainer decision.


## Materials requested by the SDK guide


- Compiled `.knl`, corresponding source and build instructions.
- Required core/SDK version, controls and resource claims.
- License and third-party notices, artwork/preset credits and a short changelog.
- Separate reporting of compilation, host/emulator checks and hardware checks.
- Stable unique plugin ID; no firmware images or device keys.


VAPORA supplies these materials. Its stable ID is `vapora`, manifest `order` is 0, and resource claims are empty. It is a standalone synth with no companion plugin or core patches. The saved effects require Reverb for Deep Pressure and Plate for Stone Hall.


## Prepared description


VAPORA 1.2.4 is a six-voice PADsynth instrument for the original OP-1 / Hundekraft
3.0.2, with two detuned playback heads per voice, 14 sound Characters and 15
cinematic presets. Character selects fourteen wave styles; Morph moves them.
Notes make temporary outward ripples; holding extends their fade modestly and slows their return, capped at 1.75 seconds. Interval controls an added star field
and signature; Thickness rotates and grows the signature. Motion/Speed control
the sun's white-to-red-orange pulse. Shift toggles a default-hidden readout.
The original 1.2.3 wave, sound and all presets are preserved. Small stars follow smooth paths with soft fades at 37.5% of larger stars' speed. All signature stars are green, with five new crossed-line designs.

The package includes a 481,756-byte `.knl`, corresponding source,
deterministic generators, preset recipes, controls, licenses and validation
records. Native Blackfin build, package/import/relocation checks and sanitizer
host tests pass. The author previously evaluated sounds and selected Solid and
Lines on hardware. Controlled device CPU/GUI timing and Stone Hall Plate/octave reload verification remain pending. The existing user-selected host-rendered images and GIFs are retained unchanged.


## Before a maintainer handoff


Confirm the final 1.2.4 installation and seven-design cycle on the OP-1 and record remaining hardware checks honestly. The SDK also recommends checking sustained chords and heavy effects, save/reload, replacement, removal and reinstall. Do not present unperformed checks as passed.


No maintainer message, issue or pull request has been submitted. The author handles any final submission.


## Manual contribution


The [Manual section in the README](../README.md#manual) is the single maintained manual entry. It covers loading, controls, sound characters, presets and saving, following the Workshop synth-entry format. This is prepared entry content, not an edit to Hundekraft’s website.
