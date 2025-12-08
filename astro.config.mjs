// @ts-check
import { defineConfig } from 'astro/config'
import cloudflare from '@astrojs/cloudflare'
import solidJs from '@astrojs/solid-js'
import svelte from '@astrojs/svelte'
import qwikdev from '@qwikdev/astro'
import mdx from '@astrojs/mdx'
import tailwindcss from "@tailwindcss/vite"
import wasm from "vite-plugin-wasm"
import topLevelAwait from "vite-plugin-top-level-await"

// https://astro.build/config
export default defineConfig({
  output: 'server',
  server: {
    port: 8443,
    host: true
  },
  integrations: [
    mdx(),
    qwikdev({
      include: [
        '**/components_qwik/*',
        '**/applications_qwik/**/*'
      ]
    }),
    solidJs({
      devtools: true,
      // all SolidJS components will be put inside 'components_solid' folder if ever other TSX frameworks are ever
      // added to this astro project
      include: [
        '**/components_solid/*',
        '**/applications_solid/**/*'
      ]
    }),
    svelte()
  ],
  adapter: cloudflare({
    platformProxy: {
      enabled: true,
    },
  }),
  vite: {
    plugins: [
      // DO NOT CHANGE THIS, OFFICIAL DOCS: https://tailwindcss.com/docs/installation/framework-guides/astro
      // THIS **IS** 100% the correct way of adding TailwindCSS to an Astro project
      // @ts-ignore
      tailwindcss(),
      wasm(),
      topLevelAwait(),
    ],
    define: {
      'process.env.BASE_URL': JSON.stringify('https://localhost:2053')
    },
    resolve: {
      conditions: [
        'import',
        'module',
        'browser',
        'default'
      ],
    },
    ssr: {
      external: [
        'node:*'
      ]
    },
    server: {
      cors: {
        origin: '*'
      }
    }
  }
})
