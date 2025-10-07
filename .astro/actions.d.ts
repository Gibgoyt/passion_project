declare module "astro:actions" {
	type Actions = typeof import("/Users/ahmed/Projects_Josam/Astro/pritchard_cloudflare_frontend/src/actions/index.ts")["server"];

	export const actions: Actions;
}