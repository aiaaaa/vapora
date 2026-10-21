# VAPORA 1.2 device check

Sound and the 15 presets remain frozen. Update only the plugin.

- Push Blue to toggle Morph/Character; verify the 14 retained styles in order.
- Turn Character: the wave style and existing audio Character change together.
  All preset Character words and audio sources remain unchanged.
- Verify the bottom dial/readout starts hidden on fresh load. Shift toggles it
  on persistently, release leaves it on, and the next Shift press hides it.
- Holding Shift must not repeat-toggle. Knob turns must not reveal a hidden footer.
- The visibility choice should survive screen/slot changes during the session.
- Hidden readout: waves must continue below the old cutoff. Its black backing
  must appear with the readout and disappear on the next Shift press.
- A key or chord should make a sharp hit that spreads outward and fades in about
  1.4 seconds, with a small wake rather than repeated elastic bouncing.
- Fast playing should overlap impacts without folded side triangles.
- Held audio should stay settled; returning to the screen should not replay notes.

- Compare with IMG_0112: the green-marked lower joints should sway with the
  rest of the wave much farther, without folding adjacent triangles.
  Bottom joints can travel behind the visible readout and are exposed when hidden.
- The maroon-marked right tips are now 8 pixels farther left. Verify the whole
  pointed outline stays visible and its joined edges remain continuous.

- Blue font is #698eff; Green font and added stars are #00ed95.
  Waves, original stars and dial colors retain their existing palette.

- Confirm 01 Solid is the default after restart.
- Confirm the cycle: 01 Solid, 02 Lines, 03 Halo, 04 Retro, 05 Bands,
  06 Saturn, 07 Triple, then back to Solid.
- The user approved Solid (trial 10) and Lines (trial 21) on hardware;
  this combined build preserves those methods and still needs an installation check.
- Sweep Brightness on both; Lines has seven 4px strokes with 2px gaps.
- Sun rise is 25% shorter (whole-pixel rounding): center y=56 to y=36.
  Lowest position and opening behavior are unchanged; check all seven designs.
- At Motion zero, confirm each sun is white throughout the Speed range.
- Increase Motion: the sun pulses toward orange, reaching the shared red-orange (#ff423f)
  at full depth. Speed changes the pulse period (about 50 seconds to 0.5 seconds).
- Check all seven designs, including the native Solid fill, for consistent tint.
- Compare Motion/Speed text and dial against the fully tinted sun: same red-orange.
- With no notes, Orange and Interval must not change ocean shape or travel speed.
  Morph controls the selected visual mode; notes make temporary trampoline impacts. Thickness and Interval
  must have no direct wave response.
- Step through all ten Intervals: particle sizes/counts and cool/Green tint snap
  immediately in the added overlay. Every step also relocates that layer.
  The original 48 cool stars remain unchanged and are never overpainted.
  Green should match the readout hue; stock knob speed is retained.
- Each Interval has a distinct signature: blue over green stars, green over cool.
- Check a full left-to-right pass and far-left wrap, with no collision jumps.
- Verify the signature passes behind Solid and the other six sun designs.
- Thickness rotates one full turn and grows the signature up to 60%; waves stay fixed.
- Check reduced overlay sizes/counts and unchanged original stars on the OLED, especially occlusion at the sun.
- Character's pointer makes one 270-degree sweep; page presses do not edit sound words.
- Check peak graphics/audio load with the densest overlay, six voices and T3 FX.
- Dial alignment, Brightness height/opening and slower daytime particles remain.
- Preserve all 15 installed sounds; no preset copying or renaming is needed.
- Confirm ordinary playback and screen switching. Audio code/data and presets
  are byte-identical; host checks do not establish hardware CPU timing.

No format, reset, or preset reload is part of this update.
