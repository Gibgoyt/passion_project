use leptos::*;
use leptos_router::*;

#[derive(Clone, Copy)]
pub struct DarkMode(pub RwSignal<bool>);

pub fn use_dark_mode() -> (ReadSignal<bool>, WriteSignal<bool>) {
    let dark_mode = use_context::<DarkMode>()
        .expect("DarkMode context not found")
        .0;
    dark_mode.split()
}

pub fn update_theme(is_dark: bool) {
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
pub fn Sidebar() -> impl IntoView {
    let (is_dark, set_dark) = use_dark_mode();

    let sidebar_class = move || {
        if is_dark.get() {
            "w-64 h-screen bg-zinc-800 border-r border-zinc-700 p-6 flex flex-col"
        } else {
            "w-64 h-screen bg-white border-r border-gray-200 p-6 flex flex-col"
        }
    };

    let link_class = move |is_active: bool| {
        let base = "block px-4 py-3 rounded-lg transition-colors mb-2";
        if is_active {
            if is_dark.get() {
                format!("{} bg-blue-600 text-white font-semibold", base)
            } else {
                format!("{} bg-blue-500 text-white font-semibold", base)
            }
        } else {
            if is_dark.get() {
                format!("{} hover:bg-zinc-700 text-zinc-300", base)
            } else {
                format!("{} hover:bg-gray-100 text-gray-700", base)
            }
        }
    };

    let toggle_theme = move |_| {
        let new_dark = !is_dark.get();
        set_dark.set(new_dark);
        update_theme(new_dark);
    };

    let navigate_home = move |_| {
        if let Some(window) = web_sys::window() {
            if let Some(location) = window.location().href().ok() {
                // Force full page reload to escape Leptos router and use Astro router
                let _ = window.location().set_href("/");
            }
        }
    };

    let button_class = move || {
        if is_dark.get() {
            "w-full p-3 mb-2 rounded-lg transition-colors hover:bg-zinc-700 dark:hover:bg-zinc-600 flex items-center justify-center gap-2"
        } else {
            "w-full p-3 mb-2 rounded-lg transition-colors hover:bg-gray-200 flex items-center justify-center gap-2"
        }
    };

    view! {
        <aside class=sidebar_class>
            <div class="mb-8">
                <h1 class="text-2xl font-bold">"🔒 Admin Panel"</h1>
            </div>

            <nav class="flex-1">
                <A href="/admin/dashboard" class=move || link_class(false)>
                    "📊 Dashboard"
                </A>
                <A href="/admin/crm" class=move || link_class(false)>
                    "👥 CRM"
                </A>
            </nav>

            <div class="mt-auto pt-6 border-t border-zinc-700">
                <button
                    on:click=navigate_home
                    class=button_class
                    title="Back to Home"
                >
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6"></path>
                    </svg>
                    <span>"Back to Home"</span>
                </button>
                <button
                    on:click=toggle_theme
                    class="w-full p-3 rounded-lg transition-colors hover:bg-zinc-700 dark:hover:bg-zinc-600 flex items-center justify-center gap-2"
                    title="Toggle theme"
                >
                    {move || if is_dark.get() {
                        view! {
                            <svg class="w-5 h-5 text-yellow-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"></path>
                            </svg>
                            <span>"Light Mode"</span>
                        }
                    } else {
                        view! {
                            <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"></path>
                            </svg>
                            <span>"Dark Mode"</span>
                        }
                    }}
                </button>
            </div>
        </aside>
    }
}
