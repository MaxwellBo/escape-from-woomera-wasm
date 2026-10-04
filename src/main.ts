import './style.css';
import { Xash3D } from 'xash3d-fwgs';
import { unzipSync } from 'fflate';
import ValveUnpackWorker from './valve-unpack.worker.ts?worker';
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
    return `${url}?v=efw-dll345`;
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

/* GoldSrc SPR (IDSP v2): uint16 palette count, RGB palette, then one frame.
   texFormat 3 is alphatest (index 255 transparent). These are the
   FUN_10044f70 CommandButton images. */
const promptSpriteUrl: Record<string, string> = {};

function decodeGoldSrcSpr(data: Uint8Array): string | null {
  if (data.length < 80 || data[0] !== 0x49 || data[1] !== 0x44 || data[2] !== 0x53 || data[3] !== 0x50)
    return null;
  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
  const tex = view.getInt32(12, true);
  let off = 40;
  const ncol = view.getUint16(off, true);
  off += 2;
  if (ncol < 1 || ncol > 256 || off + ncol * 3 + 20 > data.length)
    return null;
  const pal = data.subarray(off, off + ncol * 3);
  off += ncol * 3;
  off += 4;
  const w = view.getInt32(off + 8, true);
  const h = view.getInt32(off + 12, true);
  off += 16;
  if (w < 1 || h > 1024 || h < 1 || w > 1024 || off + w * h > data.length)
    return null;
  const canvasEl = document.createElement('canvas');
  canvasEl.width = w;
  canvasEl.height = h;
  const ctx = canvasEl.getContext('2d');
  if (!ctx)
    return null;
  const img = ctx.createImageData(w, h);
  const pix = data.subarray(off, off + w * h);
  for (let i = 0; i < pix.length; i++) {
    const b = pix[i];
    const o = i * 4;
    if (tex === 3 && b === 255)
      continue;
    if (tex === 2) {
      img.data[o] = 255;
      img.data[o + 1] = 255;
      img.data[o + 2] = 255;
      img.data[o + 3] = b;
      continue;
    }
    const p = b * 3;
    img.data[o] = pal[p];
    img.data[o + 1] = pal[p + 1];
    img.data[o + 2] = pal[p + 2];
    img.data[o + 3] = 255;
  }
  ctx.putImageData(img, 0, 0);
  return canvasEl.toDataURL('image/png');
}

function stagedBytes(rel: string): Uint8Array | undefined {
  const want = `${GAME_DIR}/${rel}`.toLowerCase();
  const direct = staged.woomera.get(`${GAME_DIR}/${rel}`);
  if (direct)
    return direct;
  for (const [key, value] of staged.woomera) {
    if (key.toLowerCase() === want)
      return value;
  }
  return undefined;
}

function ensurePromptSprites() {
  if (promptSpriteUrl.speech)
    return;
  const files: Array<[string, string]> = [
    ['speech', 'sprites/efw_speech_bubble.spr'],
    ['give', 'sprites/efw_give_icon.spr'],
    ['hide', 'sprites/efw_hide_icon.spr'],
    ['pliers', 'sprites/efw_item_pliers.spr'],
    ['lever', 'sprites/efw_item_lever.spr'],
    ['branch', 'sprites/efw_item_branch.spr'],
    ['phone', 'sprites/efw_item_simcard.spr'],
    ['idtag', 'sprites/efw_item_idtag.spr'],
    ['card', 'sprites/efw_item_phonecard.spr'],
    ['powder', 'sprites/efw_item_washingpowder.spr'],
  ];
  for (const [key, rel] of files) {
    const bytes = stagedBytes(rel);
    if (!bytes)
      continue;
    const url = decodeGoldSrcSpr(bytes);
    if (url)
      promptSpriteUrl[key] = url;
  }
}

function itemPromptSprite(id: number): string | undefined {
  switch (id) {
    case 16: return promptSpriteUrl.pliers;
    case 17: return promptSpriteUrl.lever;
    case 18: return promptSpriteUrl.branch;
    case 19: return promptSpriteUrl.phone;
    case 20: return promptSpriteUrl.idtag;
    case 21:
    case 22:
    case 23: return promptSpriteUrl.card;
    case 24: return promptSpriteUrl.powder;
    default: return promptSpriteUrl.give;
  }
}

/* FUN_10044f70: talk uses the speech bubble; give/use uses that weapon's
   item sprite (weapon info +0xbc). Pickup loads efw_give_icon.spr when
   HasWep misses, and the weapon sprite when that item is already held. */
function promptSpriteFor(cmd: string, hint?: string): string | undefined {
  ensurePromptSprites();
  if (hint === 'give')
    return promptSpriteUrl.give;
  const c = cmd.trim();
  if (hint === 'wep') {
    const numbered = c.match(/^efw_(?:Give|UseWithMarker|Pickup)\s+(\d+)/);
    if (numbered)
      return itemPromptSprite(Number(numbered[1]));
  }
  if (c.startsWith('efw_Talk'))
    return promptSpriteUrl.speech;
  if (c.startsWith('efw_HideUnderBuilding'))
    return promptSpriteUrl.hide;
  if (c.startsWith('efw_PickupPliers'))
    return promptSpriteUrl.pliers;
  const numbered = c.match(/^efw_(?:Give|UseWithMarker|Pickup)\s+(\d+)/);
  if (numbered)
    return itemPromptSprite(Number(numbered[1]));
  if (c.startsWith('efw_Pickup') || c.startsWith('efw_Give'))
    return promptSpriteUrl.give;
  /* menuselect is the HUD topic list (FUN_100c6e60), not a world sprite. */
  return undefined;
}

function stylePromptButton(btn: HTMLButtonElement, cmd: string, nx: string, ny: string, nw: string, nh: string, hint?: string) {
  const menu = cmd.trim().match(/^menuselect\s+(\d+)/);
  if (menu) {
    /* FUN_100c6e60 draws the topic list in the HUD. The HTML control is only
       the click target over those lines. */
    btn.classList.remove('efw-prompt');
    btn.classList.add('efw-menu');
    btn.setAttribute('aria-label', btn.textContent || cmd);
    btn.textContent = '';
    btn.style.backgroundImage = '';
    delete btn.dataset.ax;
    delete btn.dataset.ay;
    /* FUN_1001db00 draws the Press rows on the lower panel. Keep the
       strip off the console until that draw reports the pen. */
    btn.style.left = '0%';
    btn.style.width = '100%';
    btn.style.top = '-20%';
    btn.style.height = '0%';
    btn.style.pointerEvents = 'none';
    const saved = menuHit.get(Number(menu[1]));
    if (saved)
      placeMenuHit(Number(menu[1]), saved.y, saved.h, btn);
    return;
  }
  btn.classList.remove('efw-menu');
  const url = promptSpriteFor(cmd, hint);
  if (!url) {
    btn.classList.remove('efw-prompt');
    btn.querySelector(':scope > .efw-prompt-label')?.remove();
    btn.style.backgroundImage = '';
    delete btn.dataset.ax;
    delete btn.dataset.ay;
    btn.style.left = `${(Number(nx) * 100).toFixed(2)}%`;
    btn.style.top = `${(Number(ny) * 100).toFixed(2)}%`;
    btn.style.width = `${Math.max(8, Number(nw) * 100).toFixed(2)}%`;
    btn.style.height = `${Math.max(4, Number(nh) * 100).toFixed(2)}%`;
    return;
  }
  /* Projected point is the orbit center (FUN_10045f20). The label is a
     separate caption, drawn under the quad only while the cursor is on it. */
  btn.dataset.ax = String(Number(nx) + Number(nw) / 2);
  btn.dataset.ay = String(Number(ny) + Number(nh));
  btn.classList.add('efw-prompt');
  btn.style.backgroundImage = `url("${url}")`;
  mountPromptLabel(btn);
}

function mountPromptLabel(btn: HTMLButtonElement) {
  const label = (btn.textContent || '').trim();
  btn.setAttribute('aria-label', label);
  for (const node of [...btn.childNodes]) {
    if (node.nodeType === Node.TEXT_NODE)
      node.remove();
  }
  let cap = btn.querySelector(':scope > .efw-prompt-label');
  if (!(cap instanceof HTMLSpanElement)) {
    cap = document.createElement('span');
    cap.className = 'efw-prompt-label';
    btn.appendChild(cap);
  }
  cap.textContent = label;
}

/* FUN_1001db00 hit: Press-line pen, in engine pixels. The strip stays
   parked until a choices=1 draw reports the row. */
const menuHit = new Map<number, { y: number; h: number }>();

