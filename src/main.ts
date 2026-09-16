import './style.css';
import { Xash3D } from 'xash3d-fwgs';
import { unzipSync } from 'fflate';
import { EfwLoopbackNet } from './loopback-net';

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
    return `${url}?v=efw-dll85`;
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
let loopbackNet: EfwLoopbackNet | null = null;
let logLines = 0;

function applyEfwVgui(text: string): boolean {
  const layer = document.getElementById('efw-vgui');
  if (!layer) return false;
  const idx = text.indexOf('EFWVGUI');
  if (idx < 0) return false;
  const msg = text.slice(idx).replace(/\s+$/, '');
  if (msg === 'EFWVGUI CLR') {
    layer.innerHTML = '';
    layer.hidden = true;
    return true;
  }
  if (!msg.startsWith('EFWVGUI ADD ')) return true;
  const rest = msg.slice('EFWVGUI ADD '.length);
  const tab = rest.indexOf('\t');
  const head = tab >= 0 ? rest.slice(0, tab) : rest;
  const label = tab >= 0 ? rest.slice(tab + 1) : head;
  const parts = head.split(' ');
  if (parts.length < 5) return true;
  const [nx, ny, nw, nh, ...cmdParts] = parts;
  const cmd = cmdParts.join(' ');
  const btn = document.createElement('button');
  btn.type = 'button';
  btn.dataset.cmd = cmd;
  btn.textContent = label || cmd;
  btn.style.left = `${(Number(nx) * 100).toFixed(2)}%`;
  btn.style.top = `${(Number(ny) * 100).toFixed(2)}%`;
  btn.style.width = `${Math.max(8, Number(nw) * 100).toFixed(2)}%`;
  btn.style.height = `${Math.max(4, Number(nh) * 100).toFixed(2)}%`;
  btn.addEventListener('pointerdown', (ev) => {
    ev.preventDefault();
    ev.stopPropagation();
  });
  btn.addEventListener('click', (ev) => {
    ev.preventDefault();
    ev.stopPropagation();
    log(`> ${cmd}`);
    runEngineCmd('pausable 0');
    runGameCmd(cmd);
  });
  layer.appendChild(btn);
  layer.hidden = false;
  /* Original CommandButtons own the cursor; unlock so the HTML stand-in is clickable. */
  if (document.pointerLockElement)
    document.exitPointerLock();
  return true;
}

/* FUN_10047830 VGUI storyboards: HTML Panel stand-in while HUD SPR cannot run. */
const EFW_STORY: Record<number, { title: string; next?: string }> = {
  0x3c: { title: 'You realise that the guard will search you and find the pliers, and so decide not to leave the kitchen.' },
  0x3d: { title: "You wait until the electrician is not looking, and quickly grab the pliers from the workbench. He doesn't notice, and you hide them under your shirt. Heart pounding, you wonder how to safely get them to Amir." },
  0x3e: { title: 'Again, you wait for the ideal moment to retrieve the pliers from under your shirt and slowly lower them into the bin, careful to not make a sound.' },
  0x3f: { title: "You realise that this is an ideal place to hide yourself for the next few hours, and wait until night falls. Now that the trader has agreed to take your ID tag from the fence, you won't be missed." },
  0x40: { title: "There's a hole. You could hide here, if you ever needed to." },
  0x41: { title: "You could hide here, but you'd be caught at dusk when the guards saw your ID tag and came searching." },
  0x42: { title: 'You could hide here and come out at night to get the pliers, if only you had a way to break into the rubbish bin cage.' },
  0x43: { title: 'You return to the hiding place, with the pliers safely tucked away underneath your shirt.' },
  0x44: { title: "You could hide again, but you haven't got the pliers yet." },
  0x45: { title: 'You recognise the bin in front of you as the one from the kitchen earlier today. You open the top and dig around inside, and sure enough, the pliers are still there. You retrieve them from the foodscraps and rubbish, and hide them in your clothes. Now to work out how to safely get these back to your fellow plotters.' },
  0x46: { title: 'Isolation.', next: 'efw_changelevel efw_prototype_level2' },
  0x47: { title: 'Help' },
  0x48: { title: '' },
  0x49: { title: 'Introduction' },
  0x4a: { title: 'Introduction' },
  0x4b: { title: 'Introduction' },
  0x4c: { title: 'Ending', next: 'efw_changelevel efw_prototype_level1' },
  0x4d: { title: 'Run out of hope!' },
  0x4e: { title: 'Ending', next: 'efw_changelevel efw_prototype_level1' },
  0x4f: { title: 'Decoy', next: 'efw_changelevel efw_prototype_level2' },
  0x50: { title: 'Decoy', next: 'efw_changelevel efw_prototype_level3' },
  0x51: { title: 'Isolation.' },
  0x52: { title: 'Help' },
};
let storyNext = '';

