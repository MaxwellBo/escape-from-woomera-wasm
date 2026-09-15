import './style.css';
import { Xash3D } from 'xash3d-fwgs';
import { unzipSync } from 'fflate';

type XashInstance = InstanceType<typeof Xash3D>;

const canvas = document.getElementById('canvas') as HTMLCanvasElement;
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

function publicAsset(path: string): string {
  return `${import.meta.env.BASE_URL}${path.replace(/^\//, '')}`;
}

const MOD_ZIP_URL = publicAsset('woomera.zip');
const VALVE_ZIP_URL = publicAsset('valve.zip');
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

function formatErr(err: unknown): string {
  if (err instanceof Error) {
    const extra = 'errno' in err ? ` errno=${String((err as { errno?: unknown }).errno)}` : '';
    return `${err.name}: ${err.message}${extra}`;
  }
  if (err && typeof err === 'object') {
    const o = err as { message?: unknown; name?: unknown; status?: unknown };
    const parts = [o.name, o.message, o.status !== undefined ? `status=${String(o.status)}` : '']
      .map((p) => (p === undefined || p === '' ? '' : String(p)))
      .filter(Boolean);
    if (parts.length) return parts.join(' ');
    try {
      return JSON.stringify(err);
    } catch {
      /* ignore */
    }
  }
  return String(err);
}

/** Split a GoldSrc PACK into loose files so we never FS.writeFile a 50MB blob. */
function explodePak(pak: Uint8Array, dest: Map<string, Uint8Array>, prefix: string) {
  const magic = String.fromCharCode(pak[0] ?? 0, pak[1] ?? 0, pak[2] ?? 0, pak[3] ?? 0);
  if (magic !== 'PACK') {
    dest.set(`${prefix}pak0.pak`, pak);
    return;
  }
  const view = new DataView(pak.buffer, pak.byteOffset, pak.byteLength);
  const off = view.getUint32(4, true);
  const length = view.getUint32(8, true);
  for (let i = 0; i < length; i += 64) {
    let name = '';
    for (let j = 0; j < 56; j++) {
      const c = pak[off + i + j];
      if (!c) break;
      name += String.fromCharCode(c);
    }
    name = name.replace(/\\/g, '/');
    if (!name) continue;
    const eoff = view.getUint32(off + i + 56, true);
    const esize = view.getUint32(off + i + 60, true);
    dest.set(`${prefix}${name}`, pak.subarray(eoff, eoff + esize));
  }
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
    let payload = data as Uint8Array;
    if (rel.toLowerCase() === 'liblist.gam') {
      payload = new TextEncoder().encode(patchLibList(new TextDecoder().decode(data)));
    }
    staged.woomera.set(`${GAME_DIR}/${out}`, payload);
    bytes += (data as Uint8Array).length;
  }
  assetsStatus.textContent = `${staged.woomera.size} files (${fmtMB(bytes)}), skipped ${skipped} Win32/unused`;
  markDone('step-assets');
  setVeil('Mod assets ready', 'Loading vendored Half-Life: Uplink data…', 0.5);
  log(`mod staged: ${staged.woomera.size} files, ${fmtMB(bytes)} (skipped ${skipped})`);
}

async function stageValveZip() {
  setVeil('Loading Half-Life data…', 'Fetching Valve’s official Uplink demo (vendored).', 0.52);
  const res = await fetch(VALVE_ZIP_URL);
  if (!res.ok) throw new Error(`valve zip fetch failed: HTTP ${res.status}`);
  const buf = new Uint8Array(await res.arrayBuffer());
  setVeil('Unpacking Half-Life data…', 'Inflating pak0, wads, sprites, sounds.', 0.58);
  await new Promise((r) => setTimeout(r, 30));
  const entries = unzipSync(buf);
  staged.valve.clear();
  let bytes = 0;
  let skipped = 0;
  for (const [name, data] of Object.entries(entries)) {
    if (name.endsWith('/')) continue;
    const rel = name.replace(/\\/g, '/');
    const lower = rel.toLowerCase();
    if (lower.endsWith('.dll') || lower.includes('/dlls/') || lower.includes('/cl_dlls/')) {
      skipped++;
      continue;
    }
    const file = data as Uint8Array;
    if (lower.endsWith('.pak')) {
      explodePak(file, staged.valve, 'valve/');
      bytes += file.length;
      continue;
    }
    staged.valve.set(rel, file);
    bytes += file.length;
  }
  seedDeltaLst();
  const { pak, models } = summarizeValve();
  if (pak === 0 && models === 0) throw new Error('vendored valve.zip has no valve models or pak');
  valveStatus.textContent = `${staged.valve.size} files (${fmtMB(bytes)}) from Uplink demo`;
  markDone('step-valve');
  btnLaunch.disabled = false;
  log(`valve staged: ${staged.valve.size} files, ${fmtMB(bytes)} (skipped ${skipped} Win32)`);
}

