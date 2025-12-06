import { defineConfig } from 'astro/config';
import tailwindcss from '@tailwindcss/vite';
import svelte from '@astrojs/svelte';

export default defineConfig({
  output: 'static', // Pure frontend - no server!
  integrations: [svelte()],
  server: {
    port: 3000,
    host: 'localhost'
  },
  vite: {
    plugins: [tailwindcss()],
    define: {
      // Browser-compatible replacements for Node.js APIs used by ts_testing
      'process.env.BASE_URL': JSON.stringify('https://localhost:2053'),
      'process.env["NODE_TLS_REJECT_UNAUTHORIZED"]': JSON.stringify('0'),
      'process.env': '{}',
      'global': 'globalThis'
    }
  }
});
