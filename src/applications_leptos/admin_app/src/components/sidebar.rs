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
        current == path || (path == "/admin/dashboard" && current == "/admin")
    };

    let link_class = move |path: &str| {
        let base = "flex items-center gap-3 px-4 py-3 rounded-lg transition-colors mb-2 text-sm font-medium";
        if is_active(path) {
            format!("{} bg-blue-600 text-white", base)
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
                    <svg class="w-8 h-8 text-blue-500" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z"></path>
                    </svg>
                    <h1 class="text-xl font-bold text-white">"Pritchard Admin"</h1>
                </div>
            </div>

            // Navigation menu
            <nav class="flex-1">
                <A href="/admin/dashboard" class=move || link_class("/admin/dashboard")>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z"></path>
                    </svg>
                    <span>"Dashboard"</span>
                </A>

                <A href="/admin/database" class=move || link_class("/admin/database")>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 7v10c0 2.21 3.582 4 8 4s8-1.79 8-4V7M4 7c0 2.21 3.582 4 8 4s8-1.79 8-4M4 7c0-2.21 3.582-4 8-4s8 1.79 8 4m0 5c0 2.21-3.582 4-8 4s-8-1.79-8-4"></path>
                    </svg>
                    <span>"Database"</span>
                </A>

                <A href="/admin/crm" class=move || link_class("/admin/crm")>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0zm6 3a2 2 0 11-4 0 2 2 0 014 0zM7 10a2 2 0 11-4 0 2 2 0 014 0z"></path>
                    </svg>
                    <span>"CRM"</span>
                </A>

                <A href="/admin/ci-cd" class=move || link_class("/admin/ci-cd")>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"></path>
                    </svg>
                    <span>"CI/CD"</span>
                </A>

                <A href="/admin/settings" class=move || link_class("/admin/settings")>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M10.325 4.317c.426-1.756 2.924-1.756 3.35 0a1.724 1.724 0 002.573 1.066c1.543-.94 3.31.826 2.37 2.37a1.724 1.724 0 001.065 2.572c1.756.426 1.756 2.924 0 3.35a1.724 1.724 0 00-1.066 2.573c.94 1.543-.826 3.31-2.37 2.37a1.724 1.724 0 00-2.572 1.065c-.426 1.756-2.924 1.756-3.35 0a1.724 1.724 0 00-2.573-1.066c-1.543.94-3.31-.826-2.37-2.37a1.724 1.724 0 00-1.065-2.572c-1.756-.426-1.756-2.924 0-3.35a1.724 1.724 0 001.066-2.573c-.94-1.543.826-3.31 2.37-2.37.996.608 2.296.07 2.572-1.065z"></path>
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"></path>
                    </svg>
                    <span>"Settings"</span>
                </A>
            </nav>

            // Bottom section: User info + controls
            <div class="mt-auto pt-6 border-t border-[#1e293b]">
                // User info
                <div class="flex items-center gap-3 mb-4 px-2">
                    <div class="w-8 h-8 rounded-full bg-gradient-to-br from-blue-500 to-purple-600 flex items-center justify-center text-white text-xs font-semibold">
                        "JS"
                    </div>
                    <div class="flex-1">
                        <div class="text-sm font-medium text-white">"User"</div>
                        <div class="text-xs text-gray-400">"Admin"</div>
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
