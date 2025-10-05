declare module "astro:actions" {
	type Actions = typeof import("/Users/joshsack/Projects_Pritchard/Astro/cloudflare_frontend/src/actions/index.ts")["server"];

	export const actions: Actions;
}