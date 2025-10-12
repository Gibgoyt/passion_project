use leptos::*;
use leptos_router::*;
use crate::store::WaapiStore;
use crate::api::waapi_client;

#[component]
pub fn Dashboard() -> impl IntoView {
    // Get store from context
    let store = use_context::<WaapiStore>()
        .expect("WaapiStore should be provided");

    // Load accounts on mount
    let store_clone = store.clone();
    create_effect(move |_| {
        let store = store_clone.clone();
        spawn_local(async move {
            store.set_loading(true);
            match waapi_client::list_accounts().await {
                Ok(response) => {
                    store.update_accounts(response.accounts);
                    logging::log!("✅ Dashboard loaded: {} accounts", response.count);
                }
                Err(err) => {
                    logging::error!("❌ Failed to load accounts: {}", err);
                    store.set_error(Some(err));
                }
            }
            store.set_loading(false);
        });
    });

    let error_signal = store.error;

    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            <div class="mb-6">
                <h2 class="text-2xl font-bold text-gray-900 dark:text-white mb-2">"Dashboard"</h2>
                <p class="text-gray-600 dark:text-gray-400">"Overview of your WhatsApp accounts"</p>
            </div>

            {move || error_signal.get().map(|err| view! {
                <div class="mb-6 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg p-4">
                    <h3 class="text-lg font-semibold text-red-900 dark:text-red-400 mb-2">"Error"</h3>
                    <p class="text-red-700 dark:text-red-300">{err}</p>
                </div>
            })}

            <DashboardContent store=store />
        </div>
    }
}

#[component]
fn DashboardContent(store: WaapiStore) -> impl IntoView {
    let stats = store.stats;
    let accounts = store.accounts;

    view! {
        <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mt-6">
            // Total Accounts card
            <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700">
                <h3 class="text-sm font-medium text-gray-500 dark:text-gray-400 mb-2">"Total Accounts"</h3>
                <p class="text-3xl font-bold text-gray-900 dark:text-white">
                    {move || stats.get().total_accounts}
                </p>
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    "WhatsApp accounts managed"
                </p>
            </div>

            // Connected Accounts card
            <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700">
                <h3 class="text-sm font-medium text-gray-500 dark:text-gray-400 mb-2">"Connected"</h3>
                <p class="text-3xl font-bold text-green-600 dark:text-green-400">
                    {move || stats.get().connected_accounts}
                </p>
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    {move || format!("{}/{} online", stats.get().connected_accounts, stats.get().total_accounts)}
                </p>
            </div>

            // Authenticated Accounts card
            <div class="bg-white dark:bg-zinc-800 p-6 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700">
                <h3 class="text-sm font-medium text-gray-500 dark:text-gray-400 mb-2">"Authenticated"</h3>
                <p class="text-3xl font-bold text-blue-600 dark:text-blue-400">
                    {move || stats.get().authenticated_accounts}
                </p>
                <p class="text-xs text-gray-500 dark:text-gray-400 mt-1">
                    "Ready to send messages"
                </p>
            </div>
        </div>

        // Recent Accounts section
        <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm border border-gray-200 dark:border-zinc-700 p-6 mt-6">
            <div class="flex items-center justify-between mb-4">
                <h3 class="text-lg font-semibold text-gray-900 dark:text-white">"Recent Accounts"</h3>
                <A href="/waapi/accounts" class="text-sm text-green-600 dark:text-green-400 hover:underline">
                    "View all →"
                </A>
            </div>
            {move || {
                let acc = accounts.get();
                if acc.is_empty() {
                    view! {
                        <div class="text-center py-12">
                            <svg class="w-16 h-16 mx-auto text-gray-300 dark:text-gray-600 mb-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0zm6 3a2 2 0 11-4 0 2 2 0 014 0zM7 10a2 2 0 11-4 0 2 2 0 014 0z"></path>
                            </svg>
                            <p class="text-gray-500 dark:text-gray-400 mb-4">"No WhatsApp accounts found"</p>
                            <A href="/waapi/accounts" class="inline-flex items-center gap-2 px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors">
                                <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4"></path>
                                </svg>
                                "Add Your First Account"
                            </A>
                        </div>
                    }.into_view()
                } else {
                    // Show first 5 accounts
                    let display_accounts = acc.into_iter().take(5).collect::<Vec<_>>();
                    view! {
                        <div class="space-y-3">
                            {display_accounts.into_iter().map(|account| {
                                let jid = account.jid.clone();
                                let phone = account.phone_number.clone();
                                let device = account.device_name.clone();
                                let connected = account.connected;
                                let authenticated = account.authenticated;

                                let status_badge = if authenticated && connected {
                                    ("bg-green-100 text-green-800 dark:bg-green-900/20 dark:text-green-400", "Authenticated")
                                } else if connected {
                                    ("bg-yellow-100 text-yellow-800 dark:bg-yellow-900/20 dark:text-yellow-400", "Connected")
                                } else {
                                    ("bg-red-100 text-red-800 dark:bg-red-900/20 dark:text-red-400", "Disconnected")
                                };

                                let display_initials = if let Some(ref p) = phone {
                                    p.chars().take(2).collect::<String>()
                                } else {
                                    device.chars().take(2).collect::<String>()
                                };
                                let display_phone = phone.clone().unwrap_or_else(|| device.clone());

                                view! {
                                    <A
                                        href=format!("/waapi/accounts/{}", urlencoding::encode(&jid))
                                        class="block p-4 rounded-lg border border-gray-200 dark:border-zinc-700 hover:bg-gray-50 dark:hover:bg-zinc-700/50 transition-colors"
                                    >
                                        <div class="flex items-center justify-between">
                                            <div class="flex items-center gap-3">
                                                <div class="w-12 h-12 bg-gradient-to-br from-green-500 to-emerald-600 rounded-full flex items-center justify-center text-white font-medium text-sm">
                                                    {display_initials}
                                                </div>
                                                <div>
                                                    <p class="font-medium text-gray-900 dark:text-white">{display_phone}</p>
                                                    <p class="text-sm text-gray-500 dark:text-gray-400">{device}</p>
                                                </div>
                                            </div>
                                            <span class=format!("px-3 py-1 rounded-full text-xs font-medium {}", status_badge.0)>
                                                {status_badge.1}
                                            </span>
                                        </div>
                                    </A>
                                }
                            }).collect::<Vec<_>>()}
                        </div>
                    }.into_view()
                }
            }}
        </div>
    }
}
