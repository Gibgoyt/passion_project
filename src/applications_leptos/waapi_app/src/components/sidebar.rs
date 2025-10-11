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
    let location = use_location();

    // Navy blue sidebar - matching the design mockup
    let sidebar_class = "w-64 h-screen bg-[#0f172a] border-r border-[#1e293b] p-6 flex flex-col";

    let is_active = move |path: &str| {
        let current = location.pathname.get();
        current == path || (path == "/waapi/dashboard" && current == "/waapi")
    };

    let link_class = move |path: &str| {
        let base = "flex items-center gap-3 px-4 py-3 rounded-lg transition-colors mb-2 text-sm font-medium";
        if is_active(path) {
            format!("{} bg-green-600 text-white", base)
        } else {
            format!("{} text-gray-300 hover:bg-[#1e293b]", base)
        }
    };

    let toggle_theme = move |_| {
        let new_dark = !is_dark.get();
        set_dark.set(new_dark);
        update_theme(new_dark);
    };

    let navigate_home = move |_| {
        if let Some(window) = web_sys::window() {
            let _ = window.location().set_href("/");
        }
    };

    let button_class = "w-full p-3 mb-2 rounded-lg transition-colors hover:bg-[#1e293b] flex items-center justify-center gap-2 text-gray-300 text-sm";

    view! {
        <aside class=sidebar_class>
            // Branding
            <div class="mb-8">
                <div class="flex items-center gap-2">
                    // WhatsApp icon
                    <svg class="w-8 h-8 text-green-500" fill="currentColor" viewBox="0 0 24 24">
                        <path d="M17.472 14.382c-.297-.149-1.758-.867-2.03-.967-.273-.099-.471-.148-.67.15-.197.297-.767.966-.94 1.164-.173.199-.347.223-.644.075-.297-.15-1.255-.463-2.39-1.475-.883-.788-1.48-1.761-1.653-2.059-.173-.297-.018-.458.13-.606.134-.133.298-.347.446-.52.149-.174.198-.298.298-.497.099-.198.05-.371-.025-.52-.075-.149-.669-1.612-.916-2.207-.242-.579-.487-.5-.669-.51-.173-.008-.371-.01-.57-.01-.198 0-.52.074-.792.372-.272.297-1.04 1.016-1.04 2.479 0 1.462 1.065 2.875 1.213 3.074.149.198 2.096 3.2 5.077 4.487.709.306 1.262.489 1.694.625.712.227 1.36.195 1.871.118.571-.085 1.758-.719 2.006-1.413.248-.694.248-1.289.173-1.413-.074-.124-.272-.198-.57-.347m-5.421 7.403h-.004a9.87 9.87 0 01-5.031-1.378l-.361-.214-3.741.982.998-3.648-.235-.374a9.86 9.86 0 01-1.51-5.26c.001-5.45 4.436-9.884 9.888-9.884 2.64 0 5.122 1.03 6.988 2.898a9.825 9.825 0 012.893 6.994c-.003 5.45-4.437 9.884-9.885 9.884m8.413-18.297A11.815 11.815 0 0012.05 0C5.495 0 .16 5.335.157 11.892c0 2.096.547 4.142 1.588 5.945L.057 24l6.305-1.654a11.882 11.882 0 005.683 1.448h.005c6.554 0 11.89-5.335 11.893-11.893a11.821 11.821 0 00-3.48-8.413Z"/>
                    </svg>
                    <h1 class="text-xl font-bold text-white">"WaAPI Manager"</h1>
                </div>
            </div>

            // Navigation menu
            <nav class="flex-1">
                <A href="/waapi/dashboard" class=move || link_class("/waapi/dashboard")>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6"></path>
                    </svg>
                    <span>"Dashboard"</span>
                </A>

                <A href="/waapi/accounts" class=move || link_class("/waapi/accounts")>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0zm6 3a2 2 0 11-4 0 2 2 0 014 0zM7 10a2 2 0 11-4 0 2 2 0 014 0z"></path>
                    </svg>
                    <span>"Accounts"</span>
                </A>
            </nav>

            // Bottom section: User info + controls
            <div class="mt-auto pt-6 border-t border-[#1e293b]">
                // User info
                <div class="flex items-center gap-3 mb-4 px-2">
                    <div class="w-8 h-8 rounded-full bg-gradient-to-br from-green-500 to-emerald-600 flex items-center justify-center text-white text-xs font-semibold">
                        "WA"
                    </div>
                    <div class="flex-1">
                        <div class="text-sm font-medium text-white">"WhatsApp"</div>
                        <div class="text-xs text-gray-400">"API Manager"</div>
                    </div>
                </div>

                // Back to Home button
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

                // Theme toggle button
                <button
                    on:click=toggle_theme
                    class=button_class
                    title="Toggle theme"
                >
                    {move || if is_dark.get() {
                        view! {
                            <>
                                <svg class="w-5 h-5 text-yellow-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"></path>
                                </svg>
                                <span>"Light Mode"</span>
                            </>
                        }
                    } else {
                        view! {
                            <>
                                <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"></path>
                                </svg>
                                <span>"Dark Mode"</span>
                            </>
                        }
                    }}
                </button>

                // Copyright
                <div class="text-xs text-gray-500 text-center mt-4">
                    "© 2025 Pritchard"
                </div>
            </div>
        </aside>
    }
}
