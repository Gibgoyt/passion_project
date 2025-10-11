use leptos::*;
use leptos_router::*;
use crate::store::WaapiStore;
use crate::api::waapi_client;

#[component]
pub fn Accounts() -> impl IntoView {
    // Get store from context
    let store = use_context::<WaapiStore>()
        .expect("WaapiStore should be provided");

    // State for create account modal
    let show_create_modal = create_rw_signal(false);
    let phone_number = create_rw_signal(String::new());
    let device_name = create_rw_signal(String::from("Bot"));
    let creating = create_rw_signal(false);
    let create_error = create_rw_signal::<Option<String>>(None);

    // Clone store before using in effects
    let store_for_effect = store.clone();

    // Load accounts on mount
    create_effect(move |_| {
        let store = store_for_effect.clone();
        spawn_local(async move {
            store.set_loading(true);
            match waapi_client::list_accounts().await {
                Ok(response) => {
                    store.update_accounts(response.accounts);
                    logging::log!("✅ Accounts loaded: {} accounts", response.count);
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

    // Clone signals for the create handler
    let store_clone = store.clone();
    let phone_number_clone = phone_number.clone();
    let device_name_clone = device_name.clone();
    let creating_clone = creating.clone();
    let create_error_clone = create_error.clone();
    let show_create_modal_clone = show_create_modal.clone();

    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            <div class="flex items-center justify-between mb-6">
                <div>
                    <h2 class="text-2xl font-bold text-gray-900 dark:text-white mb-2">"WhatsApp Accounts"</h2>
                    <p class="text-gray-600 dark:text-gray-400">"Manage all your WhatsApp accounts"</p>
                </div>
                <button
                    on:click=move |_| show_create_modal.set(true)
                    class="inline-flex items-center gap-2 px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors"
                >
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4"></path>
                    </svg>
                    "Add Account"
                </button>
            </div>

            {move || error_signal.get().map(|err| view! {
                <div class="mb-6 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg p-4">
                    <h3 class="text-lg font-semibold text-red-900 dark:text-red-400 mb-2">"Error"</h3>
                    <p class="text-red-700 dark:text-red-300">{err}</p>
                </div>
            })}

            <AccountsList store=store />

            // Create Account Modal
            <Show
                when=move || show_create_modal.get()
                fallback=|| view! { <></> }
            >
                <div class="fixed inset-0 bg-black/50 backdrop-blur-sm flex items-center justify-center z-50">
                    <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-xl max-w-md w-full mx-4 p-6">
                        <div class="flex items-center justify-between mb-4">
                            <h3 class="text-xl font-bold text-gray-900 dark:text-white">"Add WhatsApp Account"</h3>
                            <button
                                on:click=move |_| show_create_modal.set(false)
                                class="text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
                            >
                                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path>
                                </svg>
                            </button>
                        </div>

                        {move || create_error.get().map(|err| view! {
                            <div class="mb-4 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg p-3">
                                <p class="text-sm text-red-700 dark:text-red-300">{err}</p>
                            </div>
                        })}

                        <div class="space-y-4">
                            <div>
                                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                                    "Phone Number"
                                </label>
                                <input
                                    type="text"
                                    placeholder="+1234567890"
                                    prop:value=move || phone_number.get()
                                    on:input=move |ev| phone_number.set(event_target_value(&ev))
                                    class="w-full px-4 py-2 bg-gray-50 dark:bg-zinc-700 border border-gray-200 dark:border-zinc-600 rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-green-500"
                                />
                            </div>

                            <div>
                                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                                    "Device Name (optional)"
                                </label>
                                <input
                                    type="text"
                                    placeholder="Bot"
                                    prop:value=move || device_name.get()
                                    on:input=move |ev| device_name.set(event_target_value(&ev))
                                    class="w-full px-4 py-2 bg-gray-50 dark:bg-zinc-700 border border-gray-200 dark:border-zinc-600 rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-green-500"
                                />
                            </div>
                        </div>

                        <div class="flex gap-3 mt-6">
                            <button
                                on:click=move |_| show_create_modal.set(false)
                                class="flex-1 px-4 py-2 bg-gray-200 dark:bg-zinc-700 text-gray-700 dark:text-gray-300 rounded-lg hover:bg-gray-300 dark:hover:bg-zinc-600 transition-colors"
                            >
                                "Cancel"
                            </button>
                            <button
                                on:click={
                                    let store = store_clone.clone();
                                    let phone_number = phone_number_clone.clone();
                                    let device_name = device_name_clone.clone();
                                    let creating = creating_clone.clone();
                                    let create_error = create_error_clone.clone();
                                    let show_create_modal = show_create_modal_clone.clone();

                                    move |_| {
                                        let phone = phone_number.get();
                                        if phone.is_empty() {
                                            create_error.set(Some("Phone number is required".to_string()));
                                            return;
                                        }

                                        creating.set(true);
                                        create_error.set(None);

                                        let store = store.clone();
                                        let phone_number = phone_number.clone();
                                        let device_name = device_name.clone();
                                        let creating = creating.clone();
                                        let create_error = create_error.clone();
                                        let show_create_modal = show_create_modal.clone();

                                        spawn_local(async move {
                                            let device = if device_name.get().is_empty() {
                                                None
                                            } else {
                                                Some(device_name.get())
                                            };

                                            match waapi_client::create_account(phone.clone(), device, Some("whatsmeow".to_string())).await {
                                                Ok(response) => {
                                                    logging::log!("✅ Account created: {}", response.session_id);
                                                    // Refresh accounts list
                                                    if let Ok(list_response) = waapi_client::list_accounts().await {
                                                        store.update_accounts(list_response.accounts);
                                                    }
                                                    // Close modal
                                                    show_create_modal.set(false);
                                                    phone_number.set(String::new());
                                                    device_name.set(String::from("Bot"));
                                                }
                                                Err(err) => {
                                                    logging::error!("❌ Failed to create account: {}", err);
                                                    create_error.set(Some(err));
                                                }
                                            }
                                            creating.set(false);
                                        });
                                    }
                                }
                                disabled=move || creating.get()
                                class="flex-1 px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                            >
                                {move || if creating.get() { "Creating..." } else { "Create Account" }}
                            </button>
                        </div>
                    </div>
                </div>
            </Show>
        </div>
    }
}

#[component]
fn AccountsList(store: WaapiStore) -> impl IntoView {
    let accounts = store.accounts;

    view! {
        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {move || {
                let acc = accounts.get();
                if acc.is_empty() {
                    view! {
                        <div class="col-span-full text-center py-12 bg-white dark:bg-zinc-800 rounded-lg border border-gray-200 dark:border-zinc-700">
                            <svg class="w-16 h-16 mx-auto text-gray-300 dark:text-gray-600 mb-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0zm6 3a2 2 0 11-4 0 2 2 0 014 0zM7 10a2 2 0 11-4 0 2 2 0 014 0z"></path>
                            </svg>
                            <p class="text-gray-500 dark:text-gray-400">"No WhatsApp accounts yet"</p>
                        </div>
                    }.into_view()
                } else {
                    acc.into_iter().map(|account| {
                        let jid = account.jid.clone();
                        let phone = account.phone_number.clone();
                        let device = account.device_name.clone();
                        let connected = account.connected;
                        let authenticated = account.authenticated;
                        let created_at = account.created_at.clone();

                        let status_badge = if authenticated && connected {
                            ("bg-green-100 text-green-800 dark:bg-green-900/20 dark:text-green-400", "Authenticated", "bg-green-500")
                        } else if connected {
                            ("bg-yellow-100 text-yellow-800 dark:bg-yellow-900/20 dark:text-yellow-400", "Connected", "bg-yellow-500")
                        } else {
                            ("bg-red-100 text-red-800 dark:bg-red-900/20 dark:text-red-400", "Disconnected", "bg-red-500")
                        };

                        view! {
                            <A
                                href=format!("/waapi/accounts/{}", urlencoding::encode(&jid))
                                class="block bg-white dark:bg-zinc-800 rounded-lg border border-gray-200 dark:border-zinc-700 p-6 hover:shadow-lg transition-all hover:scale-[1.02]"
                            >
                                <div class="flex items-start justify-between mb-4">
                                    <div class="flex items-center gap-3">
                                        <div class="w-14 h-14 bg-gradient-to-br from-green-500 to-emerald-600 rounded-full flex items-center justify-center text-white font-bold text-lg">
                                            {phone.chars().take(2).collect::<String>()}
                                        </div>
                                        <div>
                                            <p class="font-bold text-gray-900 dark:text-white text-lg">{phone.clone()}</p>
                                            <p class="text-sm text-gray-500 dark:text-gray-400">{device}</p>
                                        </div>
                                    </div>
                                    <div class=format!("w-3 h-3 rounded-full {}", status_badge.2)></div>
                                </div>

                                <div class="space-y-2">
                                    <span class=format!("inline-block px-3 py-1 rounded-full text-xs font-medium {}", status_badge.0)>
                                        {status_badge.1}
                                    </span>
                                    <p class="text-xs text-gray-500 dark:text-gray-400">
                                        "Created: " {created_at.split('T').next().unwrap_or("Unknown").to_string()}
                                    </p>
                                </div>

                                <div class="mt-4 pt-4 border-t border-gray-200 dark:border-zinc-700">
                                    <p class="text-sm text-green-600 dark:text-green-400 font-medium">
                                        "View Details →"
                                    </p>
                                </div>
                            </A>
                        }
                    }).collect::<Vec<_>>().into_view()
                }
            }}
        </div>
    }
}
