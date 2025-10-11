use leptos::*;
use leptos_router::*;
use crate::store::WaapiStore;
use crate::api::{waapi_client, types::WhatsAppAccount};

#[component]
pub fn AccountDetail() -> impl IntoView {
    let params = use_params_map();
    let store = use_context::<WaapiStore>().expect("WaapiStore should be provided");

    // Local state
    let account = create_rw_signal::<Option<WhatsAppAccount>>(None);
    let qr_code = create_rw_signal::<Option<String>>(None);
    let loading_qr = create_rw_signal(false);
    let deleting = create_rw_signal(false);
    let sending_message = create_rw_signal(false);
    let error = create_rw_signal::<Option<String>>(None);

    // Send message form
    let to_number = create_rw_signal(String::new());
    let message_text = create_rw_signal(String::new());
    let message_success = create_rw_signal::<Option<String>>(None);

    // Get JID from URL and find account
    let jid = create_memo(move |_| {
        params.with(|p| p.get("jid").cloned())
    });

    // Load account data
    create_effect(move |_| {
        if let Some(jid_value) = jid.get() {
            let decoded_jid = urlencoding::decode(&jid_value).unwrap_or_default().to_string();

            // Find account in store
            let accounts = store.accounts.get();
            if let Some(acc) = accounts.iter().find(|a| a.jid == decoded_jid).cloned() {
                account.set(Some(acc.clone()));

                // Load QR code if not authenticated
                if !acc.authenticated {
                    loading_qr.set(true);
                    spawn_local(async move {
                        match waapi_client::get_qr_code(decoded_jid.clone()).await {
                            Ok(response) => {
                                qr_code.set(Some(response.qr_code));
                                logging::log!("✅ QR code loaded");
                            }
                            Err(err) => {
                                logging::error!("❌ Failed to load QR code: {}", err);
                                error.set(Some(format!("Failed to load QR code: {}", err)));
                            }
                        }
                        loading_qr.set(false);
                    });
                }
            }
        }
    });

    // Refresh QR code
    let refresh_qr = move |_| {
        if let Some(acc) = account.get() {
            loading_qr.set(true);
            error.set(None);
            let jid = acc.jid.clone();

            spawn_local(async move {
                match waapi_client::get_qr_code(jid).await {
                    Ok(response) => {
                        qr_code.set(Some(response.qr_code));
                        logging::log!("✅ QR code refreshed");
                    }
                    Err(err) => {
                        logging::error!("❌ Failed to refresh QR code: {}", err);
                        error.set(Some(err));
                    }
                }
                loading_qr.set(false);
            });
        }
    };

    // Delete account
    let delete_account = move |_| {
        if let Some(acc) = account.get() {
            deleting.set(true);
            let jid = acc.jid.clone();
            let navigate = use_navigate();

            spawn_local(async move {
                match waapi_client::delete_account(jid).await {
                    Ok(_) => {
                        logging::log!("✅ Account deleted");
                        navigate("/waapi/accounts", Default::default());
                    }
                    Err(err) => {
                        logging::error!("❌ Failed to delete account: {}", err);
                        error.set(Some(err));
                        deleting.set(false);
                    }
                }
            });
        }
    };

    // Send message
    let send_message = move |_| {
        if let Some(acc) = account.get() {
            let to = to_number.get();
            let msg = message_text.get();

            if to.is_empty() || msg.is_empty() {
                error.set(Some("Phone number and message are required".to_string()));
                return;
            }

            sending_message.set(true);
            error.set(None);
            message_success.set(None);
            let jid = acc.jid.clone();

            spawn_local(async move {
                match waapi_client::send_message(jid, to.clone(), msg).await {
                    Ok(_) => {
                        logging::log!("✅ Message sent successfully");
                        message_success.set(Some(format!("Message sent to {}", to)));
                        to_number.set(String::new());
                        message_text.set(String::new());
                    }
                    Err(err) => {
                        logging::error!("❌ Failed to send message: {}", err);
                        error.set(Some(err));
                    }
                }
                sending_message.set(false);
            });
        }
    };

    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            <div class="mb-6">
                <A href="/waapi/accounts" class="inline-flex items-center gap-2 text-gray-600 dark:text-gray-400 hover:text-gray-900 dark:hover:text-white mb-4">
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 19l-7-7 7-7"></path>
                    </svg>
                    "Back to Accounts"
                </A>
            </div>

            {move || error.get().map(|err| view! {
                <div class="mb-6 bg-red-50 dark:bg-red-900/20 border border-red-200 dark:border-red-800 rounded-lg p-4">
                    <p class="text-red-700 dark:text-red-300">{err}</p>
                </div>
            })}

            {move || message_success.get().map(|msg| view! {
                <div class="mb-6 bg-green-50 dark:bg-green-900/20 border border-green-200 dark:border-green-800 rounded-lg p-4">
                    <p class="text-green-700 dark:text-green-300">{msg}</p>
                </div>
            })}

            {move || {
                if let Some(acc) = account.get() {
                    let status_badge = if acc.authenticated && acc.connected {
                        ("bg-green-100 text-green-800 dark:bg-green-900/20 dark:text-green-400", "Authenticated & Connected")
                    } else if acc.connected {
                        ("bg-yellow-100 text-yellow-800 dark:bg-yellow-900/20 dark:text-yellow-400", "Connected (Not Authenticated)")
                    } else {
                        ("bg-red-100 text-red-800 dark:bg-red-900/20 dark:text-red-400", "Disconnected")
                    };

                    view! {
                        <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
                            // Account Info Card
                            <div class="bg-white dark:bg-zinc-800 rounded-lg border border-gray-200 dark:border-zinc-700 p-6">
                                <h3 class="text-xl font-bold text-gray-900 dark:text-white mb-4">"Account Details"</h3>

                                <div class="space-y-4">
                                    <div>
                                        <p class="text-sm text-gray-500 dark:text-gray-400">"Phone Number"</p>
                                        <p class="text-lg font-semibold text-gray-900 dark:text-white">{acc.phone_number.clone()}</p>
                                    </div>

                                    <div>
                                        <p class="text-sm text-gray-500 dark:text-gray-400">"Device Name"</p>
                                        <p class="text-lg font-semibold text-gray-900 dark:text-white">{acc.device_name.clone()}</p>
                                    </div>

                                    <div>
                                        <p class="text-sm text-gray-500 dark:text-gray-400">"Platform"</p>
                                        <p class="text-lg font-semibold text-gray-900 dark:text-white">{acc.platform.clone()}</p>
                                    </div>

                                    <div>
                                        <p class="text-sm text-gray-500 dark:text-gray-400">"Status"</p>
                                        <span class=format!("inline-block px-3 py-1 rounded-full text-sm font-medium {}", status_badge.0)>
                                            {status_badge.1}
                                        </span>
                                    </div>

                                    <div>
                                        <p class="text-sm text-gray-500 dark:text-gray-400">"JID"</p>
                                        <p class="text-sm font-mono text-gray-700 dark:text-gray-300 break-all">{acc.jid.clone()}</p>
                                    </div>

                                    <div>
                                        <p class="text-sm text-gray-500 dark:text-gray-400">"Created At"</p>
                                        <p class="text-sm text-gray-700 dark:text-gray-300">{acc.created_at.clone()}</p>
                                    </div>
                                </div>

                                <div class="mt-6 pt-6 border-t border-gray-200 dark:border-zinc-700">
                                    <button
                                        on:click=delete_account
                                        disabled=move || deleting.get()
                                        class="w-full px-4 py-2 bg-red-600 text-white rounded-lg hover:bg-red-700 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                                    >
                                        {move || if deleting.get() { "Deleting..." } else { "Delete Account" }}
                                    </button>
                                </div>
                            </div>

                            // QR Code or Send Message Card
                            {if !acc.authenticated {
                                view! {
                                    <div class="bg-white dark:bg-zinc-800 rounded-lg border border-gray-200 dark:border-zinc-700 p-6">
                                        <h3 class="text-xl font-bold text-gray-900 dark:text-white mb-4">"Authenticate Account"</h3>
                                        <p class="text-sm text-gray-600 dark:text-gray-400 mb-4">
                                            "Scan this QR code with your WhatsApp mobile app to authenticate this account."
                                        </p>

                                        {move || {
                                            if loading_qr.get() {
                                                view! {
                                                    <div class="flex items-center justify-center h-64 bg-gray-100 dark:bg-zinc-700 rounded-lg">
                                                        <div class="text-center">
                                                            <svg class="w-12 h-12 mx-auto text-gray-400 animate-spin" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"></path>
                                                            </svg>
                                                            <p class="text-gray-500 dark:text-gray-400 mt-2">"Loading QR Code..."</p>
                                                        </div>
                                                    </div>
                                                }.into_view()
                                            } else if let Some(qr) = qr_code.get() {
                                                view! {
                                                    <div>
                                                        <div class="bg-white p-4 rounded-lg mb-4 flex items-center justify-center">
                                                            <img src=format!("https://api.qrserver.com/v1/create-qr-code/?size=300x300&data={}", urlencoding::encode(&qr)) alt="QR Code" class="w-64 h-64"/>
                                                        </div>
                                                        <button
                                                            on:click=refresh_qr
                                                            class="w-full px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors"
                                                        >
                                                            "Refresh QR Code"
                                                        </button>
                                                    </div>
                                                }.into_view()
                                            } else {
                                                view! {
                                                    <div class="text-center py-12">
                                                        <p class="text-gray-500 dark:text-gray-400 mb-4">"No QR code available"</p>
                                                        <button
                                                            on:click=refresh_qr
                                                            class="px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors"
                                                        >
                                                            "Generate QR Code"
                                                        </button>
                                                    </div>
                                                }.into_view()
                                            }
                                        }}
                                    </div>
                                }.into_view()
                            } else {
                                view! {
                                    <div class="bg-white dark:bg-zinc-800 rounded-lg border border-gray-200 dark:border-zinc-700 p-6">
                                        <h3 class="text-xl font-bold text-gray-900 dark:text-white mb-4">"Send Test Message"</h3>
                                        <p class="text-sm text-gray-600 dark:text-gray-400 mb-4">
                                            "Send a test message from this WhatsApp account."
                                        </p>

                                        <div class="space-y-4">
                                            <div>
                                                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                                                    "To (Phone Number)"
                                                </label>
                                                <input
                                                    type="text"
                                                    placeholder="+1234567890"
                                                    prop:value=move || to_number.get()
                                                    on:input=move |ev| to_number.set(event_target_value(&ev))
                                                    class="w-full px-4 py-2 bg-gray-50 dark:bg-zinc-700 border border-gray-200 dark:border-zinc-600 rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-green-500"
                                                />
                                            </div>

                                            <div>
                                                <label class="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                                                    "Message"
                                                </label>
                                                <textarea
                                                    placeholder="Enter your message here..."
                                                    rows="4"
                                                    prop:value=move || message_text.get()
                                                    on:input=move |ev| message_text.set(event_target_value(&ev))
                                                    class="w-full px-4 py-2 bg-gray-50 dark:bg-zinc-700 border border-gray-200 dark:border-zinc-600 rounded-lg text-gray-900 dark:text-white focus:outline-none focus:ring-2 focus:ring-green-500"
                                                />
                                            </div>

                                            <button
                                                on:click=send_message
                                                disabled=move || sending_message.get()
                                                class="w-full px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                                            >
                                                {move || if sending_message.get() { "Sending..." } else { "Send Message" }}
                                            </button>
                                        </div>
                                    </div>
                                }.into_view()
                            }}
                        </div>
                    }.into_view()
                } else {
                    view! {
                        <div class="text-center py-12">
                            <p class="text-gray-500 dark:text-gray-400">"Account not found"</p>
                        </div>
                    }.into_view()
                }
            }}
        </div>
    }
}
