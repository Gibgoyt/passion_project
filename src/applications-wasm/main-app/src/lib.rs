use leptos::*;
use leptos_router::*;
use wasm_bindgen::prelude::*;
use wasm_bindgen::JsCast;

#[component]
fn Dashboard() -> impl IntoView {
    view! {
        <div class="p-8">
            <h1 class="text-3xl font-bold mb-6">"Dashboard"</h1>
            <div class="grid grid-cols-1 md:grid-cols-3 gap-6">
                <div class="bg-zinc-800 p-6 rounded-lg">
                    <h2 class="text-xl font-semibold mb-2">"Total Users"</h2>
                    <p class="text-4xl font-bold text-blue-400">"1,234"</p>
                </div>
                <div class="bg-zinc-800 p-6 rounded-lg">
                    <h2 class="text-xl font-semibold mb-2">"Active Sessions"</h2>
                    <p class="text-4xl font-bold text-green-400">"567"</p>
                </div>
                <div class="bg-zinc-800 p-6 rounded-lg">
                    <h2 class="text-xl font-semibold mb-2">"WASM Performance"</h2>
                    <p class="text-4xl font-bold text-purple-400">"Fast!"</p>
                </div>
            </div>
        </div>
    }
}

#[component]
fn Counter() -> impl IntoView {
    let (count, set_count) = create_signal(0);

    view! {
        <div class="p-8">
            <h1 class="text-3xl font-bold mb-6">"WASM Counter"</h1>
            <div class="bg-zinc-800 p-8 rounded-lg max-w-md">
                <p class="text-6xl font-bold text-center mb-6">{count}</p>
                <div class="flex gap-4 justify-center">
                    <button
                        on:click=move |_| set_count.update(|n| *n -= 1)
                        class="px-6 py-3 bg-red-500 hover:bg-red-600 rounded-lg font-semibold transition"
                    >
                        "Decrement"
                    </button>
                    <button
                        on:click=move |_| set_count.set(0)
                        class="px-6 py-3 bg-zinc-600 hover:bg-zinc-700 rounded-lg font-semibold transition"
                    >
                        "Reset"
                    </button>
                    <button
                        on:click=move |_| set_count.update(|n| *n += 1)
                        class="px-6 py-3 bg-green-500 hover:bg-green-600 rounded-lg font-semibold transition"
                    >
                        "Increment"
                    </button>
                </div>
            </div>
        </div>
    }
}

#[component]
fn Home() -> impl IntoView {
    view! {
        <div class="p-8">
            <h1 class="text-4xl font-bold mb-4">"Welcome to WASM SPA"</h1>
            <p class="text-xl text-zinc-400 mb-8">
                "This is a Leptos-powered WebAssembly single-page application running in Astro!"
            </p>
            <div class="flex gap-4">
                <A href="/wasm/dashboard" class="px-6 py-3 bg-blue-500 hover:bg-blue-600 rounded-lg font-semibold transition">
                    "Go to Dashboard"
                </A>
                <A href="/wasm/counter" class="px-6 py-3 bg-purple-500 hover:bg-purple-600 rounded-lg font-semibold transition">
                    "Try Counter"
                </A>
            </div>
        </div>
    }
}

#[component]
fn Navigation() -> impl IntoView {
    view! {
        <nav class="bg-zinc-800 border-b border-zinc-700 px-6 py-4">
            <div class="flex items-center justify-between">
                <h1 class="text-xl font-bold">"WASM SPA"</h1>
                <div class="flex gap-6">
                    <A href="/wasm" class="hover:text-blue-400 transition">"Home"</A>
                    <A href="/wasm/dashboard" class="hover:text-blue-400 transition">"Dashboard"</A>
                    <A href="/wasm/counter" class="hover:text-blue-400 transition">"Counter"</A>
                </div>
            </div>
        </nav>
    }
}

#[component]
fn App() -> impl IntoView {
    view! {
        <Router>
            <div class="min-h-screen bg-zinc-900 text-zinc-100">
                <Navigation />
                <Routes>
                    <Route path="/wasm" view=Home/>
                    <Route path="/wasm/dashboard" view=Dashboard/>
                    <Route path="/wasm/counter" view=Counter/>
                </Routes>
            </div>
        </Router>
    }
}

#[wasm_bindgen]
pub fn mount_app() -> Result<(), JsValue> {
    console_error_panic_hook::set_once();

    let container = web_sys::window()
        .ok_or_else(|| JsValue::from_str("No window found"))?
        .document()
        .ok_or_else(|| JsValue::from_str("No document found"))?
        .get_element_by_id("wasm-root")
        .ok_or_else(|| JsValue::from_str("No #wasm-root element found"))?
        .dyn_into::<web_sys::HtmlElement>()?;

    mount_to(container, || view! { <App/> });

    Ok(())
}
