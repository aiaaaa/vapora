# VAPORA

Cinematic pads and spectral textures for the original OP-1, built for Hundekraft.

![Holy Dust: the sun sets and rises above VAPORA’s animated waves](media/holy-dust.gif)

*Holy Dust, with a Brightness sweep. Ten-second looping host preview; the OP-1 supplies the actual menu font. No audio in this GIF.*

## Overview

VAPORA is a six-voice PADsynth-based instrument with two independently phased, detuned playback heads per voice, 14 sound characters, and 15 curated presets. Its screen combines a moving triangular ocean, stars, and six selectable suns. Played notes bend the connecting lines.

**Version 1.1.0** refines the suns. The approved sound engine and preset bank are unchanged.

## Manual

[Open the standalone manual](MANUAL.md)

Two detuned playback heads read broad, evolving PADsynth spectra for each note. Morph between two related colors, add a pitched interval, and let the tone drift. Fourteen characters cover warm ensembles, breathy choirs, hollow flutes, glassy textures and deep drones. VAPORA plays up to six notes, follows velocity, and uses the OP-1’s normal T2 envelope and T3 effect.

**Load:** press **synth**, then **Shift + T1**. Turn **blue** to **vapora** and press **T1**. To load a factory sound, open the preset browser with **Shift + a sound key**, choose **vapora**, then the preset.

![VAPORA instrument page with Holy Dust and the sun fully open](media/vapora-screen.png)

*Holy Dust with Brightness at maximum. Host-rendered illustration; on the OP-1, the readout uses the instrument’s native font.*

### Reading the screen

1. **The water:** blue changes the balance between broad swells and tighter ridges. Green changes the spacing and layered movement of the mesh.
2. **The sun:** white raises it as the sound brightens. Eclipse opens into a full disk near the top and closes as it sinks. Push white for another design.
3. **The stars:** they slow down and dim in daylight, then brighten and drift more actively as the sun drops.
4. **The movement:** orange increases the wave motion. Its secondary Speed setting changes how quickly the water moves, with a restrained visual range.
5. **The connecting lines:** played notes bend the diagonal links between the wave points. Pitch changes that curvature; the underlying swells continue independently.
6. **The readout:** the colored dial and single text row show the control you most recently turned or selected. The dial color matches its knob.

### Controls

| Encoder | Turn | Push, then turn |
| --- | --- | --- |
| **Blue** | **Morph, 0–100%:** blend between the two spectra of the current character. Different positions can sound warmer, airier or more textured. | **Character:** select one of 14 sound families. |
| **Green** | **Thickness, 0–100%:** increase detuning and the contribution of the second playback head. Low settings are more focused; higher settings add beating and fullness. | **Interval:** choose the second head’s pitch relative to the played note. |
| **White** | **Brightness, 0–100%:** open the tone filter and raise the sun. | **Sun:** each push advances to the next design. Turning white still edits Brightness. |
| **Orange** | **Motion, 0–100%:** add slow movement to the sound’s morph and brightness, and increase the ocean’s movement. | **Speed, about 0.02–2 Hz:** set the rate of the internal movement. The screen scales this to a gentler visual rate. |

Push blue, green or orange again to return to its main control. Each has its own selection. After a white push, let the sun change before turning white. The chosen sound settings save with the preset; the selected sun is a visual choice and is not saved with the sound.

**Characters:** Warm, Swarm, Breath, Flute, Pressure, Surf, Porcelain, Sub, Velvet, Cathedral, Nebula, Aurora, Oceanic and Glassveil. Morph explores the two endpoints within a character; use Character for a larger change in sound.

**Intervals:** octave below, fifth below, unison, minor third, major third, fifth, major sixth, minor seventh, major seventh and octave above. Start with unison for a conventional pad; a fifth or octave adds a second pitch to each played note.

**Sun designs:** 05 Eclipse → 01 Halo → 02 Retro → 03 Bands → 10 Saturn → 18 Triple, then back to Eclipse. These numbers identify the designs rather than their positions in the cycle. Sun selection does not change the audio.

### Try this

Load the named factory preset first: it also recalls the character, interval, speed, envelope and effect settings. The four percentages below are approximate knob positions, in blue–green–white–orange order.

