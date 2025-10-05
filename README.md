# Astro + SolidJS + Cloudflare Template

A modern web application template combining Astro, SolidJS, Tailwind CSS v4, and Cloudflare Workers.

## Features

- **Astro** - Fast, content-focused web framework
- **SolidJS** - Reactive UI library with fine-grained reactivity
- **Tailwind CSS v4** - Utility-first CSS framework with dark mode support
- **Cloudflare Workers** - Edge deployment with access to D1, KV, R2, and more

## Getting Started

### Development

```bash
npm install
npm run dev
```

Visit `http://localhost:4321` (or the port shown in terminal)

### Building

```bash
npm run build
npm run preview
```

### Deployment to Cloudflare

1. Configure your `wrangler.toml` with your Cloudflare bindings
2. Deploy with:

```bash
npm run build
npx wrangler deploy
```

## Project Structure

```
/
├── public/
├── src/
│   ├── components_solid/
│   │   ├── Counter.tsx      # SolidJS demo component
│   │   └── DarkModeToggle.tsx # Dark mode toggle
│   ├── layouts/
│   │   └── Layout.astro     # Main layout
│   ├── pages/
│   │   ├── api/
│   │   │   └── hello.ts # API route example
│   │   ├── index.astro      # Home page
│   │   ├── about.astro      # About page
│   │   ├── demo.astro       # SolidJS demo
│   │   └── cloudflare.astro # Cloudflare features
│   └── styles/
│       └── global.css       # Global styles with Tailwind
├── astro.config.mjs
├── package.json
├── tsconfig.json
└── wrangler.toml           # Cloudflare configuration
```

## Using Cloudflare Features

Uncomment and configure the bindings in `wrangler.toml`:

- **D1 Database**: SQLite database at the edge
- **KV Storage**: Key-value storage
- **R2 Storage**: Object storage (S3-compatible)
- **Durable Objects**: Stateful serverless computing

Access these in your API routes through `locals.runtime.env`.

## Adding SolidJS Components

Create `.tsx` files in the `components_solid` directory and use them in your Astro pages with the `client:*` directive:

```astro
---
import MyComponent from 'src/components_solid/MyComponent';
---

<MyComponent client:load />
```

## Dark Mode

The template includes automatic dark mode support. Users can toggle between light and dark themes using the floating button in the top-right corner.
# scalable-frontend
