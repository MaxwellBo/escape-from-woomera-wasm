import fs from 'node:fs';
import { defineConfig, type Plugin } from 'vite';

declare const process: { env: Record<string, string | undefined> };

/**
 * Host_Frame traps on a null VGUI indirect call (XashSurface::pushMakeCurrent).
 * Emscripten's handleException turns that into quit_(), which drops the rAF
 * runner. The listen server keeps ticking via efw_pump, but the canvas stays
 * on the last barracks frame and the view never follows the player.
 * Swallow that trap so the next frame still presents.
 */
function keepXashMainLoop(): Plugin {
  const needle = '} quit_(1, e); };';
  const patch = '} console.error("efw: kept main loop after", e); };';
  return {
    name: 'keep-xash-main-loop',
    enforce: 'pre',
    config() {
      return {
        optimizeDeps: {
          esbuildOptions: {
            plugins: [
              {
                name: 'keep-xash-main-loop',
                setup(build) {
                  build.onLoad({ filter: /generated[\\/]xash\.js$/ }, (args) => {
                    const raw = fs.readFileSync(args.path, 'utf8');
                    if (!raw.includes(needle))
                      throw new Error(`xash handleException pattern missing in ${args.path}`);
                    return { contents: raw.replace(needle, patch), loader: 'js' };
                  });
                },
              },
            ],
          },
        },
      };
    },
    transform(code, id) {
      const norm = id.replace(/\\/g, '/');
      if (!norm.includes('generated/xash.js'))
        return null;
      if (!code.includes(needle))
        throw new Error(`xash handleException pattern missing in ${id}`);
      return code.replace(needle, patch);
    },
  };
}

/** GitHub Pages project sites need a trailing-slash base like `/repo/`. */
function pagesBase(value: string | undefined): string {
  if (!value || value === '/') return '/';
  const withLead = value.startsWith('/') ? value : `/${value}`;
  return withLead.endsWith('/') ? withLead : `${withLead}/`;
}

export default defineConfig({
  plugins: [keepXashMainLoop()],
  base: pagesBase(process.env.PAGES_BASE),
  server: {
    port: 47831,
    host: '127.0.0.1',
    watch: {
      ignored: ['**/third_party/**', '**/node_modules/**'],
    },
  },
  preview: {
    port: 47831,
    host: '127.0.0.1',
  },
  build: {
    target: 'es2020',
  },
});

