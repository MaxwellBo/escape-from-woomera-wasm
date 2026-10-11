import './style.css';

const frame = document.getElementById('game-frame') as HTMLIFrameElement;
const wineStatus = document.getElementById('wine-status') as HTMLSpanElement;
const gameStatus = document.getElementById('game-status') as HTMLSpanElement;
const launchStatus = document.getElementById('launch-status') as HTMLSpanElement;
const engineStatus = document.getElementById('engine-status') as HTMLElement;
const stepWine = document.getElementById('step-wine') as HTMLDivElement;
const stepGame = document.getElementById('step-game') as HTMLDivElement;
const stepLaunch = document.getElementById('step-launch') as HTMLDivElement;
const logEl = document.getElementById('log') as HTMLPreElement;
const logCount = document.getElementById('log-count') as HTMLSpanElement;

function publicAsset(path: string): string {
  return `${import.meta.env.BASE_URL}${path.replace(/^\//, '')}`;
}

let logLines = 0;
let sawWine = false;
let sawGame = false;

function log(line: string) {
  const text = line.replace(/\s+$/, '');
  if (!text)
    return;
  logLines += 1;
  logCount.textContent = String(logLines);
  logEl.textContent += (logEl.textContent ? '\n' : '') + text;
  logEl.scrollTop = logEl.scrollHeight;
  if (/boxedwine\.zip|Wine 11|Downloading/i.test(text) || /setting root zip/i.test(text)) {
    sawWine = true;
    wineStatus.textContent = 'loading filesystem…';
    engineStatus.textContent = 'loading Wine';
  }
  if (/woomera\.zip|setting app zip/i.test(text)) {
    sawGame = true;
    gameStatus.textContent = 'loading Windows build…';
  }
  if (/Emulator params/i.test(text)) {
    stepWine.classList.add('done');
    stepGame.classList.add('done');
    wineStatus.textContent = 'ready';
    gameStatus.textContent = 'mounted';
    launchStatus.textContent = 'starting xash3d.exe';
    engineStatus.textContent = 'booting';
    stepLaunch.classList.add('done');
  }
}

window.addEventListener('message', (ev) => {
  const data = ev.data as { source?: string; line?: string; status?: string } | null;
  if (!data || data.source !== 'boxedwine')
    return;
  if (typeof data.line === 'string')
    log(data.line);
  if (typeof data.status === 'string' && data.status) {
    launchStatus.textContent = data.status;
    if (!sawWine) {
      wineStatus.textContent = data.status;
      engineStatus.textContent = 'loading Wine';
    } else if (!sawGame) {
      gameStatus.textContent = data.status;
    }
  }
});

function openOverlay(id: string) {
  document.getElementById(id)?.classList.remove('hidden');
}
function closeOverlay(id: string) {
  document.getElementById(id)?.classList.add('hidden');
}

document.getElementById('btn-about')?.addEventListener('click', () => openOverlay('about'));
document.getElementById('btn-help')?.addEventListener('click', () => openOverlay('help'));
document.getElementById('btn-close-about')?.addEventListener('click', () => closeOverlay('about'));
document.getElementById('btn-close-help')?.addEventListener('click', () => closeOverlay('help'));

wineStatus.textContent = 'starting…';
gameStatus.textContent = 'queued';
launchStatus.textContent = 'booting Wine';
engineStatus.textContent = 'loading';

const params = new URLSearchParams({
  root: 'wine6',
  app: 'woomera',
  p: 'run.bat',
  resolution: '800x600',
  // Sound stays off. A placeholder wmic.exe was what aborted startup;
  // the web audio device is not required to boot and is left unused.
  sound: 'false',
  skipFrameFPS: '20',
});
frame.src = `${publicAsset('boxedwine/boxedwine.html')}?${params.toString()}`;
