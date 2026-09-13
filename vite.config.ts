import { defineConfig } from 'vite';

declare const process: { env: Record<string, string | undefined> };

/** GitHub Pages project sites need a trailing-slash base like `/repo/`. */
function pagesBase(value: string | undefined): string {
  if (!value || value === '/') return '/';
  const withLead = value.startsWith('/') ? value : `/${value}`;
  return withLead.endsWith('/') ? withLead : `${withLead}/`;
}

export default defineConfig({
  base: pagesBase(process.env.PAGES_BASE),
  server: {
    port: 47831,
    host: '127.0.0.1',
  },
  preview: {
    port: 47831,
    host: '127.0.0.1',
  },
  build: {
    target: 'es2020',
  },
});

