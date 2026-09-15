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
const btnLaunch = document.getElementById('btn-launch') as HTMLButtonElement;
const mapsPanel = document.getElementById('maps-panel') as HTMLDivElement;
const consoleForm = document.getElementById('console-form') as HTMLFormElement;
const consoleInput = document.getElementById('console-input') as HTMLInputElement;
const logEl = document.getElementById('log') as HTMLPreElement;
const logCount = document.getElementById('log-count') as HTMLSpanElement;

function publicAsset(path: string): string {
  const url = `${import.meta.env.BASE_URL}${path.replace(/^\//, '')}`;
  if (/\.wasm$/i.test(path))
    return `${url}?v=efw-decomp1`;
  return url;
}

const MOD_ZIP_URL = publicAsset('woomera.zip');
const VALVE_ZIP_URL = publicAsset('valve.zip');
const MOD_ZIP_ROOT = 'EscapeFromWoomera_v084/';
const GAME_DIR = 'woomera';
// Win32-only binaries: never staged into the WASM filesystem.
const SKIP_PREFIXES = [
  'cl_dlls/',
  'dlls/',
  'SAVE/',
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

function pressGameKey(key: string, keyCode: number) {
  canvas.focus();
  const code = key === 'e' ? 'KeyE'
    : key === 'i' ? 'KeyI'
    : /^\d$/.test(key) ? `Digit${key}`
    : `Key${key.toUpperCase()}`;
  const fire = (type: string) => {
    const ev = new KeyboardEvent(type, {
      key,
      code,
      keyCode,
      which: keyCode,
      bubbles: true,
      cancelable: true,
    });
    canvas.dispatchEvent(ev);
    window.dispatchEvent(ev);
    document.dispatchEvent(ev);
  };
  fire('keydown');
  setTimeout(() => fire('keyup'), 120);
}

let pickSeq = 0;

function pokeGameInput(slot: number) {
  pickSeq += 1;
  const line = `${pickSeq} ${slot}`;
  log(`> poke ${line}`);
  const FS = (engine?.em as unknown as {
    FS?: { writeFile: (p: string, d: Uint8Array | string) => void };
  })?.FS;
  if (FS?.writeFile) {
    for (const p of [`/${GAME_DIR}/efw_cmd.txt`, `/rwdir/${GAME_DIR}/efw_cmd.txt`]) {
      try {
        FS.writeFile(p, line);
      } catch (err) {
        log(`fs write ${p} failed: ${formatErr(err)}`);
      }
    }
  } else {
    log('poke: WASM FS unavailable');
  }
  runEngineCmd('pausable 0');
  runEngineCmd(`efw_js_pick ${slot}`);
  runEngineCmd(`efw_pick ${slot}`);
  void captureInput();
}

function chooseTalkSlot(slot: number) {
  log(`> talk choice ${slot}`);
  pokeGameInput(slot);
}

function runEngineCmd(cmd: string) {
  if (!engine) return;
  const withNl = cmd.endsWith('\n') ? cmd : `${cmd}\n`;
  try {
    engine.Cmd_ExecuteString(withNl);
  } catch (err) {
    log(`cmd failed (${cmd}): ${formatErr(err)}`);
  }
}

/** Host console, ClientCommand, and listen-server `cmd` forwarding. */
function runGameCmd(cmd: string) {
  const trimmed = cmd.trim();
  if (!trimmed) return;
  runEngineCmd(trimmed);
  if (!/^cmd\s/i.test(trimmed) && /^(efw_|menuselect\b)/i.test(trimmed))
    runEngineCmd(`cmd ${trimmed}`);
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

/**
 * Root cause of the clipped menu:
 *
 * Xash MainUI is authored at 1024×768. When the drawable is ≥ 4:3 it scales
 * with ScreenHeight/768, so the 1024-wide layout only fits if
 * ScreenWidth >= ScreenHeight * 4/3.
 *
 * SDL2's Emscripten backend does not use the <canvas> box. It treats the
 * *browser window* as the drawable:
 *   - wasm imports `window.innerWidth` / `window.innerHeight`
 *   - `emscripten_get_screen_size` writes `screen.width` / `screen.height`
 *   - it then sizes the canvas backing store (and often inline CSS
 *     `width/height: Npx !important`) to that window/screen size
 *
 * Our canvas lives in a smaller 4:3 `.canvas-wrap` with `overflow: hidden`.
 * If the GL backing store is 960×720 while MainUI rasterises for ~1280×window,
 * WebGL clips to the backing store. GL's origin is bottom-left, so the top of
 * the menu and the orange QMF_NOTIFY hints on the right are what disappear.
 *
 * Fix: report the wrap's pixel size from every metric SDL reads, CSS-lock the
 * canvas to the wrap, and never shrink the backing store below what the
 * engine actually allocated (that would clip in GL, which CSS cannot undo).
 */
type ViewSize = { width: number; height: number };

function protoGetter<T extends object>(obj: T, prop: string): (() => number) | undefined {
  const desc =
    Object.getOwnPropertyDescriptor(obj, prop) ??
    Object.getOwnPropertyDescriptor(Object.getPrototypeOf(obj), prop);
  const get = desc?.get;
  return get ? () => Number(get.call(obj)) : undefined;
}

const nativeInnerWidth = protoGetter(window, 'innerWidth');
const nativeInnerHeight = protoGetter(window, 'innerHeight');
const nativeScreenWidth = protoGetter(screen, 'width');
const nativeScreenHeight = protoGetter(screen, 'height');

function nativeWindowSize(): ViewSize {
  return {
    width: nativeInnerWidth?.() ?? window.innerWidth,
    height: nativeInnerHeight?.() ?? window.innerHeight,
  };
}

function nativeScreenSize(): ViewSize {
  return {
    width: nativeScreenWidth?.() ?? screen.width,
    height: nativeScreenHeight?.() ?? screen.height,
  };
}

function viewBox(): ViewSize {
  const wrap = canvas.parentElement;
  const fallback = { width: 960, height: 720 };
  if (!wrap) return fallback;
  let width = Math.round(wrap.clientWidth);
  let height = Math.round(wrap.clientHeight);
  if (width < 640 || height < 480) {
    width = Math.max(640, width);
    height = Math.round((width * 3) / 4);
  }
  width += width % 2;
  height -= height % 2;
  // MainUI needs width >= height * 4/3. Rounding the wrap can make it a
  // pixel too narrow and clip the hint column.
  if (width * 3 < height * 4) width += 2;
  return { width, height };
}

function sizeGameCanvas() {
  const { width, height } = viewBox();
  if (canvas.width !== width) canvas.width = width;
  if (canvas.height !== height) canvas.height = height;
  return { width, height };
}

function presentCanvasInWrap() {
  // Inline `width: 1280px !important` from Emscripten beats stylesheet
  // `width: 100% !important` and overflows `.canvas-wrap`. Rewrite it every
  // time SDL touches the style attribute.
  if (canvas.style.getPropertyValue('width') !== '100%' || canvas.style.getPropertyPriority('width') !== 'important') {
    canvas.style.setProperty('width', '100%', 'important');
    canvas.style.setProperty('height', '100%', 'important');
  }
}

let canvasCssLocked = false;
function lockCanvasCssToWrap() {
  if (canvasCssLocked) return;
  canvasCssLocked = true;
  presentCanvasInWrap();
  new MutationObserver(presentCanvasInWrap).observe(canvas, {
    attributes: true,
    attributeFilter: ['style'],
  });
}

function logViewMetrics(tag: string) {
  const wrap = canvas.parentElement;
  const r = canvas.getBoundingClientRect();
  const native = nativeWindowSize();
  log(
    `${tag}: native=${native.width}x${native.height} inner=${window.innerWidth}x${window.innerHeight} ` +
      `screen=${screen.width}x${screen.height} (nativeScreen=${nativeScreenSize().width}x${nativeScreenSize().height}) ` +
      `wrap=${wrap?.clientWidth ?? 0}x${wrap?.clientHeight ?? 0} ` +
      `attr=${canvas.width}x${canvas.height} css=${Math.round(r.width)}x${Math.round(r.height)} ` +
      `style=${canvas.style.width || '-'}x${canvas.style.height || '-'}`,
  );
}

function defineMetric(obj: object, prop: string, getter: () => number) {
  try {
    Object.defineProperty(obj, prop, {
      configurable: true,
      enumerable: true,
      get: getter,
    });
    return true;
  } catch (err) {
    log(`size-shim ${prop} failed: ${formatErr(err)}`);
    return false;
  }
}

let windowSizePinned = false;
let drawableShimOk = false;
function pinWindowSizeToView() {
  if (windowSizePinned) return drawableShimOk;
  windowSizePinned = true;
  const widthOf = () => viewBox().width;
  const heightOf = () => viewBox().height;
  // innerWidth is what the SDL wasm imports read. screen.* is used by
  // emscripten_get_screen_size — best-effort, some browsers lock it.
  const innerOk =
    defineMetric(window, 'innerWidth', widthOf) && defineMetric(window, 'innerHeight', heightOf);
  defineMetric(window, 'outerWidth', widthOf);
  defineMetric(window, 'outerHeight', heightOf);
  defineMetric(screen, 'width', widthOf);
  defineMetric(screen, 'height', heightOf);
  defineMetric(screen, 'availWidth', widthOf);
  defineMetric(screen, 'availHeight', heightOf);
  const target = viewBox();
  drawableShimOk = innerOk && window.innerWidth === target.width && window.innerHeight === target.height;
  if (window.visualViewport) {
    defineMetric(window.visualViewport, 'width', widthOf);
    defineMetric(window.visualViewport, 'height', heightOf);
  }
  log(
    `size-shim: target=${target.width}x${target.height} inner=${window.innerWidth}x${window.innerHeight} ` +
      `native=${nativeWindowSize().width}x${nativeWindowSize().height} ok=${drawableShimOk}`,
  );
  return drawableShimOk;
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
    lockCanvasCssToWrap();
    const shimOk = pinWindowSizeToView();
    const view = viewBox();
    // Only stamp the backing store to the wrap when SDL will agree. If the
    // size shim failed, leaving canvas.width at the wrap would GL-clip the
    // window-sized menu (hints on the right, title at the top).
    if (shimOk) sizeGameCanvas();
    log(`boot: creating Xash3D (${view.width}x${view.height}) shim=${shimOk}`);
    logViewMetrics('pre-init');
    engine = new Xash3D({
      canvas,
      arguments: shimOk
        ? [
            '-windowed',
            '-width',
            String(view.width),
            '-height',
            String(view.height),
            '-game',
            GAME_DIR,
            '+mp_allowmonsters',
            '1',
            '+deathmatch',
            '0',
            '+pausable',
            '0',
            '+map',
            'efw_prototype_level1',
          ]
        : [
            '-windowed',
            '-game',
            GAME_DIR,
            '+mp_allowmonsters',
            '1',
            '+deathmatch',
            '0',
            '+pausable',
            '0',
            '+map',
            'efw_prototype_level1',
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
    logViewMetrics('pre-main');
    engine.main();
    presentCanvasInWrap();
    setTimeout(() => {
      presentCanvasInWrap();
      logViewMetrics('post-main');
      engineStatus.textContent = `running (${canvas.width}×${canvas.height})`;
    }, 800);
    veil.classList.add('hidden');
    mapsPanel.classList.remove('hidden');
    markDone('step-launch');
    launchStatus.textContent = 'running — click the game view to capture mouse and keyboard';
    engineStatus.textContent = `running (${canvas.width}×${canvas.height})`;
    log('engine main loop started with +map efw_prototype_level1');
    canvas.focus();
    setTimeout(() => {
      runEngineCmd('pausable 0');
    }, 4000);
    setInterval(() => {
      runEngineCmd('pausable 0');
    }, 2000);
    const resumeLoop = () => {
      const mod = (engine?.em as { Module?: { resumeMainLoop?: () => void } } | undefined)?.Module;
      try {
        mod?.resumeMainLoop?.();
      } catch {
        /* ignore */
      }
    };
    setInterval(resumeLoop, 100);
  } catch (err) {
    const msg = formatErr(err);
    launchStatus.textContent = `failed: ${msg}`;
    engineStatus.textContent = 'failed';
    setVeil('Boot failed', msg, 1);
    log(`BOOT FAILED: ${msg}`);
    engine = null;
    btnLaunch.disabled = false;
  }
}

// ---- wiring -----------------------------------------------------------------

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
    runEngineCmd(`map ${map}`);
    void captureInput();
  });
});

