import './style.css';
import { Xash3D } from 'xash3d-fwgs';
import { unzipSync } from 'fflate';

type XashInstance = InstanceType<typeof Xash3D>;

const canvas = document.getElementById('game') as HTMLCanvasElement;
const veil = document.getElementById('veil') as HTMLDivElement;
const veilTitle = document.getElementById('veil-title') as HTMLHeadingElement;
const veilSub = document.getElementById('veil-sub') as HTMLParagraphElement;
const veilFill = document.getElementById('veil-fill') as HTMLDivElement;
const assetsStatus = document.getElementById('assets-status') as HTMLSpanElement;
const valveStatus = document.getElementById('valve-status') as HTMLSpanElement;
const launchStatus = document.getElementById('launch-status') as HTMLSpanElement;
const engineStatus = document.getElementById('engine-status') as HTMLElement;
const btnFolder = document.getElementById('btn-folder') as HTMLButtonElement;
const btnLaunch = document.getElementById('btn-launch') as HTMLButtonElement;
const dirInput = document.getElementById('dir-input') as HTMLInputElement;
const mapsPanel = document.getElementById('maps-panel') as HTMLDivElement;
const consoleForm = document.getElementById('console-form') as HTMLFormElement;
const consoleInput = document.getElementById('console-input') as HTMLInputElement;
const logEl = document.getElementById('log') as HTMLPreElement;
const logCount = document.getElementById('log-count') as HTMLSpanElement;

const MOD_ZIP_URL = '/woomera.zip';
const MOD_ZIP_ROOT = 'EscapeFromWoomera_v084/';
const GAME_DIR = 'woomera';
// Win32-only binaries and unused bulk: never staged into the WASM filesystem.
const SKIP_PREFIXES = [
  'cl_dlls/',
  'dlls/',
  'SAVE/',
  'Storyboard/',
];
const SKIP_SUFFIXES = ['.dll'];

const staged = {
  woomera: new Map<string, Uint8Array>(),
  valve: new Map<string, Uint8Array>(),
};
let engine: XashInstance | null = null;
let logLines = 0;

function log(text: string) {
  const normalized = String(text).replace(/\s+$/, '');
  if (!normalized) return;
  logLines++;
  logCount.textContent = String(logLines);
  logEl.textContent += normalized + '\n';
  if (logLines > 500) {
    const lines = logEl.textContent.split('\n');
    logEl.textContent = lines.slice(lines.length - 500).join('\n');
  }
  logEl.scrollTop = logEl.scrollHeight;
}

function setVeil(title: string, sub: string, frac: number) {
  veilTitle.textContent = title;
  veilSub.textContent = sub;
  veilFill.style.width = `${Math.max(2, Math.min(100, frac * 100))}%`;
}

function markDone(id: string) {
  document.getElementById(id)?.classList.add('done');
}

function fmtMB(bytes: number) {
  return `${(bytes / 1048576).toFixed(1)} MB`;
}

// ---- staged-file helpers -------------------------------------------------

function shouldSkip(rel: string) {
  const lower = rel.toLowerCase();
  if (SKIP_SUFFIXES.some((s) => lower.endsWith(s))) return true;
  return SKIP_PREFIXES.some((p) => lower.startsWith(p.toLowerCase()));
}