function parkMenuHits() {
  menuHit.clear();
  document.querySelectorAll('#efw-vgui button.efw-menu').forEach((node) => {
    if (!(node instanceof HTMLButtonElement)) return;
    node.style.top = '-20%';
    node.style.height = '0%';
    node.style.pointerEvents = 'none';
  });
}

function placeMenuHit(slot: number, y: number, h: number, btn?: HTMLButtonElement) {
  const canvas = document.getElementById('canvas') as HTMLCanvasElement | null;
  const H = canvas?.height || 0;
  menuHit.set(slot, { y, h });
  const node = btn ?? document.querySelector(`#efw-vgui button[data-cmd="menuselect ${slot}"]`);
  if (!(node instanceof HTMLButtonElement) || H < 1) return;
  const hh = Math.max(15, h);
  node.style.left = '0%';
  node.style.width = '100%';
  node.style.top = `${((y / H) * 100).toFixed(2)}%`;
  node.style.height = `${((hh / H) * 100).toFixed(2)}%`;
  node.style.pointerEvents = 'auto';
}

function applyMenuHit(text: string): boolean {
  const menu = text.match(/>>> FUN_1001db00 menu reveal=(-?\d+)\/(\d+) choices=(\d+)/);
  if (menu) {
    if (menu[3] === '0')
      parkMenuHits();
    return false;
  }
  const hit = text.match(/>>> FUN_1001db00 hit (\d+) y=(\d+) h=(\d+)/);
  if (!hit) return false;
  placeMenuHit(Number(hit[1]), Number(hit[2]), Number(hit[3]));
  return false;
}

/* FUN_10045f20 reads cl.time minus DAT_100bc354. That client clock stays
   near 1, so the open ease would sit at radius 0. The page clock is the
   stand-in, the same way the host pump stands in for gpGlobals->time.
   The console and the MEMFS poll both replay CLR plus the same ADDs.
   Times are remembered per button set so that replay does not jump the
   icons back to the character. A CLR with no following ADD forgets them. */
let promptPoseSig = '';
let promptPoseAt = 0;
let promptPoseRaf = 0;
let promptPoseLogged = 0;
let promptHoverLogged = 0;
let promptPosePendingClear = false;
const promptPoseTimes = new Map<string, number>();
/* FUN_10046900 feeds mouse pixels to FUN_10045ff0. The hotspot is a
   32px square on the orbit center, in the same pixels as ScreenWidth. */
let promptPointerX = -9999;
let promptPointerY = -9999;
/* FUN_10046040 caption. The engine draws it with pfnDrawCharacter once
   the 32px square is hot. Empty clears the line. */
let hotCapLine = '';
let hotCapSent = '';

function notePromptPointer(ev: PointerEvent) {
  const canvas = document.getElementById('canvas');
  if (!(canvas instanceof HTMLCanvasElement))
    return;
  const rect = canvas.getBoundingClientRect();
  if (rect.width < 1 || rect.height < 1)
    return;
  promptPointerX = (ev.clientX - rect.left) * (canvas.width / rect.width);
  promptPointerY = (ev.clientY - rect.top) * (canvas.height / rect.height);
}

function forgetPromptPose() {
  promptPoseSig = '';
  promptPoseAt = 0;
  promptPoseLogged = 0;
  promptHoverLogged = 0;
  promptPoseTimes.clear();
  if (promptPoseRaf) {
    cancelAnimationFrame(promptPoseRaf);
    promptPoseRaf = 0;
  }
}

