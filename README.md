
# VAPORA

Cinematic pads and spectral textures for the original OP-1, built for Hundekraft.

**A reworking of [PADsynth](https://zynaddsubfx.sourceforge.io/doc/PADsynth/PADsynth.htm), the synthesis algorithm by Nasca Octavian Paul, for the original OP-1.**

<img width="320" height="160" alt="27-red-eclipse-rain" src="https://github.com/user-attachments/assets/36889e82-f60a-476f-bbf2-0d537a1db341" />

-

<img width="320" height="160" alt="22-slow-red-ripples" src="https://github.com/user-attachments/assets/de574889-9111-43bc-acd0-af45425d0160" />

<img width="320" height="160" alt="19-chord-ripples-eclipse-rise" src="https://github.com/user-attachments/assets/485b07e4-f327-4b0e-aeaa-639e7189a1b8" />

-

<img width="320" height="160" alt="32-white-disc-dunes" src="https://github.com/user-attachments/assets/2af2fa20-2f4f-43d8-9e24-77a9af5fd611" />

<img width="320" height="160" alt="21-slow-red-original" src="https://github.com/user-attachments/assets/6113adbe-6549-4903-9981-4b9c45344a5e" />




## Overview

VAPORA is a six-voice PADsynth-based instrument with two independently phased, detuned playback heads per voice, 14 sound characters, and 15 curated presets. Its screen combines a moving triangular perlin space, stars, and a setting sun.

Two detuned playback heads read broad, evolving PADsynth spectra for each note. Morph between two related colors, add a pitched interval, and let the tone drift. Fourteen characters cover warm ensembles, breathy choirs, hollow flutes, glassy textures and deep drones. VAPORA plays up to six notes, follows velocity, and uses the OP-1’s normal T2 envelope and T3 effect.

**Version 1.2.2** gives the waves, stars and sun distinct visual controls, with spreading note ripples and adds Shift-toggle readout.

## Manual

**Shift** button will toggle a on screen active variable readout, can be helpful at first TRY IT!

**Load:** press **synth**, then **Shift + T1**. Turn **blue** to **vapora** and press **T1**. To load a factory sound, open the preset browser with **Shift + a sound key**, choose **vapora**, then the preset.

### Controls

| Encoder | Turn | Push, then turn |
| --- | --- | --- |
| **Blue** | **Morph, 0–100%:** blend between the two spectra of the current character. Different positions can sound warmer, airier or more textured. Morph also adjusts wave movement. | **Character:** select one of 14 sound families, each with its own wave style. |
| **Green** | **Thickness, 0–100%:** increase detuning and the contribution of the second playback head. Low settings are more focused; higher settings add beating and fullness. The signature star rotates 360° and grows up to 60%. | **Interval:** choose the second head’s pitch relative to the played note, plus its added star field and signature. |
| **White** | **Brightness, 0–100%:** open the tone filter and raise the sun. | **Sun:** each push advances to the next design. Turning white still edits Brightness. |
| **Orange** | **Motion, 0–100%:** add slow movement to the sound’s morph, detune and brightness; set how far the sun pulses from white toward red-orange. | **Speed, about 0.02–2 Hz:** set the rate of the internal movement and the sun’s pulse. |

Push blue, green or orange again to return to its main control. Each has its own selection. White does not have a push then turn setting due to current SDK setup.

**Characters:** Warm, Swarm, Breath, Flute, Pressure, Surf, Porcelain, Sub, Velvet, Cathedral, Nebula, Aurora, Oceanic and Glassveil. Morph explores the two endpoints within a character; use Character for a larger change in sound.

**Shift:** toggle the bottom control dial/readout on and off. It starts hidden and stays in the chosen state after release. The black backing appears only when shown; waves continue into that area when hidden. The choice lasts for the session, not across a fresh plugin load or in saved sounds.

**Intervals:** octave below, fifth below, unison, minor third, major third, fifth, major sixth, minor seventh, major seventh and octave above. Start with unison for a conventional pad; a fifth or octave adds a second pitch to each played note.

### Try this

Load STONE HALL and play it at -3 octave range.

### Presets and saving

The factory bank contains 15 sounds: A Fifth Tide, Asteroid Swarm, Breath Choir, Cavern Flute, Deep Pressure, Holy Dust, Ice Chapel, Last Light, Moon Choir, Pink Surf, Porcelain Orbit, Stone Hall, Sub Lantern, Tidal Arc and XFIN.

Save an edited sound with the OP-1’s usual sound-key gesture. Deep Pressure is the only factory preset with a saved T3 effect; the other fourteen are dry. If the final bank is already installed, the 1.2.2 update needs only the plugin file, with no preset copying.

## Download and install

- [VAPORA 1.2.2 plugin](downloads/vapora.knl)
- [15 factory presets](downloads/vapora-core15-presets.zip) — for new installations
- [Checksums](downloads/SHA256SUMS)

Requires the **original OP-1 with Hundekraft 3.0.2**. This is not an OP-1 field plugin.

Open the [Hundekraft Workshop](https://hundekraft.github.io/op1-workshop/), add `vapora.knl` under **Your plugins**, connect the instrument, and use **Update OP-1**. Follow the Workshop’s transfer/restart instructions.

For a new installation, unzip the preset pack and copy its `synth/vapora` folder into the instrument’s `synth` folder in normal disk mode. Eject and allow synchronization to finish. If you already have the final 15 presets, **update only the plugin; do not recopy the bank**.

Deep Pressure uses the separate Hundekraft `reverb` effect. The other factory presets are dry. The OP-1 supplies the T2 envelope and optional T3 effect.

## Build and validation

[Build instructions](docs/BUILD.md) include the pinned SDK, Blackfin toolchain and host checks. All corresponding plugin source, generators, editable geometry and preset recipes are in [plugin/vapora](plugin/vapora).

Native Blackfin compilation, package/import/relocation checks and sanitizer host checks passed. Audio sources, tables, saved parameter mappings and all 15 preset files are unchanged from 1.1.4; all 19 dry audio fixtures match the local 1.1.15 revision exactly. Geometry, Shift behavior, spreading impacts, stars and all seven suns are checked on the host. The author previously evaluated the sounds and selected Solid and Lines on hardware; the final 1.2.2 installation and combined graphics/audio performance still need a device check. No controlled device CPU benchmark is claimed.

The existing GIF is retained from 1.1.4 for a separate manual update. It is a host rendering, not an OLED capture, and does not show the new 1.2.2 behavior.

[Release notes](CHANGELOG.md) · [Validation records](validation) · [Hundekraft handoff notes](docs/HUNDEKRAFT.md)

## Credits and license

By **AiAAAA**, developed with AI coding assistance. PADsynth algorithm by Nasca Octavian Paul. Built with Hundekraft’s Kennel SDK, with credit to pcppcp, ultralazr and the contributors named in the retained notices.

GPL-3.0-or-later with the additional OP-1 firmware combination permission: see [LICENSE](LICENSE), [NOTICE](NOTICE), and [third-party credits](THIRD_PARTY_NOTICES.md). No Teenage Engineering firmware, extracted fonts or device keys are included.

Independent project; not affiliated with Teenage Engineering.