function findStaged(files: Map<string, Uint8Array>, suffix: string): Uint8Array | undefined {
  const needle = suffix.toLowerCase().replace(/\\/g, '/');
  for (const [path, data] of files) {
    const p = path.toLowerCase().replace(/\\/g, '/');
    if (p === needle || p.endsWith(`/${needle}`)) return data;
  }
  return undefined;
}

/** Xash loads delta.lst from the gamedir (then valve/). Uplink does not ship it. */
function seedDeltaLst(extra?: Uint8Array) {
  const data =
    extra ??
    findStaged(staged.valve, 'delta.lst') ??
    findStaged(staged.woomera, 'delta.lst');
  if (!data) return false;
  staged.valve.set('valve/delta.lst', data);
  staged.woomera.set(`${GAME_DIR}/delta.lst`, data);
  return true;
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
    'gamedll "dlls/hl.dll"',
    'gamedll1 "dlls/hl.dll"',
    'cldll "1"',
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
  let models = 0;
  for (const [p, d] of staged.valve) {
    bytes += d.length;
    if (/(^|\/)valve\/pak\d+\.pak$/i.test(p)) pak++;
    if (/(^|\/)valve\/models\//i.test(p)) models++;
  }
  return { files: staged.valve.size, bytes, pak, models };
}

async function acceptValveFiles() {
  seedDeltaLst();
  const { files, bytes, pak, models } = summarizeValve();
  if (pak === 0 && models === 0) {
    valveStatus.textContent = `picked ${files} files but no valve/ models or pak — pick the Half-Life root`;
    log(`WARNING: valve intake has ${files} files but no pak0.pak / models; boot will likely fail`);
    btnLaunch.disabled = true;
    return;
  }
  valveStatus.textContent = `${files} files (${fmtMB(bytes)}) incl. ${pak} pak(s), ${models} models — ready`;
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
      /* exists, or we'll fail on write */
    }
  }
}

function writeTree(
  FS: {
    mkdir: (p: string) => void;
    writeFile: (p: string, d: Uint8Array, opts?: { canOwn?: boolean }) => void;
    unlink: (p: string) => void;
  },
  files: Map<string, Uint8Array>,
  onProgress?: (done: number, total: number) => void,
) {
  const entries = [...files.entries()];
  entries.forEach(([path, data], i) => {
    const dest = path.startsWith('/') ? path : `/${path.replace(/^\//, '')}`;
    const slash = dest.lastIndexOf('/');
    if (slash > 0) mkdirTree(FS, dest.slice(0, slash));
    try {
      FS.unlink(dest);
    } catch {
      /* not present */
    }
    try {
      FS.writeFile(dest, data.slice());
    } catch (err) {
      throw new Error(`write ${dest} (${data.length} bytes): ${formatErr(err)}`);
    }
    if (onProgress && i % 50 === 0) onProgress(i, entries.length);
  });
  onProgress?.(entries.length, entries.length);
}

/** Match the WebGL backing store to the on-screen 4:3 box (not window.innerWidth). */
function sizeGameCanvas() {
  const wrap = canvas.parentElement;
  const fallback = { width: canvas.width || 960, height: canvas.height || 720 };
  if (!wrap) return fallback;
  let width = Math.round(wrap.clientWidth);
  let height = Math.round(wrap.clientHeight);
  if (width < 640 || height < 480) {
    width = Math.max(640, width);
    height = Math.round((width * 3) / 4);
  }
  width -= width % 2;
  height -= height % 2;
  canvas.width = width;
  canvas.height = height;
  return { width, height };
}