function dismissEfwStory() {
  const layer = document.getElementById('efw-story');
  if (layer)
    layer.hidden = true;
  const next = storyNext;
  storyNext = '';
  if (next) {
    log(`> ${next} (storyboard dismiss)`);
    runEngineCmd('pausable 0');
    runGameCmd(next);
  }
}

function showEfwStory(code: number, fallback?: string) {
  if (code === 0x48)
    return;
  const spec = EFW_STORY[code];
  const layer = document.getElementById('efw-story');
  const text = document.getElementById('efw-story-text');
  if (!layer || !text)
    return;
  const title = fallback || spec?.title;
  if (!title)
    return;
  text.textContent = title;
  storyNext = spec?.next || '';
  layer.hidden = false;
  if (document.pointerLockElement)
    document.exitPointerLock();
  log(`efw: storyboard 0x${code.toString(16)}`);
}

function applyEfwStory(text: string): boolean {
  const m = />>> FailOrNarrate 0x([0-9a-fA-F]+)/.exec(text);
  if (!m)
    return false;
  showEfwStory(parseInt(m[1], 16));
  return false; /* still show the log line */
}

type WasmFS = {
  mkdir: (p: string) => void;
  writeFile: (p: string, d: Uint8Array, opts?: { canOwn?: boolean }) => void;
  unlink: (p: string) => void;
  readFile?: (p: string, opts?: { encoding?: string }) => string | Uint8Array;
};
let wasmFS: WasmFS | null = null;
let lastVguiFile = '';

function ingestVguiFile(raw: string) {
  if (raw === lastVguiFile) return;
  lastVguiFile = raw;
  (window as Window & { __efwVguiFile?: string }).__efwVguiFile = raw;
  for (const line of raw.split(/\r?\n/)) {
    const trimmed = line.trim();
    if (trimmed) applyEfwVgui(trimmed);
  }
}

function pollEfwVgui() {
  if (!wasmFS?.readFile) return;
  for (const path of ['/efwvgui.txt', '/woomera/efwvgui.txt', '/rwdir/efwvgui.txt']) {
    try {
      const data = wasmFS.readFile(path, { encoding: 'utf8' });
      ingestVguiFile(typeof data === 'string' ? data : new TextDecoder().decode(data));
      return;
    } catch {
      /* path missing */
    }
  }
}