consoleForm.addEventListener('submit', (e) => {
  e.preventDefault();
  const cmd = consoleInput.value.trim();
  if (!cmd) return;
  log(`> ${cmd}`);
  // Game-DLL ClientCommand names are not host commands. Prefix with cmd
  // so the listen server forwards them to EFW_ClientCommand.
  const forwarded =
    /^(efw_|menuselect\b)/i.test(cmd) && !/^cmd\s/i.test(cmd) ? `cmd ${cmd}` : cmd;
  runEngineCmd(forwarded);
  if (forwarded !== cmd) runEngineCmd(cmd);
  consoleInput.value = '';
});

document.getElementById('btn-talk')?.addEventListener('click', () => {
  log('> talk (E / efw_Talk)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_Talk');
  void captureInput();
});
document.getElementById('btn-use')?.addEventListener('click', () => {
  log('> use (efw_spider)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_spider');
  void captureInput();
});
document.getElementById('btn-give')?.addEventListener('click', () => {
  log('> give (efw_Give)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_Give');
  void captureInput();
});
document.getElementById('btn-diary')?.addEventListener('click', () => {
  log('> diary (I / efw_diary)');
  runEngineCmd('pausable 0');
  pokeGameInput(199);
});
document.querySelectorAll<HTMLButtonElement>('button[data-talk]').forEach((btn) => {
  btn.addEventListener('click', () => {
    const slot = Number(btn.dataset.talk);
    if (Number.isFinite(slot) && slot > 0)
      chooseTalkSlot(slot);
  });
});
mapsPanel.addEventListener('pointerdown', (e) => {
  if (e.target instanceof HTMLButtonElement || e.target instanceof HTMLInputElement)
    return;
});
document.addEventListener('keydown', (e) => {
  if (e.target === consoleInput || e.target instanceof HTMLInputElement)
    return;
  if (e.repeat)
    return;
  if (e.key >= '1' && e.key <= '9') {
    chooseTalkSlot(Number(e.key));
  } else if (e.key === 'i' || e.key === 'I') {
    document.getElementById('btn-diary')?.click();
  }
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