async function stageModZip() {
  setVeil('Loading mod assets…', 'Fetching EscapeFromWoomera v0.84 (vendored).', 0.05);
  const res = await fetch(MOD_ZIP_URL);
  if (!res.ok) throw new Error(`mod zip fetch failed: HTTP ${res.status}`);
  const buf = new Uint8Array(await res.arrayBuffer());
  setVeil('Unpacking mod assets…', 'Inflating maps, models, textures, sounds.', 0.3);
  await new Promise((r) => setTimeout(r, 30));
  const entries = unzipSync(buf);
  let bytes = 0;
  let skipped = 0;
  for (const [name, data] of Object.entries(entries)) {
    if (name.endsWith('/')) continue;
    if (!name.startsWith(MOD_ZIP_ROOT)) continue;
    const rel = name.slice(MOD_ZIP_ROOT.length);
    if (!rel || shouldSkip(rel)) {
      skipped++;
      continue;
    }
    let out = rel;
    if (rel.toLowerCase() === 'liblist.gam') out = patchLibList(new TextDecoder().decode(data));
    staged.woomera.set(`${GAME_DIR}/${out}`, data as Uint8Array);
    bytes += (data as Uint8Array).length;
  }
  assetsStatus.textContent = `${staged.woomera.size} files (${fmtMB(bytes)}), skipped ${skipped} Win32/unused`;
  markDone('step-assets');
  setVeil('Mod assets ready', 'Now point step 2 at your Half-Life install.', 0.5);
  log(`mod staged: ${staged.woomera.size} files, ${fmtMB(bytes)} (skipped ${skipped})`);
}

/** Point the mod at WASM game-logic names instead of the Win32 DLLs. */
function patchLibList(original: string): string {
  void original;
  return [
    'game "Escape from Woomera"',
    'url_info "http://escapefromwoomera.org"',
    'version "0.84 (Beta 2)"',
    'size "800000"',
    'hlversion "1108"',
    'type "Singleplayer"',
    'startmap "efw_prototype_level1"',
    'trainingmap "efw_prototype_level1"',
    'mpentity "info_player_deathmatch"',
    'gamedll "dlls/hl_emscripten_wasm32.wasm"',
    'gamedll1 "dlls/hl_emscripten_wasm32.wasm"',
    'cldll "cl_dlls/client_emscripten_wasm32.wasm"',
    '',
  ].join('\n');
}

// ---- Half-Life folder intake ----------------------------------------------

async function collectDirectory(handle: unknown, base: string, out: Map<string, Uint8Array>) {
  const dir = handle as { values: () => AsyncIterable<{ kind: string; name: string; getFile?: () => Promise<File> }> };
  for await (const entry of dir.values()) {
    const path = base ? `${base}/${entry.name}` : entry.name;
    if (entry.kind === 'file') {
      const file = await entry.getFile!();
      out.set(path, new Uint8Array(await file.arrayBuffer()));
    } else if (entry.kind === 'directory') {
      await collectDirectory(entry, path, out);
    }
  }
}

function summarizeValve() {
  let bytes = 0;
  let pak = 0;
  for (const [p, d] of staged.valve) {
    bytes += d.length;
    if (/(^|\/)valve\/pak\d+\.pak$/i.test(p)) pak++;
  }
  return { files: staged.valve.size, bytes, pak };
}

async function acceptValveFiles() {
  const { files, bytes, pak } = summarizeValve();
  if (pak === 0) {
    valveStatus.textContent = `picked ${files} files but no valve/pak*.pak found — pick the Half-Life root`;
    log(`WARNING: valve intake has ${files} files but no pak0.pak; boot will likely fail`);
    btnLaunch.disabled = true;
    return;
  }
  valveStatus.textContent = `${files} files (${fmtMB(bytes)}) incl. ${pak} pak(s) — ready`;
  markDone('step-valve');
  btnLaunch.disabled = false;
  log(`valve staged: ${files} files, ${fmtMB(bytes)}, ${pak} pak(s)`);
}

async function pickFolderNative() {
  const picker = (window as unknown as { showDirectoryPicker?: () => Promise<unknown> }).showDirectoryPicker;
  if (!picker) {
    dirInput.click();
    return;
  }
  try {
    const root = await picker.call(window);
    staged.valve.clear();
    await collectDirectory(root, '', staged.valve);
    await acceptValveFiles();
  } catch (err) {
    if ((err as DOMException)?.name !== 'AbortError') log(`folder pick failed: ${String(err)}`);
  }
}

