use leptos::*;
use crate::store::WaapiStore;
use crate::api::waapi_client;

#[component]
pub fn Header() -> impl IntoView {
    // Get store from context for global refresh
    let store = use_context::<WaapiStore>()
        .expect("WaapiStore should be provided");

    // Global refresh handler - fetches all accounts from backend
    let global_refresh = {
        let store = store.clone();
        move |_| {
            let store = store.clone();
            store.set_loading(true);
            store.set_error(None);

            spawn_local(async move {
                match waapi_client::list_accounts().await {
                    Ok(response) => {
                        store.update_accounts(response.accounts);
                        logging::log!("✅ WhatsApp accounts refreshed: {} accounts", response.count);
                    }
                    Err(err) => {
                        logging::error!("❌ Failed to refresh accounts: {}", err);
                        store.set_error(Some(err));
                    }
                }
                store.set_loading(false);
            });
        }
    };

    view! {
        <header class="bg-white dark:bg-zinc-800 border-b border-gray-200 dark:border-zinc-700 px-6 py-4">
            <div class="flex items-center justify-between">
                // Title section
                <div class="flex-1">
                    <h2 class="text-2xl font-bold text-gray-900 dark:text-white">"WhatsApp API Manager"</h2>
                    <p class="text-sm text-gray-500 dark:text-gray-400 mt-1">"Manage your WhatsApp accounts and messages"</p>
                </div>

                // Right side: Global Refresh and Stats
                <div class="flex items-center gap-6 ml-6">
                    // Quick stats
                    <div class="flex items-center gap-4 text-sm">
                        <div class="flex items-center gap-2">
                            <div class="w-2 h-2 rounded-full bg-green-500"></div>
                            <span class="text-gray-600 dark:text-gray-300">
                                {move || format!("{} Connected", store.stats.get().connected_accounts)}
                            </span>
                        </div>
                        <div class="flex items-center gap-2">
                            <div class="w-2 h-2 rounded-full bg-blue-500"></div>
                            <span class="text-gray-600 dark:text-gray-300">
                                {move || format!("{} Total", store.stats.get().total_accounts)}
                            </span>
                        </div>
                    </div>

                    // Global Refresh Button
                    <button
                        on:click=global_refresh
                        disabled=move || store.loading.get()
                        class="p-2 hover:bg-gray-100 dark:hover:bg-zinc-700 rounded-lg transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                        title="Refresh accounts list"
                    >
                        <svg
                            class=move || if store.loading.get() {
                                "w-5 h-5 text-gray-600 dark:text-gray-300 animate-spin"
                            } else {
                                "w-5 h-5 text-gray-600 dark:text-gray-300"
                            }
                            fill="none"
                            stroke="currentColor"
                            viewBox="0 0 24 24"
                        >
                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"></path>
                        </svg>
                    </button>
                </div>
            </div>
        </header>
    }
}
