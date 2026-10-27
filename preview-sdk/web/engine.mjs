/* Shared ABI validation and source-free browser runtime. */
export async function loadEngine(url, expectedHash) {
  const response = await fetch(url);
  if (!response.ok) throw new Error(`Engine download failed (${response.status})`);
  const bytes = await response.arrayBuffer();
  if (expectedHash) {
    const hash = [...new Uint8Array(await crypto.subtle.digest('SHA-256', bytes))].map(x => x.toString(16).padStart(2,'0')).join('');
    if (hash !== expectedHash) throw new Error('Engine and preset manifest do not match. Reload the page.');
  }
  const module = await WebAssembly.compile(bytes);
  if (WebAssembly.Module.imports(module).length) throw new Error('Preview engine must be self-contained');
  return module;
}
export function instantiate(module, kind) {
  const e = new WebAssembly.Instance(module).exports;
  if (e.pv_abi() !== 1 || e.pv_rate() !== 44100 || e.pv_block() !== 128 || e.pv_kind() !== (kind === 'effect' ? 1 : 0)) throw new Error('Unsupported preview ABI');
  e.pv_init();
  return e;
}
export function validatePatch(patch, manifest) {
  if (!patch || patch.schema !== 1 || patch.kind !== 'hundekraft-browser-preview' || patch.plugin !== manifest.id || !Array.isArray(patch.values) || patch.values.length !== manifest.parameters.length || !patch.values.every(v => Number.isFinite(v) && v >= 0 && v <= 1)) throw new Error('This patch does not match this preview.');
  if (!Number.isInteger(patch.octave) || patch.octave < -3 || patch.octave > 3) throw new Error('Invalid octave.');
  if (!Array.isArray(patch.envelope) || patch.envelope.length !== 4 || !patch.envelope.every(Number.isFinite) || patch.envelope.some((v,i) => v < 0 || v > (i===2?1:20))) throw new Error('Invalid audition envelope.');
  return { values: [...patch.values], octave: patch.octave, envelope: [...patch.envelope] };
}
