use leptos::*;
use leptos_router::*;
use wasm_bindgen::prelude::*;
use wasm_bindgen::JsCast;

// Theme management hook
#[derive(Clone, Copy)]
struct DarkMode(RwSignal<bool>);

fn use_dark_mode() -> (ReadSignal<bool>, WriteSignal<bool>) {
    let dark_mode = use_context::<DarkMode>()
        .expect("DarkMode context not found")
        .0;
    dark_mode.split()
}

fn initialize_dark_mode() -> bool {
    if let Some(window) = web_sys::window() {
        // Check localStorage first
        if let Ok(Some(storage)) = window.local_storage() {
            if let Ok(Some(stored)) = storage.get_item("darkMode") {
                return stored == "true";
            }
        }

        // Check document element class
        if let Some(document) = window.document() {
            if let Some(html) = document.document_element() {
                if html.class_list().contains("dark") {
                    return true;
                }
            }
        }

        // Check system preference
        if let Ok(media_query) = window.match_media("(prefers-color-scheme: dark)") {
            if let Some(query) = media_query {
                return query.matches();
            }
        }
    }
    false
}

fn update_theme(is_dark: bool) {
    if let Some(window) = web_sys::window() {
        // Update localStorage
        if let Ok(Some(storage)) = window.local_storage() {
            let _ = storage.set_item("darkMode", &is_dark.to_string());
        }

        // Update document element class
        if let Some(document) = window.document() {
            if let Some(html) = document.document_element() {
                if is_dark {
                    let _ = html.class_list().add_1("dark");
                } else {
                    let _ = html.class_list().remove_1("dark");
                }
            }
        }
    }
}

#[component]
fn Dashboard() -> impl IntoView {
    let (is_dark, _) = use_dark_mode();

    let card_class = move || {
        if is_dark.get() {
            "bg-zinc-800 p-6 rounded-lg"
        } else {
            "bg-white p-6 rounded-lg shadow-md"
        }
    };

    view! {
        <div class="p-8">
            <h1 class="text-3xl font-bold mb-6">"Dashboard"</h1>
            <div class="grid grid-cols-1 md:grid-cols-3 gap-6">
                <div class=card_class>
                    <h2 class="text-xl font-semibold mb-2">"Total Users"</h2>
                    <p class="text-4xl font-bold text-blue-400">"1,234"</p>
                </div>
                <div class=card_class>
                    <h2 class="text-xl font-semibold mb-2">"Active Sessions"</h2>
                    <p class="text-4xl font-bold text-green-400">"567"</p>
                </div>
                <div class=card_class>
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
    let (is_dark, _) = use_dark_mode();

    let container_class = move || {
        if is_dark.get() {
            "bg-zinc-800 p-8 rounded-lg max-w-md"
        } else {
            "bg-white p-8 rounded-lg max-w-md shadow-md"
        }
    };

    let reset_class = move || {
        if is_dark.get() {
            "px-6 py-3 bg-zinc-600 hover:bg-zinc-700 rounded-lg font-semibold transition"
        } else {
            "px-6 py-3 bg-gray-200 hover:bg-gray-300 text-gray-700 rounded-lg font-semibold transition"
        }
    };

    view! {
        <div class="p-8">
            <h1 class="text-3xl font-bold mb-6">"WASM Counter"</h1>
            <div class=container_class>
                <p class="text-6xl font-bold text-center mb-6">{count}</p>
                <div class="flex gap-4 justify-center">
                    <button
                        on:click=move |_| set_count.update(|n| *n -= 1)
                        class="px-6 py-3 bg-red-500 hover:bg-red-600 text-white rounded-lg font-semibold transition"
                    >
                        "Decrement"
                    </button>
                    <button
                        on:click=move |_| set_count.set(0)
                        class=reset_class
                    >
                        "Reset"
                    </button>
                    <button
                        on:click=move |_| set_count.update(|n| *n += 1)
                        class="px-6 py-3 bg-green-500 hover:bg-green-600 text-white rounded-lg font-semibold transition"
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
    let (is_dark, _) = use_dark_mode();

    let subtitle_class = move || {
        if is_dark.get() {
            "text-xl text-zinc-400 mb-8"
        } else {
            "text-xl text-gray-600 mb-8"
        }
    };

    view! {
        <div class="p-8">
            <h1 class="text-4xl font-bold mb-4">"Welcome to WASM SPA"</h1>
            <p class=subtitle_class>
                "This is a Leptos-powered WebAssembly single-page application running in Astro!"
            </p>
            <div class="flex gap-4">
                <A href="/leptos-spa/dashboard" class="px-6 py-3 bg-blue-500 hover:bg-blue-600 text-white rounded-lg font-semibold transition">
                    "Go to Dashboard"
                </A>
                <A href="/leptos-spa/counter" class="px-6 py-3 bg-purple-500 hover:bg-purple-600 text-white rounded-lg font-semibold transition">
                    "Try Counter"
                </A>
            </div>
        </div>
    }
}

#[component]
fn Navigation() -> impl IntoView {
    let (is_dark, set_dark) = use_dark_mode();

    let nav_class = move || {
        if is_dark.get() {
            "bg-zinc-800 border-b border-zinc-700 px-6 py-4"
        } else {
            "bg-white border-b border-gray-200 px-6 py-4"
        }
    };

    let toggle_theme = move |_| {
        let new_dark = !is_dark.get();
        set_dark.set(new_dark);
        update_theme(new_dark);
    };

    view! {
        <nav class=nav_class>
            <div class="flex items-center justify-between">
                <h1 class="text-xl font-bold">"WASM SPA"</h1>
                <div class="flex items-center gap-6">
                    <A href="/leptos-spa" class="hover:text-blue-400 transition">"Home"</A>
                    <A href="/leptos-spa/dashboard" class="hover:text-blue-400 transition">"Dashboard"</A>
                    <A href="/leptos-spa/counter" class="hover:text-blue-400 transition">"Counter"</A>

                    // Theme toggle button
                    <button
                        on:click=toggle_theme
                        class="p-2 rounded-lg transition-colors hover:bg-zinc-700 dark:hover:bg-zinc-600"
                        title="Toggle theme"
                    >
                        {move || if is_dark.get() {
                            view! {
                                <svg class="w-5 h-5 text-yellow-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"></path>
                                </svg>
                            }
                        } else {
                            view! {
                                <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"></path>
                                </svg>
                            }
                        }}
                    </button>
                </div>
            </div>
        </nav>
    }
}

#[component]
fn App() -> impl IntoView {
    // Initialize dark mode from localStorage/system preference/DOM
    let is_dark = initialize_dark_mode();
    let dark_mode = create_rw_signal(is_dark);
    provide_context(DarkMode(dark_mode));

    // Create derived signal for dynamic classes
    let bg_class = move || {
        if dark_mode.get() {
            "min-h-screen bg-zinc-900 text-zinc-100"
        } else {
            "min-h-screen bg-gray-50 text-gray-900"
        }
    };

    view! {
        <Router>
            <div class=bg_class>
                <Navigation />
                <Routes>
                    <Route path="/leptos-spa" view=Home/>
                    <Route path="/leptos-spa/dashboard" view=Dashboard/>
                    <Route path="/leptos-spa/counter" view=Counter/>
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
