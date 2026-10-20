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


VAPORA is a six-voice PADsynth-based instrument for the original OP-1 / Hundekraft 3.0.2, with two detuned playback heads per voice, 14 sound characters and 15 curated cinematic presets. Its procedural screen combines a triangular ocean, stars and six sun designs, with note-dependent connector curvature. Version 1.1 refines sun geometry while preserving the approved sound and presets.


The package includes a 473,392-byte `.knl`, corresponding source, deterministic generators, preset recipes, controls, licenses and validation records. Blackfin compilation, package/import/relocation checks and sanitizer host tests passed. The sounds have been evaluated by the author on hardware; final native visual confirmation for the 1.1 sun changes remains pending. The preview GIF uses a host renderer and substitute font.


## Before a maintainer handoff


Confirm the final 1.1 sun rendering on the OP-1 and record remaining hardware checks honestly. The SDK also recommends checking sustained chords and heavy effects, save/reload/restart, replacement, removal and reinstall. Do not present unperformed checks as passed.


No maintainer message, issue or pull request has been submitted. The author handles any final submission.


## Manual contribution


The [Manual section in the README](../README.md#manual) is the single maintained manual entry. It covers loading, controls, sound characters, presets and saving, following the Workshop synth-entry format. This is prepared entry content, not an edit to Hundekraft’s website.