function inputCaptured() {
  return document.pointerLockElement === canvas;
}

function syncCaptureUi() {
  canvas.parentElement?.classList.toggle('captured', inputCaptured());
  if (!engine) return;
  launchStatus.textContent = inputCaptured()
    ? 'running — mouse captured (Esc to release)'
    : 'running — click the game view to capture mouse and keyboard';
}

async function captureInput() {
  canvas.focus();
  if (inputCaptured()) return;
  try {
    const pending = canvas.requestPointerLock();
    if (pending) await pending;
  } catch (err) {
    log(`pointer lock failed: ${formatErr(err)}`);
  }
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
    const [clientRes, serverRes, extrasRes, deltaRes] = await Promise.all([
      fetch(publicAsset('hlsdk/client.wasm')),
      fetch(publicAsset('hlsdk/server.wasm')),
      fetch(publicAsset('engine/extras.pk3')),
      fetch(publicAsset('hlsdk/delta.lst')),
    ]);
    if (!clientRes.ok || !serverRes.ok) throw new Error('game-logic WASM fetch failed');
    const clientWasm = new Uint8Array(await clientRes.arrayBuffer());
    const serverWasm = new Uint8Array(await serverRes.arrayBuffer());
    const extras = extrasRes.ok ? new Uint8Array(await extrasRes.arrayBuffer()) : null;
    const sdkDelta = deltaRes.ok ? new Uint8Array(await deltaRes.arrayBuffer()) : undefined;
    if (!seedDeltaLst(sdkDelta)) {
      throw new Error('missing valve/delta.lst (HLSDK network table; required to boot)');
    }

    setVeil('Starting engine…', 'Initializing Xash3D WebAssembly runtime.', 0.7);
    const view = sizeGameCanvas();
    log(`boot: creating Xash3D (${view.width}x${view.height})`);
    engine = new Xash3D({
      canvas,
      arguments: [
        '-windowed',
        '-width',
        String(view.width),
        '-height',
        String(view.height),
        '-game',
        GAME_DIR,
      ],
      filesMap: {
        'xash.wasm': publicAsset('engine/xash.wasm'),
        'filesystem_stdio.wasm': publicAsset('engine/filesystem_stdio.wasm'),
        'cl_dlls/menu_emscripten_wasm32.wasm': publicAsset('engine/libmenu.wasm'),
        'libref_webgl2.wasm': publicAsset('engine/libref_webgl2.wasm'),
        'libref_soft.wasm': publicAsset('engine/libref_soft.wasm'),
        'cl_dlls/client_emscripten_wasm32.wasm': publicAsset('hlsdk/client.wasm'),
        'dlls/hl_emscripten_wasm32.wasm': publicAsset('hlsdk/server.wasm'),
      },
      module: {
        print: (text: string) => log(text),
        printErr: (text: string) => log(`ERR: ${text}`),
        // Emscripten only auto-locks the pointer on click when this is set.
        elementPointerLock: true,
      },
    });
    log('boot: init()');
    await engine.init();
    log('boot: init ok');

    const FS = (engine.em as unknown as { FS?: unknown })?.FS as
      | {
          mkdir: (p: string) => void;
          writeFile: (p: string, d: Uint8Array, opts?: { canOwn?: boolean }) => void;
          unlink: (p: string) => void;
        }
      | undefined;
    if (!FS) throw new Error('WASM filesystem unavailable after init');
    log(`boot: writing ${staged.valve.size} valve files + ${staged.woomera.size} woomera files`);

    setVeil('Installing game files…', 'Writing woomera/ + valve/ into WASM memory.', 0.8);
    await new Promise((r) => setTimeout(r, 30));
    writeTree(FS, staged.valve, (done, total) =>
      setVeil('Installing game files…', `valve/ ${done}/${total}`, 0.8 + 0.1 * (done / Math.max(1, total))),
    );
    log('boot: valve files written');
    writeTree(FS, staged.woomera);
    log('boot: woomera files written');
    mkdirTree(FS, `/${GAME_DIR}/cl_dlls`);
    mkdirTree(FS, `/${GAME_DIR}/dlls`);
    mkdirTree(FS, '/valve');
    mkdirTree(FS, '/rwdir');
    FS.writeFile(`/${GAME_DIR}/cl_dlls/client_emscripten_wasm32.wasm`, clientWasm);
    FS.writeFile(`/${GAME_DIR}/dlls/hl_emscripten_wasm32.wasm`, serverWasm);
    if (extras) {
      try {
        FS.writeFile('/valve/extras.pk3', extras);
      } catch {
        /* non-fatal */
      }
    }
    const FSEx = FS as typeof FS & {
      readdir?: (p: string) => string[];
      symlink?: (oldpath: string, newpath: string) => void;
      chdir?: (p: string) => void;
    };
    try {
      FSEx.symlink?.('/woomera', '/rwdir/woomera');
    } catch {
      /* exists */
    }
    try {
      FSEx.symlink?.('/valve', '/rwdir/valve');
    } catch {
      /* exists */
    }
    try {
      FSEx.chdir?.('/rwdir');
    } catch {
      /* ignore */
    }
    log(`boot: root=${(FSEx.readdir?.('/') ?? []).join(' ')}`);
    log(`boot: rwdir=${(FSEx.readdir?.('/rwdir') ?? []).join(' ')}`);
    log(`boot: woomera=${(FSEx.readdir?.('/woomera') ?? []).slice(0, 20).join(' ')}`);
    log('filesystem staged: woomera + valve + WASM game logic');
    setVeil('Running…', 'Main loop starting.', 0.95);
    log('boot: main()');
    engine.main();
    veil.classList.add('hidden');
    mapsPanel.classList.remove('hidden');
    markDone('step-launch');
    launchStatus.textContent = 'running — click the game view to capture mouse and keyboard';
    engineStatus.textContent = 'running';
    log('engine main loop started; auto-loading efw_prototype_level1…');
    canvas.focus();
    engine.Cmd_ExecuteString('in_mouse 1');
    engine.Cmd_ExecuteString('m_rawinput 1');
    setTimeout(() => {
      try {
        engine?.Cmd_ExecuteString('map efw_prototype_level1');
      } catch (err) {
        log(`auto-map failed: ${String(err)}`);
      }
    }, 4000);
  } catch (err) {
    const msg = formatErr(err);
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
canvas.addEventListener('click', () => void captureInput());
canvas.addEventListener('pointerdown', () => canvas.focus());
document.addEventListener('pointerlockchange', syncCaptureUi);
document.addEventListener('pointerlockerror', () => log('pointer lock error'));
consoleInput.addEventListener('keydown', (e) => e.stopPropagation());
consoleInput.addEventListener('keyup', (e) => e.stopPropagation());

mapsPanel.querySelectorAll('button[data-map]').forEach((btn) => {
  btn.addEventListener('click', () => {
    const map = (btn as HTMLButtonElement).dataset.map;
    log(`> map ${map}`);
    engine?.Cmd_ExecuteString(`map ${map}`);
    void captureInput();
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

void (async () => {
  await stageModZip();
  await stageValveZip();
  setVeil('Ready', 'Click the game view after boot to capture mouse and keyboard.', 0.62);
  const params = new URLSearchParams(location.search);
  if (params.get('noboot') === '1') {
    log('noboot: assets ready, waiting for manual launch');
    return;
  }
  log('assets ready — booting Woomera');
  await boot();
})().catch((err: unknown) => {
  const msg = formatErr(err);
  const assetsReady = document.getElementById('step-assets')?.classList.contains('done');
  if (assetsReady) valveStatus.textContent = `failed: ${msg}`;
  else assetsStatus.textContent = `failed: ${msg}`;
  setVeil('Asset load failed', msg, 1);
  log(`STAGE FAILED: ${msg}`);
});