| Preset | Blue: Morph | Green: Thickness | White: Brightness | Orange: Motion | Character |
| --- | --- | --- | --- | --- | --- |
| A Fifth Tide | 58% | 85% | 63% | 43% | Warm |
| Holy Dust | 43% | 87% | 89% | 58% | Cathedral |
| Deep Pressure | 17% | 75% | 98% | 12% | Pressure |

**A Fifth Tide:** start with a held note or an open chord. Its fifth interval adds a second pitch; sweep Morph slowly to hear the texture change, then reduce Thickness for a more focused sound.

**Holy Dust:** hold a low note with a fifth above it. Slowly turn white down and back up: the tone darkens and opens while the sun sets and rises. Push orange to try a slower Speed, then push again to return to Motion.

**Deep Pressure:** use widely spaced low notes and let the T2 envelope swell. This preset includes Reverb; install Hundekraft’s separate `reverb` effect to hear the saved treatment. Switch the T3 effect off when comparing the dry texture.

**Tip:** shape the attack and release on **T2**. VAPORA’s internal Motion is separate from the **T4** LFO; use T4 when you want an additional sweep of a main encoder. Reverb and other effects are selected on **T3**.

### Presets and saving

The factory bank contains 15 sounds: A Fifth Tide, Asteroid Swarm, Breath Choir, Cavern Flute, Deep Pressure, Holy Dust, Ice Chapel, Last Light, Moon Choir, Pink Surf, Porcelain Orbit, Stone Hall, Sub Lantern, Tidal Arc and XFIN.

Save an edited sound with the OP-1’s usual sound-key gesture. Deep Pressure is the only factory preset with a saved T3 effect; the other fourteen are dry. If the final bank is already installed, the 1.1 update needs only the plugin file, with no preset copying.

## Download and install

- [VAPORA 1.1 plugin](downloads/vapora.knl)
- [15 factory presets](downloads/vapora-core15-presets.zip) — for new installations
- [Checksums](downloads/SHA256SUMS)

Requires the **original OP-1 with Hundekraft 3.0.2**. This is not an OP-1 field plugin.

Open the [Hundekraft Workshop](https://hundekraft.github.io/op1-workshop/), add `vapora.knl` under **Your plugins**, connect the instrument, and use **Update OP-1**. Follow the Workshop’s transfer/restart instructions.

For a new installation, unzip the preset pack and copy its `synth/vapora` folder into the instrument’s `synth` folder in normal disk mode. Eject and allow synchronization to finish. If you already have the final 15 presets, **update only the plugin; do not recopy the bank**.

Deep Pressure uses the separate Hundekraft `reverb` effect. The other factory presets are dry. The OP-1 supplies the T2 envelope and optional T3 effect.

## Factory sounds

A Fifth Tide · Asteroid Swarm · Breath Choir · Cavern Flute · Deep Pressure · Holy Dust · Ice Chapel · Last Light · Moon Choir · Pink Surf · Porcelain Orbit · Stone Hall · Sub Lantern · Tidal Arc · XFIN

## Build and validation

[Build instructions](docs/BUILD.md) include the pinned SDK, Blackfin toolchain and host checks. All corresponding plugin source, generators, editable geometry and preset recipes are in [plugin/vapora](plugin/vapora).

Native Blackfin compilation, package/import/relocation checks and sanitizer host checks passed. All 15 preset files match the approved bank byte for byte. The author has evaluated the sounds on hardware; the new 1.1 circle rendering still needs its final on-device visual check. The GIF is a host rendering, not an OLED capture. No controlled device CPU benchmark is claimed.

[Release notes](CHANGELOG.md) · [Validation records](validation) · [Hundekraft handoff notes](docs/HUNDEKRAFT.md)

## Credits and license

By **aiaaaa**, developed with AI coding assistance. PADsynth algorithm by Nasca Octavian Paul. Built with Hundekraft’s Kennel SDK, with credit to pcppcp, ultralazr and the contributors named in the retained notices.

GPL-3.0-or-later with the additional OP-1 firmware combination permission: see [LICENSE](LICENSE), [NOTICE](NOTICE), and [third-party credits](THIRD_PARTY_NOTICES.md). No Teenage Engineering firmware, extracted fonts or device keys are included.

Independent project; not affiliated with Teenage Engineering or an official Hundekraft release.