function layoutPromptColumn(layer: HTMLElement) {
  const buttons = [...layer.querySelectorAll('button.efw-prompt')] as HTMLButtonElement[];
  if (!buttons.length) {
    hotCapLine = '';
    return;
  }
  /* FUN_10045f20 adds pixels to the projected anchor, not a 640-wide
     fraction. base = pi * (1 + 2*index/count). Over the first 0.5s,
     u = 1 - min(2*dt, 1), radius = (1-u^3)*130,
     angle = base - u^3*pi + sin(dt*pi*0.8)*0.01.
     FUN_10044bf0 draws a centered quad of that side in screen pixels.
     FUN_10046040 drops the quad when the center is within 32px of an edge. */
  const sig = buttons.map((b) => b.dataset.cmd || '').join('|');
  if (sig !== promptPoseSig) {
    promptPoseSig = sig;
    let opened = promptPoseTimes.get(sig);
    if (opened == null) {
      opened = performance.now();
      promptPoseTimes.set(sig, opened);
      promptPoseLogged = 0;
    }
    promptPoseAt = opened;
  }
  const dt = Math.max(0, (performance.now() - promptPoseAt) / 1000);
  const clamped = Math.min(1, dt * 2);
  const u = 1 - clamped;
  const u3 = u * u * u;
  const radiusScale = 1 - u3;
  const wobble = Math.sin(dt * Math.PI * 0.8) * 0.01;
  const radius = 130 * radiusScale;
  const canvas = document.getElementById('canvas');
  const sw = canvas instanceof HTMLCanvasElement && canvas.width > 0 ? canvas.width : 640;
  const sh = canvas instanceof HTMLCanvasElement && canvas.height > 0 ? canvas.height : 480;
  /* FUN_10046040: sin(cl.time * pi * 10/7). Idle side is
     (sin*0.0225+1.5225)*64. Hover is (sin*0.15+1.65)*64, white.
     FUN_10045ff0 sets that hover flag only inside the 32px square. */
  const wave = Math.sin((performance.now() / 1000) * Math.PI * 1.4285715);
  if (promptPoseLogged === 0) {
    promptPoseLogged = 1;
    log(`efw: orbit ease dt=${dt.toFixed(2)} r=${radius.toFixed(1)}`);
  } else if (promptPoseLogged === 1 && clamped >= 0.5) {
    promptPoseLogged = 2;
    log(`efw: orbit ease dt=${dt.toFixed(2)} r=${radius.toFixed(1)}`);
  } else if (promptPoseLogged === 2 && clamped >= 1) {
    promptPoseLogged = 3;
    const idle = (wave * 0.0225 + 1.5225) * 64;
    log(`efw: orbit ease dt=${dt.toFixed(2)} r=${radius.toFixed(1)} side=${idle.toFixed(1)} sw=${sw} hot=32`);
  }
  const n = buttons.length;
  const radiusX = (130 * radiusScale) / sw;
  const radiusY = (130 * radiusScale) / sh;
  let hovering = false;
  let hotLine = '';
  buttons.forEach((b, i) => {
    const ax = Number(b.dataset.ax || '0.5');
    const ay = Number(b.dataset.ay || '0.5');
    const angle = Math.PI * (1 + (2 * i) / n) - u3 * Math.PI + wobble;
    const cx = ax + Math.sin(angle) * radiusX;
    const cy = ay + Math.cos(angle) * radiusY;
    const cxPx = cx * sw;
    const cyPx = cy * sh;
    const onScreen = cxPx - 32 > 0 && cxPx + 32 < sw && cyPx - 32 > 0 && cyPx + 32 < sh;
    const dx = promptPointerX - cxPx;
    const dy = promptPointerY - cyPx;
    const hover = onScreen && Math.abs(dx) <= 32 && Math.abs(dy) <= 32;
    if (hover) {
      hovering = true;
      const label = (b.getAttribute('aria-label') || '').replace(/[\n\r;"]/g, ' ').trim();
      if (label)
        hotLine = `${Math.round(cxPx)} ${Math.round(cyPx)} ${label}`;
    }
    b.classList.toggle('efw-hot', hover);
    b.style.visibility = onScreen ? 'visible' : 'hidden';
    const mul = hover ? wave * 0.15 + 1.65 : wave * 0.0225 + 1.5225;
    const side = mul * 64;
    const sprW = side / sw;
    const sprH = side / sh;
    b.style.left = `${((cx - sprW / 2) * 100).toFixed(2)}%`;
    b.style.top = `${((cy - sprH / 2) * 100).toFixed(2)}%`;
    b.style.width = `${(sprW * 100).toFixed(2)}%`;
    b.style.height = `${(sprH * 100).toFixed(2)}%`;
  });
  if (hovering && promptHoverLogged < 3) {
    let bucket = 0;
    if (wave <= -0.85)
      bucket = 2;
    else if (wave >= 0.85)
      bucket = 1;
    if (bucket >= promptHoverLogged) {
      promptHoverLogged = bucket + 1;
      const side = (wave * 0.15 + 1.65) * 64;
      log(`efw: orbit hover s=${wave.toFixed(2)} side=${side.toFixed(1)}`);
    }
  } else if (!hovering && promptHoverLogged >= 3) {
    promptHoverLogged = 0;
  }
  hotCapLine = hotLine;
  if ((clamped < 1 || hovering) && promptPoseRaf === 0) {
    promptPoseRaf = requestAnimationFrame(() => {
      promptPoseRaf = 0;
      const live = document.getElementById('efw-vgui');
      if (live)
        layoutPromptColumn(live);
    });
  }
}

function applyEfwVgui(text: string): boolean {
  const layer = document.getElementById('efw-vgui');
  if (!layer) return false;
  const idx = text.indexOf('EFWVGUI');
  if (idx < 0) return false;
  const msg = text.slice(idx).replace(/\s+$/, '');
  if (msg === 'EFWVGUI CLR') {
    /* Keep the last Press-row positions. The file poll repeats this
       clear, and the draw log does not repeat the rows. */
    layer.innerHTML = '';
    layer.hidden = true;
    promptPosePendingClear = true;
    queueMicrotask(() => {
      if (!promptPosePendingClear)
        return;
      promptPosePendingClear = false;
      forgetPromptPose();
    });
    return true;
  }
  if (msg.startsWith('EFWVGUI SCHEME ')) {
    /* FUN_100352e0: Default Scheme / Arial / 17 / FgColor 255 255 255 255 */
    const rest = msg.slice('EFWVGUI SCHEME '.length);
    const parts = rest.split('\t');
    const font = parts[1] || 'Arial';
    const size = Number(parts[2] || 17);
    const rgba = (parts[3] || '255,255,255,255').split(',').map((n) => Number(n));
    layer.dataset.scheme = parts[0] || 'Default Scheme';
    layer.dataset.font = font;
    layer.style.fontFamily = `${font}, Helvetica, sans-serif`;
    layer.style.fontSize = `${size}px`;
    layer.style.color = `rgba(${rgba[0] || 255}, ${rgba[1] || 255}, ${rgba[2] || 255}, ${(rgba[3] ?? 255) / 255})`;
    for (const btn of layer.querySelectorAll('button')) {
      btn.style.fontFamily = layer.style.fontFamily;
      btn.style.fontSize = layer.style.fontSize;
      btn.style.color = layer.style.color;
    }
    return true;
  }
  if (msg.startsWith('EFWVGUI HOPE ')) {
    setHopeHud(parseInt(msg.slice('EFWVGUI HOPE '.length), 10) || 0);
    return true;
  }
  if (!msg.startsWith('EFWVGUI ADD ')) return true;
  promptPosePendingClear = false;
  const rest = msg.slice('EFWVGUI ADD '.length);
  const fields = rest.split('\t');
  const head = fields[0] || '';
  const label = fields.length > 1 ? fields[1] : head;
  const hint = (fields[2] || '').trim();
  const parts = head.split(' ');
  if (parts.length < 5) return true;
  const [nx, ny, nw, nh, ...cmdParts] = parts;
  const cmd = cmdParts.join(' ');
  /* FUN_100c6d70: six Panel* slots. Replace an existing cmd instead of
     stacking HUD Con_Printf + MEMFS ingest duplicates. */
  const buttons = [...layer.querySelectorAll('button')];
  const existing = buttons.find((b) => b.dataset.cmd === cmd);
  if (existing) {
    existing.textContent = label || cmd;
    stylePromptButton(existing, cmd, nx, ny, nw, nh, hint);
    layoutPromptColumn(layer);
    return true;
  }
  if (buttons.length >= 6)
    return true;
  const btn = document.createElement('button');
  btn.type = 'button';
  btn.dataset.cmd = cmd;
  btn.textContent = label || cmd;
  stylePromptButton(btn, cmd, nx, ny, nw, nh, hint);
  if (layer.dataset.font) {
    btn.style.fontFamily = layer.style.fontFamily;
    btn.style.fontSize = layer.style.fontSize;
    btn.style.color = layer.style.color;
  }
  const activate = () => {
    log(`> ${cmd}`);
    runEngineCmd('pausable 0');
    /* FUN_100463c0 flies the button under the cursor, then drops the panel. */
    if (btn.classList.contains('efw-prompt')) {
      const prompts = [...layer.querySelectorAll('button.efw-prompt')];
      const idx = prompts.indexOf(btn);
      if (idx >= 0)
        runEngineCmd(`efw_iconfly ${idx}`);
      promptContext = false;
      for (const node of prompts)
        node.remove();
      layer.hidden = true;
      forgetPromptPose();
      hotCapLine = '';
      if (hotCapSent) {
        runEngineCmd('efw_hotcap');
        hotCapSent = '';
      }
      runGameCmd('efw_context 0');
    }
    runGameCmd(cmd);
  };
  btn.addEventListener('pointerdown', (ev) => {
    ev.stopPropagation();
  });
  btn.addEventListener('click', (ev) => {
    ev.preventDefault();
    ev.stopPropagation();
    activate();
  });
  layer.appendChild(btn);
  layoutPromptColumn(layer);
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
  0x3f: { title: "You realise that this is an ideal place to hide yourself for the next few hours, and wait until night falls. Now that the trader has agreed to take your ID tag from the fence, you won't be missed.", next: 'efw_ShowMenu 79' },
  0x40: { title: "There's a hole. You could hide here, if you ever needed to." },
  0x41: { title: "You could hide here, but you'd be caught at dusk when the guards saw your ID tag and came searching." },
  0x42: { title: 'You could hide here and come out at night to get the pliers, if only you had a way to break into the rubbish bin cage.' },
  0x43: { title: 'You return to the hiding place, with the pliers safely tucked away underneath your shirt.', next: 'efw_ShowMenu 80' },
  0x44: { title: "You could hide again, but you haven't got the pliers yet." },
  0x45: { title: 'You recognise the bin in front of you as the one from the kitchen earlier today. You open the top and dig around inside, and sure enough, the pliers are still there. You retrieve them from the foodscraps and rubbish, and hide them in your clothes. Now to work out how to safely get these back to your fellow plotters.' },
  0x46: { title: 'Isolation.', next: 'efw_changelevel efw_prototype_level2' },
  0x47: { title: 'The package is from a pen-friend, a member of a refugee support group in Melbourne. The letter accompanying it brings you some hope, knowing that there is someone in this country that cares about your fate. Inside the package are some chocolate bars, which you give to some children, and a box of washing powder. Your suspicions aroused by mysterious rattling sound, you feel inside the box and discover a SIM card for a mobile phone.' },
  0x48: { title: '' },
  0x49: { title: 'Introduction' },
  0x4a: { title: 'Introduction' },
  0x4b: { title: 'Introduction' },
  0x4c: { title: 'Ending', next: 'efw_changelevel efw_prototype_level1' },
  0x4d: { title: 'Run out of hope!', next: 'efw_ShowMenu 78' },
  0x4e: { title: 'Ending', next: 'efw_changelevel efw_prototype_level1' },
  0x4f: { title: 'Decoy', next: 'efw_changelevel efw_prototype_level2' },
  0x50: { title: 'Decoy', next: 'efw_changelevel efw_prototype_level3' },
  0x51: { title: 'Isolation.' },
  0x52: { title: 'Help' },
};
let storyNext = '';
let storyPaused = false;

function storyIsSprite(code: number): boolean {
  /* FUN_10047830 loads a Storyboard SPR for these codes. */
  return code === 0x3f || code === 0x43 || code === 0x46
    || (code >= 0x49 && code <= 0x52);
}

function storyboardPauses(code: number): boolean {
  /* FUN_10047830 EFW_Menu Panel ctors; 0x3c–0x45 (not 0x3f/0x43) are ShowMenu.
     0x47 FUN_10048790 caption Panel also pauses. 0x48 FUN_10048710 pauses. */
  return code === 0x3f || code === 0x43 || (code >= 0x46 && code <= 0x52);
}

function isCaptionMenu(code: number): boolean {
  /* FUN_10047830 else-branch: not SPR, not 0x48. Server sends 0x47 this way. */
  return code === 0x47;
}

function hideLetterbox() {
  const layer = document.getElementById('efw-letter');
  if (layer) layer.hidden = true;
}

function showLetterbox(code: number, caption?: string) {
  const layer = document.getElementById('efw-letter');
  const text = document.getElementById('efw-letter-text');
  const story = document.getElementById('efw-story');
  const interact = document.getElementById('efw-interact');
  if (!layer || !text)
    return;
  const body = caption || EFW_STORY[code]?.title || '';
  if (!body)
    return;
  text.textContent = body;
  layer.hidden = false;
  if (story) story.hidden = true;
  if (interact) interact.hidden = true;
  /* FUN_10048790 stores the caption, then the Panel sends efw_pause 1.
     A page pause before that length is set is the context-open edge. */
  if (storyboardPauses(code))
    storyPaused = true;
  if (document.pointerLockElement)
    document.exitPointerLock();
  log(`efw: letterbox 0x${code.toString(16)}`);
}

function dismissLetterbox() {
  hideLetterbox();
  if (storyPaused) {
    storyPaused = false;
    runEngineCmd('pausable 0');
    runGameCmd('efw_pause 0');
  }
}

/* The SPR comic is the engine storyboard. This layer is the click catcher
   over it, and while it is up the orbit buttons stay visibility:hidden.
   efw_story_key clears the engine panel on any key. Hide the catcher with
   that key so the buttons can draw. storyNext stays put so the page can
   still loadMap the stored changelevel. */
function hideStoryCatcher(): boolean {
  const story = document.getElementById('efw-story');
  const letter = document.getElementById('efw-letter');
  const storyUp = !!(story && !story.hidden);
  const letterUp = !!(letter && !letter.hidden);
  if (!storyUp && !letterUp)
    return false;
  if (story)
    story.hidden = true;
  if (letter)
    letter.hidden = true;
  return true;
}

function dismissEfwStory() {
  const layer = document.getElementById('efw-story');
  if (layer)
    layer.hidden = true;
  hideLetterbox();
  const next = storyNext;
  const paused = storyPaused;
  storyNext = '';
  storyPaused = false;
  /* FUN_100485d0: efw_pause 0, then stored changelevel. */
  if (paused) {
    runEngineCmd('pausable 0');
    runGameCmd('efw_pause 0');
  }
  if (next) {
    log(`> ${next} (storyboard dismiss)`);
    runEngineCmd('pausable 0');
    /* FUN_10047830 stores DAT_100bc9b0/c0 as a changelevel. pfnChangeLevel
       returns while Host stays RUNFRAME, so SV_ExecChangeLevel never ran.
       loadMap() keeps the rAF runner then falls back to disconnect+map. */
    const change = /^efw_changelevel\s+(\S+)/.exec(next);
    if (change) loadMap(change[1], 'storyboard dismiss');
    else runGameCmd(next);
  }
}

function isNarrationMenu(code: number): boolean {
  /* FUN_100c81d0 calls FUN_100c6e60 for these codes. 0x3f and 0x43
     add hope and send EFW_Menu. The blue panel is the caption. */
  return code === 0x3c || code === 0x3d || code === 0x3e
    || code === 0x40 || code === 0x41 || code === 0x42
    || code === 0x44 || code === 0x45;
}

function showEfwStory(code: number, fallback?: string) {
  if (isNarrationMenu(code)) {
    const layer = document.getElementById('efw-story');
    if (layer && !layer.classList.contains('efw-story-spr'))
      layer.hidden = true;
    log(`efw: FUN_100c81d0 menu 0x${code.toString(16)}`);
    return;
  }
  if (code === 0x48) {
    /* FUN_10048650: 0xd4 Panel, no storyboard SPR. FUN_10048710 pauses. */
    if (!storyPaused) {
      storyPaused = true;
      runEngineCmd('pausable 0');
      runGameCmd('efw_pause 1');
    }
    log('efw: FUN_10048650 panel 0xd4');
    return;
  }
  if (isCaptionMenu(code)) {
    showLetterbox(code, fallback);
    return;
  }
  const spec = EFW_STORY[code];
  const layer = document.getElementById('efw-story');
  const text = document.getElementById('efw-story-text');
  if (!layer || !text)
    return;
  const title = fallback || spec?.title;
  if (!title)
    return;
  /* Intro 0x49–0x4b are SPR-only in the PE. Do not cover the WASM view
     with an empty Continue — that blocked every browser play session. */
  if (code >= 0x49 && code <= 0x4b)
    return;
  text.textContent = title;
  storyNext = spec?.next || '';
  layer.classList.toggle('efw-story-spr', storyIsSprite(code));
  layer.hidden = false;
  const interact = document.getElementById('efw-interact');
  if (interact) interact.hidden = true;
  if (storyboardPauses(code)) {
    /* FUN_10048590: ClientCmd efw_pause 1 once per Panel show, not every
       FailOrNarrate log reprint. */
    if (!storyPaused) {
      storyPaused = true;
      runEngineCmd('pausable 0');
      runGameCmd('efw_pause 1');
    } else {
      storyPaused = true;
    }
  }
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

/* FUN_1001daa0: 10 ticks, filled while ticks >= i (empty when ticks < i). */
function setHopeHud(n: number): void {
  const hope = Math.max(0, Math.min(100, Math.round(n)));
  const ticks = Math.floor(hope / 10);
  const el = document.getElementById('efw-hope');
  const label = document.getElementById('efw-hope-label');
  const row = document.getElementById('efw-hope-ticks');
  /* The GL redraw draws FUN_1001daa0. This DOM copy sat on those bars. */
  if (el) el.hidden = true;
  if (label) label.textContent = `HOPE  ${hope}`;
  if (!row) return;
  if (row.childElementCount !== 10) {
    row.innerHTML = '';
    for (let i = 0; i < 10; i++) row.appendChild(document.createElement('i'));
  }
  [...row.children].forEach((tick, i) => tick.classList.toggle('on', !(ticks < i)));
}

function applyHopeHud(text: string): boolean {
  const daa = text.match(/>>> FUN_1001daa0 ticks=(\d+) hope=([\d.]+)/);
  if (daa) {
    setHopeHud(Number(daa[2]));
    return false;
  }
  const num = text.match(/>>> FUN_1001e880 n=(\d+)/);
  if (num) {
    setHopeHud(Number(num[1]));
    return false;
  }
  const persist = text.match(/persist hope=([\d.]+)/);
  if (persist) {
    setHopeHud(Number(persist[1]));
    return false;
  }
  const pulse = text.match(/>>> hope ([\d.]+)/);
  if (pulse) {
    setHopeHud(Number(pulse[1]));
    return false;
  }
  const m = text.match(/>>> hopehud ([\d.]+)/);
  if (!m) return false;
  setHopeHud(Number(m[1]));
  return false;
}

function applyHudColor(text: string): boolean {
  const m = text.match(/>>> FUN_1001e4c0 p=([\d.]+) lvl=(\d+) rgb=(\d+),(\d+),(\d+) a=(\d+)/);
  if (!m) return false;
  const layer = document.getElementById('efw-vgui');
  if (layer) {
    layer.dataset.hudColor = `${m[3]},${m[4]},${m[5]},${m[6]}`;
    layer.dataset.hudLevel = m[2];
  }
  return false;
}

function applyClockHud(text: string): boolean {
  const scheme = text.match(/>>> FUN_100352e0 scheme=(.+) font=(.+) size=(\d+)/);
  if (scheme) {
    const layer = document.getElementById('efw-vgui');
    if (layer) {
      const font = scheme[2].trim();
      const size = Number(scheme[3]);
      layer.dataset.scheme = scheme[1].trim();
      layer.dataset.font = font;
      layer.style.fontFamily = `${font}, Helvetica, sans-serif`;
      layer.style.fontSize = `${size}px`;
      layer.style.color = 'rgb(255, 255, 255)';
    }
  }
  const m = text.match(/>>> FUN_1001db00 clock=(.+) fade=([\d.]+) logo=(\d+)/);
  if (!m) return false;
  const el = document.getElementById('efw-clock');
  if (!el) return false;
  const fade = Number(m[2]);
  el.textContent = m[1].trim();
  el.hidden = fade <= 0;
  const rgb = Math.max(0, Math.min(100, Math.round(fade * 100)));
  el.style.color = `rgb(${rgb}, ${rgb}, ${rgb})`;
  el.style.opacity = String(fade);
  return false;
}

function applyDiaryHud(text: string): boolean {
  const fade = text.match(/>>> FUN_1001db00 diaryfade=([\d.]+) inv=([\d.]+) veil=([\d.]+) page=(\d+)/);
  if (fade) {
    const df = Number(fade[1]);
    const page = fade[4];
    const el = document.getElementById('efw-diary');
    const label = document.getElementById('efw-diary-label');
    const inv = document.getElementById('efw-inv');
    if (el) {
      el.hidden = df <= 0;
      el.style.setProperty('--efw-diary-fade', String(df));
    }
    if (label) label.textContent = `DIARY  ${page}`;
    /* FUN_10043dd0 draws the names in the client HUD. The HTML copy
       sat the two labels on top of each other. */
    if (inv) {
      inv.hidden = true;
      inv.replaceChildren();
    }
    return false;
  }
  const m = text.match(/>>> (?:diaryhud|efw_diary) open=(\d+) page=(\d+)/);
  if (!m) return false;
  const open = m[1] !== '0';
  const page = m[2];
  const el = document.getElementById('efw-diary');
  const label = document.getElementById('efw-diary-label');
  if (el && open) el.hidden = false;
  if (label) label.textContent = `DIARY  ${page}`;
  return false;
}

function applyContextHud(text: string): boolean {
  const none = document.getElementById('efw-none');
  if (text.includes('>>> FUN_10046370')) {
    const interact = document.getElementById('efw-interact');
    if (interact) interact.hidden = true;
    if (none) none.hidden = true;
    return false;
  }
  if (text.includes('>>> FUN_100463c0')) {
    if (none) none.hidden = true;
    return false;
  }
  if (text.includes('>>> FUN_10046590 none')) {
    /* FUN_10046590 already DrawHudString's this line. */
    if (none) none.hidden = true;
    return false;
  }
  return false;
}

function applyInteractHud(text: string): boolean {
  const m = text.match(/>>> FUN_10046590 interact=(.*)$/);
  if (!m) return false;
  const el = document.getElementById('efw-interact');
  /* FUN_10046590 paints the name bar and the click line. The HTML copy
     sat a second caption on that bar. */
  if (el) el.hidden = true;
  return false;
}

function applyLetterHud(text: string): boolean {
  const open = text.match(/>>> FUN_10048790 n=(\d+) code=0x([0-9a-fA-F]+)(?: (.*))?$/);
  if (open) {
    const code = parseInt(open[2], 16);
    const caption = open[3] || EFW_STORY[code]?.title;
    showLetterbox(code, caption);
    return false;
  }
  if (text.includes('>>> FUN_10043bb0') || text.includes('>>> FUN_1001d750')) {
    /* FUN_10043bb0 draws "Press left mouse button to continue". */
    return false;
  }
  return false;
}

function applyInvHud(text: string): boolean {
  const el = document.getElementById('efw-inv');
  if (!el) return false;
  /* Names are DrawHudString in FUN_10043dd0, 128px apart. */
  if (text.includes('>>> FUN_10043dd0')) {
    el.hidden = true;
    el.replaceChildren();
  }
  return false;
}

function applyPrevQuestion(_text: string): boolean {
  const el = document.getElementById('efw-prevq');
  /* FUN_1001db00 draws the previous line in the menu. The page header
     duplicated that line in white above the view. */
  if (el) el.hidden = true;
  return false;
}

function log(text: string) {
  const normalized = String(text).replace(/\s+$/, '');
  if (!normalized) return;
  applyHopeHud(normalized);
  applyHudColor(normalized);
  applyClockHud(normalized);
  applyDiaryHud(normalized);
  applyContextHud(normalized);
  applyInteractHud(normalized);
  applyInvHud(normalized);
  applyLetterHud(normalized);
  applyPrevQuestion(normalized);
  applyMenuHit(normalized);
  if (normalized.includes('efw: ServerActivate ents='))
    onServerActivateSeen();
  if (normalized.includes('not valid from the console'))
    armBootConsoleClose();
  if (normalized.includes('CHANGE_LEVEL returned') || normalized.includes('CHANGE_LEVEL StartFrame'))
    logChangeLevelProgress(normalized);
  if (normalized.includes('HUD_Redraw skip') || normalized.includes('StartFrame done live=')
      || normalized.includes('efw: world present live='))
    resumeAfterFirstClientFrame();
  if (normalized.includes('>>> FUN_10043750') || /efw: HUD_Draw n=(\d+)/.test(normalized)) {
    const n = /efw: HUD_Draw n=(\d+)/.exec(normalized);
    if (n)
      maybeCloseBootConsole(Number(n[1]));
    if (!n || Number(n[1]) > 8) {
      setTimeout(() => {
        runEngineCmd('con_notifytime -1');
        log('listen: con_notifytime -1');
      }, 400);
    }
  }
  if (consoleForPlaque && listenReady && !changeWatch && normalized.includes('HUD_Redraw skip'))
    schedulePlaqueClose();
  if (chapterNeedsGameKey && listenReady && !changeWatch && normalized.includes('HUD_Redraw skip')) {
    chapterNeedsGameKey = false;
    setTimeout(() => dismissChapterOverlay(), 2500);
  }
  if (/\bSpawning\b/.test(normalized) && normalized.includes('loopback'))
    finishListenSpawn();
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
let plaqueCloseTimer: ReturnType<typeof setTimeout> | null = null;
/* 0 idle, 1 engine rejected `begin` while key_dest was still the console,
   2 the follow-up toggle has run. Con_ToggleConsole_f returns immediately
   unless cls.state is ca_active, so the HUD_Redraw toggle is a no-op and
   the half-screen console stays over the view. The rejection is the proof
   key_dest is still key_console; the next toggle takes the close branch. */
let bootConsoleState = 0;
/* First-map libmenu pause is why engine StartFrame never advances without
   HostPump. Software present hung on UI_SetActiveMenu(false). WebGL2
   (gles3compat) is the new present path that can survive key_game. */
let firstMapKeyGame = false;
let chapterChanging = false;
let chapterNeedsGameKey = false;
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

function dismissChapterOverlay() {
  /* Same open-then-close as the first map. One toggle leaves the console
     up; the second calls UI_SetActiveMenu(false) and the chapter view
     stays on screen. */
  log('listen: chapter double toggleconsole → key_game');
  runEngineCmd('pausable 0');
  runEngineCmd('toggleconsole');
  setTimeout(() => {
    runEngineCmd('toggleconsole');
    runEngineCmd('setpause 0');
    runEngineCmd('unpause');
    runEngineCmd('pausable 0');
    log('listen: chapter key_game');
  }, 300);
}

function schedulePlaqueClose() {
  if (!consoleForPlaque || plaqueCloseTimer)
    return;
  /* HUD_Redraw skip is the ca_active proof. A toggle any earlier is ignored
     and the loading console stays over the next chapter. */
  plaqueCloseTimer = setTimeout(() => {
    plaqueCloseTimer = null;
    releaseConsoleToGame();
  }, 3500);
}

function releaseConsoleToGame() {
  if (!consoleForPlaque)
    return;
  consoleForPlaque = false;
  if (plaqueCloseTimer) {
    clearTimeout(plaqueCloseTimer);
    plaqueCloseTimer = null;
  }
  /* The first-map double toggle must not run after this close, or it
     opens the console again on top of the next chapter. */
  firstMapKeyGame = true;
  /* ca_active is required: otherwise Con_ToggleConsole_f reopens the menu. */
  runEngineCmd('toggleconsole');
  log('listen: toggleconsole while ca_active (UI_SetActiveMenu false)');
}

function armBootConsoleClose() {
  if (bootConsoleState !== 0 || consoleForPlaque || changeWatch)
    return;
  bootConsoleState = 1;
  log('listen: begin rejected from console — close once HUD is up');
}

function maybeCloseBootConsole(hudN: number) {
  if (bootConsoleState !== 1 || hudN < 40)
    return;
  if (consoleForPlaque || changeWatch || chapterChanging)
    return;
  bootConsoleState = 2;
  /* Outside the print callback. toggleconsole from inside Con_Printf
     nests Cmd_ExecuteString in the frame that is still painting. */
  setTimeout(() => {
    if (consoleForPlaque || changeWatch) {
      bootConsoleState = 1;
      return;
    }
    runEngineCmd('toggleconsole');
    log('listen: toggleconsole closed boot console');
  }, 400);
}

function dismissMenuAfterHud() {
  if (consoleForPlaque) {
    /* HUD_Redraw can fire while CHANGE_LEVEL is still connecting.
       toggleconsole then leaves the console open over the next chapter.
       The ServerActivate world-present timeout closes it once the view
       origin is live. */
    startHostPumps();
    log('listen: hold plaque console until world present');
    return;
  }
  if (firstMapKeyGame)
    return;
  firstMapKeyGame = true;
  /* HUD_Redraw proves ca_active, and the boot console is already
     key_console (Con_DestHeight is half the framebuffer). One
     Con_ToggleConsole_f closes it and calls UI_SetActiveMenu(false).
     A second toggle opens it again: the 3D view stays the bottom half
     and the intro pixels above it are never redrawn. Deferred so
     Cmd_ExecuteString is not nested inside HUD_Redraw. */
  log('listen: gles3compat first-map toggleconsole → key_game');
  runEngineCmd('toggleconsole');
  setTimeout(() => {
    runEngineCmd('setpause 0');
    runEngineCmd('unpause');
    runEngineCmd('pausable 0');
    /* SV_Spawn_f already ran (PreThink + game_playerspawn). Bare `spawn`
       is an unknown console command; `cmd spawn` with no spawncount calls
       SV_New_f and restarts signon. SV_Begin_f is the missing step. */
    runEngineCmd('cmd begin');
    runEngineCmd('cmd sendents');
    runEngineCmd('fullupdate');
    runEngineCmd('r_norefresh 0');
    runEngineCmd('r_drawworld 1');
    runEngineCmd('r_drawentities 1');
    runEngineCmd('r_fullbright 0');
    runEngineCmd('r_novis 1');
    runEngineCmd('gl_clear 1');
    runEngineCmd('ui_renderworld 1');
    startHostPumps();
    log('listen: key_game (gles3compat present, world draw on, cmd begin)');
  }, 250);
}

function applyMapSky() {
  /* sv_skyname alone does not rebuild the skybox. skyname does.
     Level 2 is the dusk set; the other maps are day256. */
  if (!startedMap) return;
  const sky = startedMap.indexOf('level2') >= 0 ? 'evening256' : 'day256';
  runEngineCmd(`skyname ${sky}`);
}

function forceWorldPresent() {
  runEngineCmd('r_norefresh 0');
  runEngineCmd('r_drawworld 1');
  runEngineCmd('r_drawentities 1');
  runEngineCmd('r_fullbright 0');
  runEngineCmd('r_novis 1');
  runEngineCmd('gl_clear 1');
  runEngineCmd('ui_renderworld 1');
  applyMapSky();
}

function resumeAfterFirstClientFrame() {
  if (changeWatch) return;
  if (resumedAfterClientFrame) return;
  resumedAfterClientFrame = true;
  /* StartFrame can prove the pawn is live before HUD_Redraw. Turn the
     world present path on immediately; do not wait for ClientFrame. */
  forceWorldPresent();
  /* Do not Cmd_ExecuteString or resumeMainLoop from inside HUD_Redraw's
     Con_Printf → JS log. dll74 nested toggleconsole+resume there and
     aborted the rAF runner, so CHANGE_LEVEL never reached SV_Exec. */
  setTimeout(() => {
    if (changeWatch) return;
    /* Client cvars register after ServerActivate, which puts the
       default back. 0 does not expire a notify line while the client
       clock is frozen (the test is cl.time - stamp > cvar). -1 does. */
    runEngineCmd('con_notifytime -1');
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
  /* Keep pulsing StartFrame after key_game. Libmenu used to pause the
     listen server; a 80-tick cap left hope/TalkScan frozen once HostPump
     stopped even though WebGL2 was still presenting. */
  let lastPumpMs = 0;
  pumpTimer = setInterval(() => {
    const now = performance.now();
    const dt = lastPumpMs ? Math.min(0.25, (now - lastPumpMs) / 1000) : 0.12;
    lastPumpMs = now;
    runEngineCmd(`efw_pump ${dt.toFixed(3)}`);
    /* FUN_10045f20 keeps moving for half a second after the buttons exist.
       The vgui file does not rewrite, so the pump is what steps the ease
       when a paint callback does not land between wasm frames. */
    const poseLayer = document.getElementById('efw-vgui');
    if (poseLayer?.querySelector('button.efw-prompt'))
      layoutPromptColumn(poseLayer);
    else
      hotCapLine = '';
    if (hotCapLine !== hotCapSent) {
      runEngineCmd(hotCapLine ? `efw_hotcap ${hotCapLine}` : 'efw_hotcap');
      hotCapSent = hotCapLine;
    }
    pumps++;
    if (pumps === 1 || (pumps % 80) === 0)
      log(`listen: hostpump n=${pumps}`);
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
  /* FUN_100c6d70 slot dtor: drop HTML CommandButtons so level1 Talk
     widgets do not sit on the next chapter's splash. */
  {
    const layer = document.getElementById('efw-vgui');
    if (layer) {
      layer.innerHTML = '';
      layer.hidden = true;
    }
    const story = document.getElementById('efw-story');
    if (story) story.hidden = true;
    hideLetterbox();
  }
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
  chapterChanging = true;
  chapterNeedsGameKey = true;
  if (plaqueCloseTimer) {
    clearTimeout(plaqueCloseTimer);
    plaqueCloseTimer = null;
  }
  firstMapKeyGame = true;
  if (bootConsoleState === 2)
    bootConsoleState = 0;
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
    /* WebGL presents through the loading plaque, so do not open the
       console here. While cls.state is ca_connected the engine draws the
       console anyway; toggleconsole before ca_active is a no-op, and the
       same call once background cvars are set opens the main menu. */
    runEngineCmd('sv_background 0');
    runEngineCmd('cl_background 0');
    consoleForPlaque = false;
    log('listen: changelevel without plaque console');
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
let spawnTries = 0;
function finishListenSpawn() {
  if (spawnTries >= 8)
    return;
  spawnTries++;
  /* Nested Cmd_ExecuteString from Con_Printf (status) aborts Host_Frame.
     SV_Begin_f requires cs_spawning; GoldSrc protocol sends sendents. */
  const n = spawnTries;
  setTimeout(() => {
    runEngineCmd('cmd begin');
    runEngineCmd('cmd sendents');
    runEngineCmd('fullupdate');
    runEngineCmd('r_norefresh 0');
    runEngineCmd('r_drawworld 1');
    runEngineCmd('r_drawentities 1');
    runEngineCmd('r_novis 1');
    log(`listen: cmd begin/sendents try=${n}`);
  }, 0);
}
function onServerActivateSeen() {
  const now = Date.now();
  if (now - lastActivateMs < 800)
    return;
  lastActivateMs = now;
  spawnTries = 0;
  listenReady = true;
  resumedAfterClientFrame = false;
  const changing = !!changeWatch;
  if (changeWatch) {
    clearTimeout(changeWatch);
    changeWatch = null;
  }
  log(`listen: ServerActivate — ${loopbackNet?.summary() ?? 'no loopback net'}`);
  startHostPumps();
  /* First map: keep the world presenting. r_norefresh 1 here used to paint
     a black canvas for the whole session because HUD_Redraw often never
     ran. Only blank the plaque during CHANGE_LEVEL. */
  /* GoldSrc draws the held view model (v_idtag and the rest). */
  runEngineCmd('r_drawviewmodel 1');
  if (changing) {
    runEngineCmd('r_norefresh 1');
  } else {
    forceWorldPresent();
  }
  runEngineCmd('sv_validate_changelevel 0');
  runEngineCmd('sv_newunit 1');
  runEngineCmd('pausable 0');
  runEngineCmd('cancelselect');
  runEngineCmd('scr_loading 0');
  runEngineCmd('ui_renderworld 1');
  setTimeout(() => {
    runEngineCmd('developer 0');
    runEngineCmd('con_notifytime -1');
    applyMapSky();
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
    log('listen: host_clientloaded');
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
    runEngineCmd('r_drawworld 1');
    runEngineCmd('r_drawentities 1');
    runEngineCmd('ui_renderworld 1');
    runEngineCmd('scr_loading 0');
    runEngineCmd('con_notifytime -1');
    finishListenSpawn();
    runEngineCmd('status');
    log('listen: r_norefresh 0 r_drawworld 1 (world present)');
    if (!chapterChanging && !firstMapKeyGame && !consoleForPlaque)
      dismissMenuAfterHud();
    chapterChanging = false;
  }, 2500);
  if (!pausableTimer) {
    pausableTimer = setInterval(() => {
      runEngineCmd('pausable 0');
      runEngineCmd('con_notifytime -1');
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

function yieldFrame(): Promise<void> {
  return new Promise((r) => setTimeout(r, 0));
}

/** Split a GoldSrc PACK into loose files so we never FS.writeFile a 50MB blob. */
async function explodePakYielding(
  pak: Uint8Array,
  dest: Map<string, Uint8Array>,
  prefix: string,
  onProgress?: (done: number, total: number) => void,
) {
  const magic = String.fromCharCode(pak[0] ?? 0, pak[1] ?? 0, pak[2] ?? 0, pak[3] ?? 0);
  if (magic !== 'PACK') {
    dest.set(`${prefix}pak0.pak`, pak);
    return;
  }
  const view = new DataView(pak.buffer, pak.byteOffset, pak.byteLength);
  const off = view.getUint32(4, true);
  const length = view.getUint32(8, true);
  const total = Math.floor(length / 64);
  const decoder = new TextDecoder('latin1');
  let n = 0;
  for (let i = 0; i < length; i += 64) {
    const raw = pak.subarray(off + i, off + i + 56);
    let nlen = raw.indexOf(0);
    if (nlen < 0) nlen = 56;
    const name = decoder.decode(raw.subarray(0, nlen)).replace(/\\/g, '/');
    if (name) {
      const eoff = view.getUint32(off + i + 56, true);
      const esize = view.getUint32(off + i + 60, true);
      dest.set(`${prefix}${name}`, pak.subarray(eoff, eoff + esize));
    }
    n++;
    if (n % 64 === 0) {
      onProgress?.(n, total);
      await yieldFrame();
    }
  }
  onProgress?.(total, total);
}

function unzipZipOnMain(buf: Uint8Array): Record<string, Uint8Array> {
  return unzipSync(buf);
}

/* Retail halflife.wad is not in the Uplink set. These names are on
   drawn faces (texinfo flags 0). Xash paints the missing-texture
   checker there: the yard lamp heads are metal_bord06 plus
   skkylightdim, and the kitchen shelf is generic007. Each stand-in
   is already in woomera.wad. clip, origin, and aaatrigger stay,
   because their texinfo flag is TEX_SPECIAL. */
const WAD_STANDIN: Record<string, string> = {
  metal_bord01: 'metal_edge',
  metal_bord06: 'metal_edge',
  skkylightdim: 'fluro',
  skkylite: 'fluro',
  generic007: 'electric',
  freezer_bx2: 'fridge_door',
  freezer_bx3: 'fridge_door',
  freezer_bx5: 'fridge_door',
  freezer_bx6: 'fridge_door',
  freezer_bx8: 'fridge_door',
  xcrate9a: 'cardboard',
  xcrate9b: 'cardboard',
  xcrate9c: 'cardboard',
  paper4: '{paper',
  paper6: '{paper',
  fiftsfile1: 'file',
  fiftsfile2b: 'file',
  out_roof1: 'roof',
  subway_seat: 'benchtop',
  rope: 'pipe',
  '{blue': 'folder_blue',
  '{grass2': '{dmcgrassb',
  '8ball1': 'greywall_plain',
  '-0out_cuby': 'sky',
  '-1out_cuby': 'sky',
  '-2out_cuby': 'sky',
  '-3out_cuby': 'sky',
  '-4out_cuby': 'sky',
  '-5out_cuby': 'sky',
};

function retargetMissingWadTextures(bsp: Uint8Array): { bsp: Uint8Array; replaced: number } {
  if (bsp.length < 124) return { bsp, replaced: 0 };
  const view = new DataView(bsp.buffer, bsp.byteOffset, bsp.byteLength);
  if (view.getInt32(0, true) !== 30) return { bsp, replaced: 0 };
  const lumpOff = view.getInt32(4 + 2 * 8, true);
  const lumpLen = view.getInt32(4 + 2 * 8 + 4, true);
  if (lumpOff < 0 || lumpLen < 4 || lumpOff + lumpLen > bsp.length)
    return { bsp, replaced: 0 };
  const count = view.getInt32(lumpOff, true);
  if (count < 1 || count > 512 || lumpOff + 4 + count * 4 > bsp.length)
    return { bsp, replaced: 0 };
  const out = bsp.slice();
  const outView = new DataView(out.buffer, out.byteOffset, out.byteLength);
  let replaced = 0;
  for (let i = 0; i < count; i++) {
    const rel = outView.getInt32(lumpOff + 4 + i * 4, true);
    if (rel < 0) continue;
    const at = lumpOff + rel;
    if (at < 0 || at + 16 > out.length) continue;
    let name = '';
    for (let c = 0; c < 16; c++) {
      const ch = out[at + c];
      if (ch === 0) break;
      name += String.fromCharCode(ch);
    }
    const next = WAD_STANDIN[name];
    if (!next || next.length > 15) continue;
    for (let c = 0; c < 16; c++) out[at + c] = 0;
    for (let c = 0; c < next.length; c++) out[at + c] = next.charCodeAt(c);
    replaced++;
  }
  return replaced ? { bsp: out, replaced } : { bsp, replaced: 0 };
}

function unzipZipInWorker(buf: Uint8Array): Promise<Record<string, Uint8Array>> {
  return new Promise((resolve, reject) => {
    let worker: Worker;
    try {
      worker = new ValveUnpackWorker();
    } catch (err) {
      reject(err);
      return;
    }
    const timer = setTimeout(() => {
      worker.terminate();
      reject(new Error('valve unzip worker timed out'));
    }, 180000);
    worker.onmessage = (ev: MessageEvent<{ names?: string[]; buffers?: ArrayBuffer[]; error?: string }>) => {
      clearTimeout(timer);
      worker.terminate();
      if (ev.data?.error) {
        reject(new Error(ev.data.error));
        return;
      }
      const names = ev.data?.names || [];
      const buffers = ev.data?.buffers || [];
      const entries: Record<string, Uint8Array> = {};
      for (let i = 0; i < names.length; i++) {
        const raw = buffers[i];
        if (raw) entries[names[i]] = new Uint8Array(raw);
      }
      resolve(entries);
    };
    worker.onerror = (err) => {
      clearTimeout(timer);
      worker.terminate();
      reject(err.error || new Error(err.message || 'valve unzip worker failed'));
    };
    const copy = buf.slice();
    worker.postMessage(copy.buffer, [copy.buffer]);
  });
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
    if (rel.toLowerCase().endsWith('.bsp')) {
      const patched = retargetMissingWadTextures(payload);
      if (patched.replaced) {
        payload = patched.bsp;
        log(`wad stand-in ${rel} ${patched.replaced}`);
      }
    }
    staged.woomera.set(`${GAME_DIR}/${out}`, payload);
    bytes += (data as Uint8Array).length;
  }
  /* evening256 shipped five faces. Xash drops the whole skybox when one
     face is missing and draws the Quake cloud sphere instead, so level 2
     never showed the dusk set. The down face is the up face: the ground
     covers it. */
  {
    const up = staged.woomera.get(`${GAME_DIR}/gfx/env/evening256up.tga`);
    const dn = `${GAME_DIR}/gfx/env/evening256dn.tga`;
    if (up && !staged.woomera.has(dn)) {
      staged.woomera.set(dn, up);
      log('sky: evening256dn copied from evening256up');
    }
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
  setVeil('Unpacking Half-Life data…', 'Inflating pak0, wads, sprites, sounds (worker).', 0.58);
  await yieldFrame();
  let entries: Record<string, Uint8Array>;
  try {
    entries = await unzipZipInWorker(buf);
    log(`valve unzip worker: ${Object.keys(entries).length} zip entries`);
  } catch (err) {
    log(`valve unzip worker failed (${formatErr(err)}) — main thread fallback`);
    setVeil('Unpacking Half-Life data…', 'Inflating on the main thread (slower).', 0.58);
    await yieldFrame();
    entries = unzipZipOnMain(buf);
  }
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
    const file = data;
    if (lower.endsWith('.pak')) {
      await explodePakYielding(file, staged.valve, 'valve/', (done, total) => {
        setVeil('Unpacking Half-Life data…', `Exploding pak0 ${done}/${total}`, 0.58);
        valveStatus.textContent = `exploding pak0 ${done}/${total}`;
      });
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

async function writeTree(
  FS: {
    mkdir: (p: string) => void;
    writeFile: (p: string, d: Uint8Array, opts?: { canOwn?: boolean }) => void;
    unlink: (p: string) => void;
  },
  files: Map<string, Uint8Array>,
  onProgress?: (done: number, total: number) => void,
) {
  const entries = [...files.entries()];
  for (let i = 0; i < entries.length; i++) {
    const [path, data] = entries[i];
    const dest = path.startsWith('/') ? path : `/${path.replace(/^\//, '')}`;
    const slash = dest.lastIndexOf('/');
    if (slash > 0) mkdirTree(FS, dest.slice(0, slash));
    try {
      FS.unlink(dest);
    } catch {
      /* not present */
    }
    try {
      const copy = data.slice();
      FS.writeFile(dest, copy, { canOwn: true });
    } catch (err) {
      throw new Error(`write ${dest} (${data.length} bytes): ${formatErr(err)}`);
    }
    if (onProgress && i % 32 === 0) onProgress(i, entries.length);
    if (i % 32 === 0) await yieldFrame();
  }
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

function pinDevicePixelRatio() {
  /* Xash multiplies the -width/-height client area by devicePixelRatio.
     This display reports 0.984375, so the 800×600 wrap became a 787×590
     backing store. FUN_10043750 then centers the 800×600 storyboard at
     ox=-6, oy=-5 and the page stretches that buffer back to the wrap,
     clipping the comic. The client area is the CSS box. */
  try {
    Object.defineProperty(window, 'devicePixelRatio', {
      configurable: true,
      get: () => 1,
    });
  } catch (err) {
    log(`dpr shim failed: ${formatErr(err)}`);
  }
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
  pinDevicePixelRatio();
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
      '1',
      '+r_drawviewmodel',
      '1',
      '+r_drawparticles',
      '0',
      '+r_norefresh',
      '0',
      '+sv_validate_changelevel',
      '0',
      '+sv_newunit',
      '1',
      '+ui_renderworld',
      '1',
      '+r_fullbright',
      '0',
      '+cl_himodels',
      '0',
    ];
    if (shimOk) {
      bootArgs.splice(1, 0, '-width', String(view.width), '-height', String(view.height));
    }
    engine = new Xash3D({
      canvas,
      renderer: 'gles3compat',
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
    (window as Window & { __efwGame?: typeof runGameCmd }).__efwGame = runGameCmd;
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
    await yieldFrame();
    await writeTree(FS, staged.valve, (done, total) =>
      setVeil('Installing game files…', `valve/ ${done}/${total}`, 0.8 + 0.1 * (done / Math.max(1, total))),
    );
    log('boot: valve files written');
    await writeTree(FS, staged.woomera);
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
    firstMapKeyGame = false;
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
/* FUN_10048710: a click on the interact bar opens the command buttons
   (efw_pause 1, FUN_10046370). The next click dismisses them. The first
   press only locks the pointer; the game click is the one after that. */
let promptContext = false;
canvas.addEventListener('pointerdown', (ev) => {
  canvas.focus();
  const caught = ev.button === 0 && hideStoryCatcher();
  if (ev.button === 0)
    runGameCmd('efw_story_key');
  if (caught) {
    dismissEfwStory();
    return;
  }
  if (ev.button !== 0 || document.pointerLockElement !== canvas) return;
  promptContext = !promptContext;
  runGameCmd(promptContext ? 'efw_context 1' : 'efw_context 0');
});
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
document.getElementById('efw-letter')?.addEventListener('click', (ev) => {
  ev.preventDefault();
  ev.stopPropagation();
  dismissLetterbox();
  /* Caption InputSignal: EFW_DismissCaption on mouse1. */
  runGameCmd('efw_story_key');
});
document.getElementById('btn-talk')?.addEventListener('click', () => {
  log('> talk (efw_Talk Amir)');
  runEngineCmd('pausable 0');
  runGameCmd('efw_Talk Amir');
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
document.getElementById('efw-diary')?.addEventListener('click', (ev) => {
  const btn = (ev.target as HTMLElement).closest('button[data-diary]') as HTMLButtonElement | null;
  if (!btn) return;
  ev.preventDefault();
  const which = btn.dataset.diary;
  const cmd = which === 'prev' ? 'efw_diary_prev' : which === 'next' ? 'efw_diary_next' : 'efw_diary';
  log(`> ${cmd}`);
  runEngineCmd('pausable 0');
  runGameCmd(cmd);
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
const walkKeys = { w: false, a: false, s: false, d: false };
function walkSlot(key: string): keyof typeof walkKeys | null {
  if (key === 'w' || key === 'W' || key === 'ArrowUp') return 'w';
  if (key === 's' || key === 'S' || key === 'ArrowDown') return 's';
  if (key === 'a' || key === 'A') return 'a';
  if (key === 'd' || key === 'D') return 'd';
  return null;
}
function syncWalkLatch() {
  const fwd = (walkKeys.w ? 1 : 0) + (walkKeys.s ? -1 : 0);
  const side = (walkKeys.d ? 1 : 0) + (walkKeys.a ? -1 : 0);
  /* CL_CreateMove writes efw_pmove into the usercmd. Libmenu pauses the
     listen server, so that usercmd never lands on pev->button and PM_Move
     never spends it. The host pump walks from efw_move. */
  runGameCmd(`efw_move ${fwd} ${side}`);
  runGameCmd(`efw_pmove ${fwd} ${side}`);
}
function syncJump(on: boolean) {
  /* Listen-server console runs the client command. Stufftext from
     efw_cjump does not reach CL_CreateMove. */
  runGameCmd(`efw_pjump ${on ? 1 : 0}`);
}
function syncDuck(on: boolean) {
  runGameCmd(`efw_pduck ${on ? 1 : 0}`);
}
function syncSpeed(on: boolean) {
  /* +speed is SHIFT in config.cfg. The pump rebuilds the wish from
     cl_forwardspeed, so the key has to reach the server as efw_pspeed. */
  runGameCmd(`efw_pspeed ${on ? 1 : 0}`);
}
function syncUseHold(on: boolean) {
  /* +use on the ground cuts maxspeed to a third. The pump does not read
     the usercmd magnitude, so the hold has to reach the server. */
  runGameCmd(`efw_puse ${on ? 1 : 0}`);
}

/* Coalesce pointer deltas to one usercmd look per frame. The client DLL
   adds them to cmd->viewangles; PM_Move turns. efw_turn remains the
   server latch for anything that still calls it directly. */
const lookPending = { yaw: 0, pitch: 0 };
let lookFlushQueued = false;
function queueLook(yaw: number, pitch: number) {
  if (!yaw && !pitch) return;
  lookPending.yaw += yaw;
  lookPending.pitch += pitch;
  if (lookFlushQueued) return;
  lookFlushQueued = true;
  requestAnimationFrame(() => {
    lookFlushQueued = false;
    const y = lookPending.yaw;
    const p = lookPending.pitch;
    lookPending.yaw = 0;
    lookPending.pitch = 0;
    if (!y && !p) return;
    /* Same console path as efw_pmove. efw_clook's stufftext never reaches
       CL_CreateMove, so the view delta is applied here. */
    runGameCmd(`efw_plook ${y.toFixed(3)} ${p.toFixed(3)}`);
  });
}

document.addEventListener('pointermove', notePromptPointer);

document.addEventListener('mousemove', (e) => {
  if (!inputCaptured()) return;
  if (!e.movementX && !e.movementY) return;
  queueLook(-e.movementX * 0.08, e.movementY * 0.08);
});

document.addEventListener('keydown', (e) => {
  if (e.target === consoleInput || e.target instanceof HTMLInputElement)
    return;
  if (!e.repeat) {
    const caught = hideStoryCatcher();
    runGameCmd('efw_story_key');
    if (caught) {
      dismissEfwStory();
      return;
    }
  }
  if (e.code === 'Space') {
    e.preventDefault();
    if (e.repeat) return;
    syncJump(true);
    return;
  }
  if (e.code === 'ControlLeft' || e.code === 'ControlRight') {
    e.preventDefault();
    if (e.repeat) return;
    syncDuck(true);
    return;
  }
  if (e.code === 'ShiftLeft' || e.code === 'ShiftRight') {
    e.preventDefault();
    if (e.repeat) return;
    syncSpeed(true);
    return;
  }
  const walk = walkSlot(e.key);
  if (walk) {
    if (e.repeat) return;
    walkKeys[walk] = true;
    syncWalkLatch();
    return;
  }
  if (e.repeat)
    return;
  if (e.key === 'ArrowLeft' || e.key === 'q' || e.key === 'Q') {
    queueLook(12, 0);
    return;
  }
  if (e.key === 'ArrowRight' || e.key === 'z' || e.key === 'Z') {
    queueLook(-12, 0);
    return;
  }
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
    syncUseHold(true);
    runGameCmd('efw_inuse');
    runGameCmd('use');
  } else if (e.key === 'i' || e.key === 'I') {
    document.getElementById('btn-diary')?.click();
  }
});
document.addEventListener('keyup', (e) => {
  if (e.target === consoleInput || e.target instanceof HTMLInputElement)
    return;
  if (e.code === 'Space') {
    syncJump(false);
    return;
  }
  if (e.code === 'ControlLeft' || e.code === 'ControlRight') {
    syncDuck(false);
    return;
  }
  if (e.code === 'ShiftLeft' || e.code === 'ShiftRight') {
    syncSpeed(false);
    return;
  }
  if (e.key === 'e' || e.key === 'E') {
    syncUseHold(false);
    return;
  }
  const walk = walkSlot(e.key);
  if (!walk) return;
  walkKeys[walk] = false;
  syncWalkLatch();
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
