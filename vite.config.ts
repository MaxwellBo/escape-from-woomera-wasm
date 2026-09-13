import { defineConfig } from 'vite';

export default defineConfig({
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