function log(text: string) {
  const normalized = String(text).replace(/\s+$/, '');
  if (!normalized) return;
  if (normalized.includes('efw: ServerActivate ents='))
    onServerActivateSeen();
  if (normalized.includes('CHANGE_LEVEL returned') || normalized.includes('CHANGE_LEVEL StartFrame'))
    logChangeLevelProgress(normalized);
  if (normalized.includes('HUD_Redraw skip') || normalized.includes('StartFrame done live='))
    resumeAfterFirstClientFrame();
  if (applyEfwStory(normalized)) return;
  if (applyEfwVgui(normalized)) return;
  logLines++;
  logCount.textContent = String(logLines);
  logEl.textContent += normalized + '\n';
  if (logLines > 20000) {
    const lines = logEl.textContent.split('\n');
    logEl.textContent = lines.slice(lines.length - 20000).join('\n');
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
(window as Window & { pressGameKey?: typeof pressGameKey }).pressGameKey = pressGameKey;

function chooseTalkSlot(slot: number) {
  log(`> menuselect ${slot}`);
  runEngineCmd('pausable 0');
  runEngineCmd(`menuselect ${slot}`);
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

let resumedAfterClientFrame = false;
let lastResumeMs = 0;
/* Con_ToggleConsole_f: closing the console while cls.state==ca_active
   calls UI_SetActiveMenu(false). Closing it earlier reopens libmenu.
   Hold key_console only for SCR_BeginLoadingPlaque, then release after
   HUD_Redraw (proof of ca_active). */
let consoleForPlaque = false;
function resumeEngineLoop() {
  const now = Date.now();
  /* resume() increments currentlyRunningMainloop and aborts the in-flight
     Host_Frame. Do not call it from the 120ms pump or CHANGE_LEVEL never
     finishes loading the next map. */
  if (now - lastResumeMs < 1500)
    return;
  lastResumeMs = now;
  const mod = (engine?.em as { Module?: { resumeMainLoop?: () => void } } | undefined)?.Module;
  try {
    mod?.resumeMainLoop?.();
  } catch {
    /* ignore */
  }
}

function releaseConsoleToGame() {
  if (!consoleForPlaque)
    return;
  consoleForPlaque = false;
  /* ca_active is required: otherwise Con_ToggleConsole_f reopens the menu. */
  runEngineCmd('toggleconsole');
  log('listen: toggleconsole while ca_active (UI_SetActiveMenu false)');
}

function dismissMenuAfterHud() {
  if (consoleForPlaque)
    releaseConsoleToGame();
}

function resumeAfterFirstClientFrame() {
  if (changeWatch) return;
  if (resumedAfterClientFrame) return;
  resumedAfterClientFrame = true;
  /* Do not Cmd_ExecuteString or resumeMainLoop from inside HUD_Redraw's
     Con_Printf → JS log. dll74 nested toggleconsole+resume there and
     aborted the rAF runner, so CHANGE_LEVEL never reached SV_Exec. */
  setTimeout(() => {
    if (changeWatch) return;
    dismissMenuAfterHud();
  }, 80);
}

let startedMap = '';
let listenReady = false;
let pumpTimer: ReturnType<typeof setInterval> | null = null;
let pausableTimer: ReturnType<typeof setInterval> | null = null;
let changeWatch: ReturnType<typeof setTimeout> | null = null;

function startHostPumps() {
  if (pumpTimer) {
    clearInterval(pumpTimer);
    pumpTimer = null;
  }
  let pumps = 0;
  pumpTimer = setInterval(() => {
    if (pumps < 80) {
      runEngineCmd('efw_pump');
      pumps++;
    } else if (pumpTimer) {
      clearInterval(pumpTimer);
      pumpTimer = null;
    }
  }, 120);
}

function logChangeLevelProgress(line: string) {
  if (line.includes('CHANGE_LEVEL returned'))
    log('listen: pfnChangeLevel returned — waiting for SV_ExecChangeLevel (no resumeMainLoop)');
}

function loadMap(name: string, reason: string) {
  if (startedMap === name) {
    log(`skip duplicate map ${name} (${reason})`);
    return;
  }
  const prev = startedMap;
  startedMap = name;
  log(`> map ${name} (${reason})`);
  /* Extra `map` is a no-op while Host is in RUNFRAME. CHANGE_LEVEL (the
     PE ClientCommand pfnChangeLevel) queues Host STATE_CHANGELEVEL. */
  if (!prev) {
    runEngineCmd(`map ${name}`);
    return;
  }
  listenReady = false;
  lastActivateMs = 0;
  resumedAfterClientFrame = false;
  consoleForPlaque = false;
  if (pumpTimer) {
    clearInterval(pumpTimer);
    pumpTimer = null;
  }
  /* resumeMainLoop() increments currentlyRunningMainloop and kills the
     rAF runner. dll65/66 kicked resume 80–2500ms after pfnChangeLevel
     and Host stayed RUNFRAME (StartFrame live=30) then went silent.
     COM_Frame only runs SV_ExecChangeLevel on the *next* rAF after
     Host_RunFrame promotes STATE_CHANGELEVEL — leave that runner alone. */
  runEngineCmd('pausable 0');
  runEngineCmd('cancelselect');
  runEngineCmd('r_norefresh 1');
  runEngineCmd('sv_validate_changelevel 0');
  runEngineCmd('sv_newunit 1');
  runEngineCmd('sv_validate_changelevel');
  setTimeout(() => {
    if (listenReady)
      return;
    log(`listen: pfnChangeLevel ${name}`);
    runEngineCmd('pausable 0');
    runEngineCmd('sv_validate_changelevel 0');
    /* Plaque SCR_UpdateScreen hangs the software renderer. key_console
       makes SCR_BeginLoadingPlaque return before that present.
       Do not toggleconsole again until HUD_Redraw: closing the console
       while !ca_active calls UI_SetActiveMenu(true) and brings libmenu back. */
    runEngineCmd('toggleconsole');
    consoleForPlaque = true;
    log('listen: key_console for plaque (skip software present hang)');
    runEngineCmd(`efw_changelevel ${name}`);
    /* Do not resumeMainLoop here. A live rAF is what runs SV_ExecChangeLevel
       on the next COM_Frame; kicking resume aborts that runner (dll65/74). */
  }, 200);
  if (changeWatch)
    clearTimeout(changeWatch);
  changeWatch = setTimeout(() => {
    if (listenReady)
      return;
    log(`listen: CHANGE_LEVEL stalled, disconnect+map ${name}`);
    startedMap = '';
    runEngineCmd('disconnect');
    lastResumeMs = 0;
    resumeEngineLoop();
    setTimeout(() => {
      runEngineCmd('disconnect');
      setTimeout(() => loadMap(name, 'after disconnect'), 400);
    }, 400);
  }, 18000);
}

let lastActivateMs = 0;
let menuDismissed = false;
function onServerActivateSeen() {
  const now = Date.now();
  if (now - lastActivateMs < 800)
    return;
  lastActivateMs = now;
  listenReady = true;
  resumedAfterClientFrame = false;
  if (changeWatch) {
    clearTimeout(changeWatch);
    changeWatch = null;
  }
  log(`listen: ServerActivate — ${loopbackNet?.summary() ?? 'no loopback net'}`);
  /* Keep r_norefresh 1 until HUD_Redraw has looped; software has no
     r_drawworld. Host_Frame now returns so HostPump is not required. */
  runEngineCmd('r_drawviewmodel 0');
  runEngineCmd('r_norefresh 1');
  runEngineCmd('sv_validate_changelevel 0');
  runEngineCmd('sv_newunit 1');
  runEngineCmd('pausable 0');
  runEngineCmd('cancelselect');
  runEngineCmd('scr_loading 0');
  runEngineCmd('ui_renderworld 1');
  setTimeout(() => {
    runEngineCmd('developer 0');
    runEngineCmd('con_notifytime 0');
    runEngineCmd('pausable 0');
    runEngineCmd('cancelselect');
    runEngineCmd('ui_renderworld 1');
    /* togglemenu is CL_Escape_f: no-op when key_dest is the menu, otherwise
       it OPENS the menu. Resume Game is hidden unless CL_IsActive() at
       VidInit. Dismiss via Con_ToggleConsole_f only after HUD_Redraw
       (ca_active). Do not toggleconsole here — ServerActivate runs while
       the client is still connecting and would reopen libmenu. */
    if (!menuDismissed) {
      menuDismissed = true;
      log('listen: waiting HUD_Redraw before menu dismiss');
    } else {
      log('listen: CHANGE_LEVEL activate — hold console until HUD_Redraw');
    }
  }, 250);
  setTimeout(() => {
    log(`listen: net ${loopbackNet?.summary() ?? 'none'}`);
    runEngineCmd('status');
  }, 2000);
  setTimeout(() => {
    if (changeWatch) return;
    log('listen: host_clientloaded (r_norefresh 1)');
    runEngineCmd('host_clientloaded 1');
    runEngineCmd('host_gameloaded 1');
    if (resumedAfterClientFrame) {
      log('listen: skip 4s resume (HUD already looping)');
      return;
    }
    lastResumeMs = 0;
    resumeEngineLoop();
  }, 4000);
  setTimeout(() => {
    if (changeWatch) return;
    runEngineCmd('r_norefresh 0');
    runEngineCmd('r_drawentities 1');
    runEngineCmd('ui_renderworld 1');
    runEngineCmd('scr_loading 0');
    log('listen: r_norefresh 0 ui_renderworld 1 (soft world present)');
  }, 8000);
  if (!pausableTimer) {
    pausableTimer = setInterval(() => {
      runEngineCmd('pausable 0');
    }, 4000);
  }
}

/** Host console plus EFW AddServerCommand (EFW_HostFwd). Do not prefix
 *  `cmd` — that queues a usercmd and never runs while ClientFrame is stuck. */
function runGameCmd(cmd: string) {
  const trimmed = cmd.trim();
  if (!trimmed) return;
  runEngineCmd(trimmed);
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
    log(`hlsdk wasm ${publicAsset('hlsdk/client.wasm')} ${clientWasm.byteLength}b / server ${serverWasm.byteLength}b`);
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
    const bootArgs = [
      '-windowed',
      '-nointro',
      '-console',
      '-dev',
      '1',
      '-game',
      GAME_DIR,
      '+maxplayers',
      '1',
      '+mp_allowmonsters',
      '1',
      '+deathmatch',
      '0',
      '+pausable',
      '0',
      '+sv_lan',
      '1',
      '+r_drawentities',
      '0',
      '+r_drawviewmodel',
      '0',
      '+r_drawparticles',
      '0',
      '+r_norefresh',
      '1',
      '+sv_validate_changelevel',
      '0',
      '+sv_newunit',
      '1',
      '+ui_renderworld',
      '1',
      '+r_fullbright',
      '1',
      '+cl_himodels',
      '0',
    ];
    if (shimOk) {
      bootArgs.splice(1, 0, '-width', String(view.width), '-height', String(view.height));
    }
    engine = new Xash3D({
      canvas,
      renderer: 'soft',
      arguments: bootArgs,
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
    loopbackNet = new EfwLoopbackNet();
    loopbackNet.onLog = log;
    engine.net = loopbackNet;
    (window as Window & { __efwNet?: EfwLoopbackNet }).__efwNet = loopbackNet;
    (window as Window & { __efwRun?: typeof runEngineCmd }).__efwRun = runEngineCmd;
    log('boot: init() with in-process loopback net');
    await engine.init();
    log('boot: init ok');
    {
      const mod = (engine.em as { Module?: {
        pauseMainLoop?: () => void;
        resumeMainLoop?: () => void;
      } } | undefined)?.Module;
      if (mod?.pauseMainLoop) {
        const origPause = mod.pauseMainLoop.bind(mod);
        mod.pauseMainLoop = () => {
          log('listen: MainLoop.pause');
          origPause();
        };
      }
    }

    const FS = (engine.em as unknown as { FS?: WasmFS })?.FS;
    if (!FS) throw new Error('WASM filesystem unavailable after init');
    wasmFS = FS;
    log(`boot: writing ${staged.valve.size} valve files + ${staged.woomera.size} woomera files`);

    setVeil('Installing game files…', 'Writing woomera/ + valve/ into WASM memory.', 0.8);
    await new Promise((r) => setTimeout(r, 30));
    writeTree(FS, staged.valve, (done, total) =>
      setVeil('Installing game files…', `valve/ ${done}/${total}`, 0.8 + 0.1 * (done / Math.max(1, total))),
    );
    log('boot: valve files written');
    writeTree(FS, staged.woomera);
    log('boot: woomera files written');
    mkdirTree(FS, `/${GAME_DIR}/overviews`);
    {
      const maps = ['efw_prototype_level1', 'efw_prototype_level2', 'efw_prototype_level3'];
      const txt = new TextEncoder().encode('ZOOM 1.0\nORIGIN 0 0 0\nROTATED 0\nHEIGHT 0\n');
      const tga = new Uint8Array(21);
      tga[2] = 2;
      tga[12] = 1;
      tga[14] = 1;
      tga[16] = 24;
      for (const map of maps) {
        try {
          FS.writeFile(`/${GAME_DIR}/overviews/${map}.txt`, txt);
          FS.writeFile(`/${GAME_DIR}/overviews/${map}.tga`, tga);
        } catch {
          /* ignore */
        }
      }
      log('boot: stub overviews written');
    }
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
    log('engine main loop started; +map is in Host_Init argv');
    canvas.focus();
    startedMap = '';
    listenReady = false;
    menuDismissed = false;
    consoleForPlaque = false;
    setTimeout(() => {
      const bootMap = new URLSearchParams(window.location.search).get('map') || 'efw_prototype_level1';
      loadMap(bootMap, 'deferred after Host_Init');
    }, 1500);
    setInterval(pollEfwVgui, 250);
    pollEfwVgui();
    document.addEventListener('visibilitychange', () => {
      if (!document.hidden && listenReady) resumeEngineLoop();
    });
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
    if (map) loadMap(map, 'maps panel');
    void captureInput();
  });
});

consoleForm.addEventListener('submit', (e) => {
  e.preventDefault();
  const cmd = consoleInput.value.trim();
  if (!cmd) return;
  log(`> ${cmd}`);
  runEngineCmd(cmd);
  consoleInput.value = '';
});

document.getElementById('efw-story-dismiss')?.addEventListener('click', (ev) => {
  ev.preventDefault();
  ev.stopPropagation();
  dismissEfwStory();
});
document.getElementById('efw-story')?.addEventListener('click', (ev) => {
  if (ev.target === document.getElementById('efw-story'))
    dismissEfwStory();
});
document.getElementById('btn-talk')?.addEventListener('click', () => {
  log('> talk (efw_Talk)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_Talk');
});
document.getElementById('btn-use')?.addEventListener('click', () => {
  log('> use (IN_USE / FUN_100c4af0)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_inuse');
  runGameCmd('use');
});
document.getElementById('btn-give')?.addEventListener('click', () => {
  log('> give (efw_Give)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_Give');
});
document.getElementById('btn-diary')?.addEventListener('click', () => {
  log('> diary (I / efw_diary)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_diary');
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
  if (e.key === 'e' || e.key === 'E' || e.key === 'Escape' || e.key === 'Enter') {
    const story = document.getElementById('efw-story');
    if (story && !story.hidden) {
      dismissEfwStory();
      return;
    }
  }
  if (e.key >= '1' && e.key <= '9') {
    chooseTalkSlot(Number(e.key));
  } else if (e.key === 'e' || e.key === 'E') {
    log('> E (IN_USE)');
    runEngineCmd('pausable 0');
    runGameCmd('efw_inuse');
    runGameCmd('use');
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