function pickFolderFallback(files: FileList | null) {
  if (!files || files.length === 0) return;
  staged.valve.clear();
  const jobs: Promise<void>[] = [];
  for (const f of Array.from(files)) {
    const rel = (f as File & { webkitRelativePath?: string }).webkitRelativePath || f.name;
    jobs.push(
      f.arrayBuffer().then((buf) => {
        staged.valve.set(rel, new Uint8Array(buf));
      }),
    );
  }
  void Promise.all(jobs).then(() => acceptValveFiles());
}

// ---- WASM filesystem -------------------------------------------------------

function mkdirTree(FS: { mkdir: (p: string) => void }, path: string) {
  const parts = path.split('/').filter(Boolean);
  let cur = '';
  for (const part of parts) {
    cur += `/${part}`;
    try {
      FS.mkdir(cur);
    } catch {
      /* already exists */
    }
  }
}

function writeTree(
  FS: { mkdir: (p: string) => void; writeFile: (p: string, d: Uint8Array) => void; unlink: (p: string) => void },
  files: Map<string, Uint8Array>,
  onProgress?: (done: number, total: number) => void,
) {
  const entries = [...files.entries()];
  entries.forEach(([path, data], i) => {
    const slash = path.lastIndexOf('/');
    if (slash > 0) mkdirTree(FS, path.slice(0, slash));
    try {
      FS.unlink(path);
    } catch {
      /* not present */
    }
    FS.writeFile(path, data);
    if (onProgress && i % 50 === 0) onProgress(i, entries.length);
  });
  onProgress?.(entries.length, entries.length);
}

// ---- boot ------------------------------------------------------------------

async function boot() {
  if (engine || btnLaunch.disabled) return;
  btnLaunch.disabled = true;
  btnFolder.disabled = true;
  launchStatus.textContent = 'starting engine…';
  engineStatus.textContent = 'initializing WASM';
  try {
    setVeil('Fetching game logic…', 'WASM client/server + engine extras.', 0.55);
    const [clientRes, serverRes, extrasRes] = await Promise.all([
      fetch('/hlsdk/client.wasm'),
      fetch('/hlsdk/server.wasm'),
      fetch('/engine/extras.pk3'),
    ]);
    if (!clientRes.ok || !serverRes.ok) throw new Error('game-logic WASM fetch failed');
    const clientWasm = new Uint8Array(await clientRes.arrayBuffer());
    const serverWasm = new Uint8Array(await serverRes.arrayBuffer());
    const extras = extrasRes.ok ? new Uint8Array(await extrasRes.arrayBuffer()) : null;

    setVeil('Starting engine…', 'Initializing Xash3D WebAssembly runtime.', 0.7);
    engine = new Xash3D({
      canvas,
      arguments: ['-game', GAME_DIR],
      filesMap: {
        'xash.wasm': '/engine/xash.wasm',
        'filesystem_stdio.wasm': '/engine/filesystem_stdio.wasm',
        'cl_dlls/menu_emscripten_wasm32.wasm': '/engine/libmenu.wasm',
        'libref_webgl2.wasm': '/engine/libref_webgl2.wasm',
        'libref_soft.wasm': '/engine/libref_soft.wasm',
        'cl_dlls/client_emscripten_wasm32.wasm': '/hlsdk/client.wasm',
        'dlls/hl_emscripten_wasm32.wasm': '/hlsdk/server.wasm',
      },
      module: {
        print: (text: string) => log(text),
        printErr: (text: string) => log(`ERR: ${text}`),
      },
    });
    await engine.init();

    const FS = (engine.em as unknown as { FS?: unknown })?.FS as
      | { mkdir: (p: string) => void; writeFile: (p: string, d: Uint8Array) => void; unlink: (p: string) => void }
      | undefined;
    if (!FS) throw new Error('WASM filesystem unavailable after init');

    setVeil('Installing game files…', 'Writing woomera/ + valve/ into WASM memory.', 0.8);
    await new Promise((r) => setTimeout(r, 30));
    writeTree(FS, staged.valve, (done, total) =>
      setVeil('Installing game files…', `valve/ ${done}/${total}`, 0.8 + 0.1 * (done / Math.max(1, total))),
    );
    writeTree(FS, staged.woomera);
    FS.writeFile(`${GAME_DIR}/cl_dlls/client_emscripten_wasm32.wasm`, clientWasm);
    FS.writeFile(`${GAME_DIR}/dlls/hl_emscripten_wasm32.wasm`, serverWasm);
    if (extras) {
      try {
        FS.writeFile('valve/extras.pk3', extras);
      } catch {
        /* non-fatal */
      }
    }
    log('filesystem staged: woomera + valve + WASM game logic');

    setVeil('Running…', 'Main loop starting.', 0.95);
    engine.main();
    veil.classList.add('hidden');
    mapsPanel.classList.remove('hidden');
    markDone('step-launch');
    launchStatus.textContent = 'running — click the game view to capture input';
    engineStatus.textContent = 'running';
    log('engine main loop started; auto-loading efw_prototype_level1…');
    setTimeout(() => {
      try {
        engine?.Cmd_ExecuteString('map efw_prototype_level1');
      } catch (err) {
        log(`auto-map failed: ${String(err)}`);
      }
    }, 4000);
  } catch (err) {
    const msg = err instanceof Error ? err.message : String(err);
    launchStatus.textContent = `failed: ${msg}`;
    engineStatus.textContent = 'failed';
    setVeil('Boot failed', msg, 1);
    log(`BOOT FAILED: ${msg}`);
    engine = null;
    btnLaunch.disabled = false;
    btnFolder.disabled = false;
  }
}

