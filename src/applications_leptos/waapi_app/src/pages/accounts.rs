use leptos::*;
use crate::store::WaapiStore;
use crate::api::waapi_client;
use crate::api::types::WhatsAppAccount;

#[component]
pub fn Accounts() -> impl IntoView {
    // Get store from context
    let store = use_context::<WaapiStore>()
        .expect("WaapiStore should be provided");

    // State for loading and error
    let loading = create_rw_signal(false);
    let error_signal = create_rw_signal::<Option<String>>(None);

    // State for QR modal
    let show_qr_modal = create_rw_signal(false);
    let qr_code_data = create_rw_signal::<Option<String>>(None);
    let qr_loading = create_rw_signal(false);
    let qr_account_jid = create_rw_signal::<Option<String>>(None);

    // State for delete modal
    let show_delete_modal = create_rw_signal(false);
    let delete_account = create_rw_signal::<Option<WhatsAppAccount>>(None);
    let deleting = create_rw_signal(false);

    // State for add account modal
    let show_add_modal = create_rw_signal(false);
    let add_phone_number = create_rw_signal(String::new());
    let add_device_name = create_rw_signal(String::new());
    let adding = create_rw_signal(false);
    let add_error = create_rw_signal::<Option<String>>(None);

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

    // Refresh handler
    let refresh_accounts = {
        let store = store.clone();
        move || {
            loading.set(true);
            error_signal.set(None);
            let store = store.clone();
            spawn_local(async move {
                match waapi_client::list_accounts().await {
                    Ok(response) => {
                        store.update_accounts(response.accounts);
                        logging::log!("✅ Accounts refreshed: {} accounts", response.count);
                    }
                    Err(err) => {
                        logging::error!("❌ Failed to refresh accounts: {}", err);
                        error_signal.set(Some(err));
                    }
                }
                loading.set(false);
            });
        }
    };

    // Authenticate handler
    let authenticate_account = {
        move |jid: String| {
            qr_account_jid.set(Some(jid.clone()));
            qr_loading.set(true);
            show_qr_modal.set(true);
            qr_code_data.set(None);

            spawn_local(async move {
                match waapi_client::get_qr_code(jid.clone()).await {
                    Ok(response) => {
                        if response.authenticated {
                            // Account already authenticated, show message and close modal
                            logging::log!("✅ Account {} is already authenticated", jid);
                            error_signal.set(Some(response.message));
                            show_qr_modal.set(false);
                        } else if let Some(qr) = response.qr_code {
                            // Show QR code
                            qr_code_data.set(Some(qr));
                            logging::log!("✅ QR code retrieved for {}", jid);
                        } else {
                            // No QR code available
                            logging::error!("❌ No QR code available for {}", jid);
                            error_signal.set(Some("QR code not available".to_string()));
                            show_qr_modal.set(false);
                        }
                    }
                    Err(err) => {
                        logging::error!("❌ Failed to get QR code: {}", err);
                        error_signal.set(Some(err));
                        show_qr_modal.set(false);
                    }
                }
                qr_loading.set(false);
            });
        }
    };

    // Delete handler and add handler - we'll define these inline in the modals instead

    let accounts = store.accounts;

    // Clone store for use in the view closures
    let store_for_add = store.clone();
    let store_for_delete = store.clone();

    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            <div class="flex items-center justify-between mb-6">
                <div>
                    <h2 class="text-2xl font-bold text-gray-900 dark:text-white mb-2">"WhatsApp Accounts"</h2>
                    <p class="text-gray-600 dark:text-gray-400">"Manage all your WhatsApp accounts"</p>
                </div>
                <div class="flex gap-3">
                    <button
                        on:click=move |_| show_add_modal.set(true)
                        class="inline-flex items-center gap-2 px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors"
                    >
                        <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4"></path>
                        </svg>
                        "Add Account"
                    </button>
                    <button
                        on:click=move |_| refresh_accounts()
                        disabled=move || loading.get()
                        class="inline-flex items-center gap-2 px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                    >
                        <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"></path>
                        </svg>
                        {move || if loading.get() { "Refreshing..." } else { "Refresh" }}
                    </button>
                </div>
            </div>

            {move || error_signal.get().map(|err| view! {
                <div class="mb-6 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg p-4">
                    <h3 class="text-lg font-semibold text-red-900 dark:text-red-400 mb-2">"Error"</h3>
                    <p class="text-red-700 dark:text-red-300">{err}</p>
                </div>
            })}

            // Accounts Table
            <div class="bg-white dark:bg-zinc-800 rounded-lg shadow overflow-hidden">
                <table class="min-w-full divide-y divide-gray-200 dark:divide-zinc-700">
                    <thead class="bg-gray-50 dark:bg-zinc-900">
                        <tr>
                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Phone Number"</th>
                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Device Name"</th>
                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Connected"</th>
                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Authenticated"</th>
                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Actions"</th>
                        </tr>
                    </thead>
                    <tbody class="bg-white dark:bg-zinc-800 divide-y divide-gray-200 dark:divide-zinc-700">
                        {move || {
                            let acc = accounts.get();
                            if acc.is_empty() {
                                view! {
                                    <tr>
                                        <td colspan="5" class="px-6 py-12 text-center text-gray-500 dark:text-gray-400">
                                            <svg class="w-16 h-16 mx-auto text-gray-300 dark:text-gray-600 mb-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0zm6 3a2 2 0 11-4 0 2 2 0 014 0zM7 10a2 2 0 11-4 0 2 2 0 014 0z"></path>
                                            </svg>
                                            <p>"No WhatsApp accounts yet"</p>
                                        </td>
                                    </tr>
                                }.into_view()
                            } else {
                                acc.into_iter().map(|account| {
                                    let jid = account.jid.clone();
                                    let phone = format_phone_number(account.phone_number.clone());
                                    let device = account.device_name.clone();
                                    let connected = account.connected;
                                    let authenticated = account.authenticated;

                                    let jid_for_auth = jid.clone();
                                    let account_for_delete = account.clone();

                                    view! {
                                        <tr class="hover:bg-gray-50 dark:hover:bg-zinc-700/50 transition-colors">
                                            <td class="px-6 py-4 whitespace-nowrap text-sm font-medium text-gray-900 dark:text-white">
                                                {phone}
                                            </td>
                                            <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-600 dark:text-gray-300">
                                                {device}
                                            </td>
                                            <td class="px-6 py-4 whitespace-nowrap">
                                                {if connected {
                                                    view! {
                                                        <span class="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-green-100 text-green-800 dark:bg-green-900/20 dark:text-green-400">
                                                            <svg class="w-3 h-3 mr-1" fill="currentColor" viewBox="0 0 8 8">
                                                                <circle cx="4" cy="4" r="3" />
                                                            </svg>
                                                            "Connected"
                                                        </span>
                                                    }.into_view()
                                                } else {
                                                    view! {
                                                        <span class="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-red-100 text-red-800 dark:bg-red-900/20 dark:text-red-400">
                                                            <svg class="w-3 h-3 mr-1" fill="currentColor" viewBox="0 0 8 8">
                                                                <circle cx="4" cy="4" r="3" />
                                                            </svg>
                                                            "Disconnected"
                                                        </span>
                                                    }.into_view()
                                                }}
                                            </td>
                                            <td class="px-6 py-4 whitespace-nowrap">
                                                {if authenticated {
                                                    view! {
                                                        <span class="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-green-100 text-green-800 dark:bg-green-900/20 dark:text-green-400">
                                                            "Authenticated"
                                                        </span>
                                                    }.into_view()
                                                } else {
                                                    view! {
                                                        <button
                                                            on:click=move |_| authenticate_account(jid_for_auth.clone())
                                                            class="inline-flex items-center gap-2 px-4 py-2 rounded-lg text-sm font-semibold bg-blue-500 text-white hover:bg-blue-600 dark:bg-blue-600 dark:hover:bg-blue-700 transition-colors border-2 border-blue-500 dark:border-blue-600 shadow-sm"
                                                        >
                                                            <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z"></path>
                                                            </svg>
                                                            "Authenticate Now"
                                                        </button>
                                                    }.into_view()
                                                }}
                                            </td>
                                            <td class="px-6 py-4 whitespace-nowrap text-sm">
                                                <button
                                                    on:click=move |_| {
                                                        delete_account.set(Some(account_for_delete.clone()));
                                                        show_delete_modal.set(true);
                                                    }
                                                    class="inline-flex items-center p-2 text-red-600 dark:text-red-400 hover:bg-red-50 dark:hover:bg-red-900/20 rounded-lg transition-colors"
                                                    title="Delete account"
                                                >
                                                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16"></path>
                                                    </svg>
                                                </button>
                                            </td>
                                        </tr>
                                    }
                                }).collect::<Vec<_>>().into_view()
                            }
                        }}
                    </tbody>
                </table>
            </div>

            // QR Code Modal
            <Show
                when=move || show_qr_modal.get()
                fallback=|| view! { <></> }
            >
                <div class="fixed inset-0 bg-black/50 backdrop-blur-sm flex items-center justify-center z-50">
                    <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-xl max-w-md w-full mx-4 p-6">
                        <div class="flex items-center justify-between mb-4">
                            <h3 class="text-xl font-bold text-gray-900 dark:text-white">"Scan QR Code"</h3>
                            <button
                                on:click=move |_| {
                                    show_qr_modal.set(false);
                                    qr_code_data.set(None);
                                    qr_account_jid.set(None);
                                }
                                class="text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
                            >
                                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path>
                                </svg>
                            </button>
                        </div>

                        <div class="flex flex-col items-center justify-center py-8">
                            {move || {
                                if qr_loading.get() {
                                    view! {
                                        <div class="flex flex-col items-center">
                                            <div class="w-16 h-16 border-4 border-green-600 border-t-transparent rounded-full animate-spin"></div>
                                            <p class="mt-4 text-gray-600 dark:text-gray-400">"Loading QR code..."</p>
                                        </div>
                                    }.into_view()
                                } else if let Some(qr) = qr_code_data.get() {
                                    view! {
                                        <div class="flex flex-col items-center">
                                            <div class="bg-white p-4 rounded-lg">
                                                <QRCodeDisplay qr_data=qr />
                                            </div>
                                            <p class="mt-4 text-sm text-gray-600 dark:text-gray-400 text-center">
                                                "Scan this QR code with your WhatsApp mobile app"
                                            </p>
                                        </div>
                                    }.into_view()
                                } else {
                                    view! {
                                        <p class="text-gray-600 dark:text-gray-400">"Failed to load QR code"</p>
                                    }.into_view()
                                }
                            }}
                        </div>
                    </div>
                </div>
            </Show>

            // Add Account Modal
            <Show
                when=move || show_add_modal.get()
                fallback=|| view! { <></> }
            >
                <div class="fixed inset-0 bg-black/50 backdrop-blur-sm flex items-center justify-center z-50">
                    <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-xl max-w-md w-full mx-4 p-6">
                        <div class="flex items-center justify-between mb-4">
                            <h3 class="text-xl font-bold text-gray-900 dark:text-white">"Add WhatsApp Account"</h3>
                            <button
                                on:click=move |_| {
                                    show_add_modal.set(false);
                                    add_error.set(None);
                                }
                                class="text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
                            >
                                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path>
                                </svg>
                            </button>
                        </div>

                        {move || add_error.get().map(|err| view! {
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
                                    placeholder="+27796669999"
                                    prop:value=move || add_phone_number.get()
                                    on:input=move |ev| add_phone_number.set(event_target_value(&ev))
                                    class="w-full px-4 py-2 bg-gray-50 dark:bg-zinc-700 border border-gray-200 dark:border-zinc-600 rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-blue-500"
                                />
                                <p class="mt-1 text-xs text-gray-500 dark:text-gray-400">"Include country code (e.g., +27796669999)"</p>
                            </div>

                            <div>
                                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                                    "Device Name (optional)"
                                </label>
                                <input
                                    type="text"
                                    placeholder="079 666 9999"
                                    prop:value=move || add_device_name.get()
                                    on:input=move |ev| add_device_name.set(event_target_value(&ev))
                                    class="w-full px-4 py-2 bg-gray-50 dark:bg-zinc-700 border border-gray-200 dark:border-zinc-600 rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-blue-500"
                                />
                            </div>
                        </div>

                        <div class="flex gap-3 mt-6">
                            <button
                                on:click=move |_| {
                                    show_add_modal.set(false);
                                    add_error.set(None);
                                }
                                class="flex-1 px-4 py-2 bg-gray-200 dark:bg-zinc-700 text-gray-700 dark:text-gray-300 rounded-lg hover:bg-gray-300 dark:hover:bg-zinc-600 transition-colors"
                            >
                                "Cancel"
                            </button>
                            <button
                                on:click={
                                    let store = store_for_add.clone();
                                    move |_| {
                                        let phone = add_phone_number.get();
                                        let device = add_device_name.get();

                                        if phone.is_empty() {
                                            add_error.set(Some("Phone number is required".to_string()));
                                            return;
                                        }

                                        adding.set(true);
                                        add_error.set(None);

                                        let store = store.clone();

                                        spawn_local(async move {
                                            match waapi_client::create_account(
                                                phone.clone(),
                                                if device.is_empty() { None } else { Some(device) },
                                                Some("whatsmeow".to_string())
                                            ).await {
                                                Ok(response) => {
                                                    logging::log!("✅ Account created: {}", response.session_id);

                                                    // If not authenticated and QR code is returned, show it immediately
                                                    if !response.authenticated && response.qr_code.is_some() {
                                                        qr_code_data.set(response.qr_code);
                                                        qr_account_jid.set(Some(response.session_id));
                                                        show_add_modal.set(false);
                                                        show_qr_modal.set(true);
                                                    } else {
                                                        show_add_modal.set(false);
                                                    }

                                                    // Refresh accounts list
                                                    if let Ok(list_response) = waapi_client::list_accounts().await {
                                                        store.update_accounts(list_response.accounts);
                                                    }

                                                    // Reset form
                                                    add_phone_number.set(String::new());
                                                    add_device_name.set(String::new());
                                                }
                                                Err(err) => {
                                                    logging::error!("❌ Failed to create account: {}", err);
                                                    add_error.set(Some(err));
                                                }
                                            }
                                            adding.set(false);
                                        });
                                    }
                                }
                                disabled=move || adding.get()
                                class="flex-1 px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                            >
                                {move || if adding.get() { "Adding..." } else { "Add Account" }}
                            </button>
                        </div>
                    </div>
                </div>
            </Show>

            // Delete Confirmation Modal
            <Show
                when=move || show_delete_modal.get()
                fallback=|| view! { <></> }
            >
                <div class="fixed inset-0 bg-black/50 backdrop-blur-sm flex items-center justify-center z-50">
                    <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-xl max-w-md w-full mx-4 p-6">
                        <div class="flex items-center justify-between mb-4">
                            <h3 class="text-xl font-bold text-gray-900 dark:text-white">"Confirm Deletion"</h3>
                            <button
                                on:click=move |_| {
                                    show_delete_modal.set(false);
                                    delete_account.set(None);
                                }
                                class="text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
                            >
                                <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path>
                                </svg>
                            </button>
                        </div>

                        <div class="mb-6">
                            <p class="text-gray-600 dark:text-gray-400">
                                {move || {
                                    if let Some(account) = delete_account.get() {
                                        let display = if let Some(phone) = account.phone_number {
                                            format_phone_number(Some(phone))
                                        } else {
                                            account.device_name
                                        };
                                        format!("Are you sure you want to delete {}?", display)
                                    } else {
                                        "Are you sure you want to delete this account?".to_string()
                                    }
                                }}
                            </p>
                            <p class="text-sm text-gray-500 dark:text-gray-500 mt-2">
                                "This action cannot be undone."
                            </p>
                        </div>

                        <div class="flex gap-3">
                            <button
                                on:click=move |_| {
                                    show_delete_modal.set(false);
                                    delete_account.set(None);
                                }
                                class="flex-1 px-4 py-2 bg-gray-200 dark:bg-zinc-700 text-gray-700 dark:text-gray-300 rounded-lg hover:bg-gray-300 dark:hover:bg-zinc-600 transition-colors"
                            >
                                "Cancel"
                            </button>
                            <button
                                on:click={
                                    let store = store_for_delete.clone();
                                    move |_| {
                                        if let Some(account) = delete_account.get() {
                                            deleting.set(true);
                                            let jid = account.jid.clone();
                                            let store = store.clone();

                                            spawn_local(async move {
                                                match waapi_client::delete_account(jid.clone()).await {
                                                    Ok(_) => {
                                                        logging::log!("✅ Account deleted: {}", jid);
                                                        // Refresh accounts list
                                                        if let Ok(list_response) = waapi_client::list_accounts().await {
                                                            store.update_accounts(list_response.accounts);
                                                        }
                                                        show_delete_modal.set(false);
                                                        delete_account.set(None);
                                                    }
                                                    Err(err) => {
                                                        logging::error!("❌ Failed to delete account: {}", err);
                                                        error_signal.set(Some(err));
                                                    }
                                                }
                                                deleting.set(false);
                                            });
                                        }
                                    }
                                }
                                disabled=move || deleting.get()
                                class="flex-1 px-4 py-2 bg-red-600 text-white rounded-lg hover:bg-red-700 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                            >
                                {move || if deleting.get() { "Deleting..." } else { "Confirm" }}
                            </button>
                        </div>
                    </div>
                </div>
            </Show>
        </div>
    }
}

/// Format phone number for display
fn format_phone_number(phone: Option<String>) -> String {
    match phone {
        Some(p) if p.starts_with("27") && p.len() == 11 => {
            // Format South African numbers: 27639019917 -> +27 63 901 9917
            format!("+27 {} {} {}", &p[2..4], &p[4..7], &p[7..11])
        }
        Some(p) => format!("+{}", p),
        None => "N/A".to_string(),
    }
}

/// QR Code Display Component
#[component]
fn QRCodeDisplay(qr_data: String) -> impl IntoView {
    use qrcode::QrCode;
    use qrcode::render::svg;

    // Generate QR code SVG
    let svg_string = match QrCode::new(qr_data.as_bytes()) {
        Ok(code) => {
            code.render::<svg::Color>()
                .min_dimensions(256, 256)
                .dark_color(svg::Color("#000000"))
                .light_color(svg::Color("#ffffff"))
                .build()
        }
        Err(_) => {
            return view! {
                <div class="w-64 h-64 flex items-center justify-center bg-gray-100 dark:bg-zinc-700 rounded">
                    <p class="text-red-600 dark:text-red-400">"Failed to generate QR code"</p>
                </div>
            }.into_view()
        }
    };

    view! {
        <div inner_html=svg_string class="w-64 h-64"></div>
    }.into_view()
}