// ---- wiring -----------------------------------------------------------------

btnFolder.addEventListener('click', () => void pickFolderNative());
dirInput.addEventListener('change', () => pickFolderFallback(dirInput.files));
btnLaunch.addEventListener('click', () => void boot());

mapsPanel.querySelectorAll('button[data-map]').forEach((btn) => {
  btn.addEventListener('click', () => {
    const map = (btn as HTMLButtonElement).dataset.map;
    log(`> map ${map}`);
    engine?.Cmd_ExecuteString(`map ${map}`);
    canvas.focus();
  });
});

consoleForm.addEventListener('submit', (e) => {
  e.preventDefault();
  const cmd = consoleInput.value.trim();
  if (!cmd) return;
  log(`> ${cmd}`);
  engine?.Cmd_ExecuteString(cmd);
  consoleInput.value = '';
});

document.getElementById('btn-about')?.addEventListener('click', () => {
  document.getElementById('about')?.classList.remove('hidden');
});
document.getElementById('btn-close-about')?.addEventListener('click', () => {
  document.getElementById('about')?.classList.add('hidden');
});
document.getElementById('btn-help')?.addEventListener('click', () => {
  document.getElementById('help')?.classList.remove('hidden');
});
document.getElementById('btn-close-help')?.addEventListener('click', () => {
  document.getElementById('help')?.classList.add('hidden');
});

void stageModZip()
  .then(() => {
    if (staged.valve.size > 0) void acceptValveFiles();
    // Diagnostics: ?selftest=1 boots the engine with no retail data to prove
    // the WASM runtime, FS staging and log plumbing work. It is expected to
    // fail when the engine can't find valve/pak0.pak — the log shows how far
    // it got.
    if (new URLSearchParams(location.search).get('selftest') === '1') {
      log('selftest: booting without retail data (expected to fail at valve/pak0.pak)');
      btnLaunch.disabled = false;
      void boot();
    }
  })
  .catch((err: unknown) => {
    const msg = err instanceof Error ? err.message : String(err);
    assetsStatus.textContent = `failed: ${msg}`;
    setVeil('Asset load failed', msg, 1);
    log(`MOD STAGE FAILED: ${msg}`);
  });
