use leptos::*;
use leptos_router::*;
use serde::{Deserialize, Serialize};
use wasm_bindgen::JsValue;
use wasm_bindgen::JsCast;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ContactFormRecord {
    pub id: i32,
    #[serde(rename = "Name")]
    pub name: String,
    #[serde(rename = "Phone")]
    pub phone: String,
    #[serde(rename = "Department")]
    pub department: String,
    #[serde(rename = "Message")]
    pub message: String,
    #[serde(rename = "RawEmail")]
    pub raw_email: String,
    #[serde(rename = "CreatedAt")]
    pub created_at: String,
}

async fn fetch_contact_forms() -> Result<Vec<ContactFormRecord>, JsValue> {
    let window = web_sys::window().ok_or(JsValue::from_str("No window"))?;

    let resp = wasm_bindgen_futures::JsFuture::from(
        window.fetch_with_str("/api/contact-form")
    ).await?;

    let resp: web_sys::Response = resp.dyn_into()?;

    // Check if response is ok
    if !resp.ok() {
        let status = resp.status();
        return Err(JsValue::from_str(&format!("API request failed with status {}", status)));
    }

    let json = wasm_bindgen_futures::JsFuture::from(resp.json()?).await?;

    // Try to parse as Vec first, if it fails, check if it's an error object
    match serde_wasm_bindgen::from_value::<Vec<ContactFormRecord>>(json.clone()) {
        Ok(records) => Ok(records),
        Err(_) => {
            // Try to parse as error response
            #[derive(Deserialize)]
            struct ErrorResponse {
                error: Option<String>,
                message: Option<String>,
            }

            if let Ok(err_resp) = serde_wasm_bindgen::from_value::<ErrorResponse>(json) {
                let msg = err_resp.message
                    .or(err_resp.error)
                    .unwrap_or_else(|| "Unknown error from API".to_string());
                Err(JsValue::from_str(&msg))
            } else {
                Err(JsValue::from_str("Failed to parse API response"))
            }
        }
    }
}

#[component]
pub fn DatabaseTable() -> impl IntoView {
    let params = use_params_map();
    let table_name = move || params.with(|p| p.get("table").cloned().unwrap_or_default());

    let (records, set_records) = create_signal::<Vec<ContactFormRecord>>(vec![]);
    let (loading, set_loading) = create_signal(true);
    let (error, set_error) = create_signal::<Option<String>>(None);

    create_effect(move |_| {
        let table = table_name();
        if table == "contact-form" {
            spawn_local(async move {
                set_loading.set(true);
                set_error.set(None);

                match fetch_contact_forms().await {
                    Ok(data) => {
                        set_records.set(data);
                        set_loading.set(false);
                    }
                    Err(e) => {
                        set_error.set(Some(format!("Failed to load data: {:?}", e)));
                        set_loading.set(false);
                    }
                }
            });
        }
    });

    view! {
        <div class="p-8 bg-gray-50 dark:bg-zinc-900 min-h-screen">
            // Back button
            <div class="mb-6">
                <a href="/admin/database" class="inline-flex items-center gap-2 text-blue-600 dark:text-blue-400 hover:text-blue-700 dark:hover:text-blue-300 transition">
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 19l-7-7 7-7"></path>
                    </svg>
                    "Back to Database"
                </a>
            </div>

            // Page header
            <div class="mb-8">
                <h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">
                    {move || format!("Table: {}", table_name())}
                </h1>
                <p class="text-gray-600 dark:text-gray-400">"View and manage contact form submissions"</p>
            </div>

            // Content
            {move || {
                if loading.get() {
                    view! {
                        <div class="flex items-center justify-center h-64">
                            <div class="text-center">
                                <div class="inline-block animate-spin rounded-full h-12 w-12 border-b-2 border-blue-600"></div>
                                <p class="mt-4 text-gray-600 dark:text-gray-400">"Loading data..."</p>
                            </div>
                        </div>
                    }.into_view()
                } else if let Some(err) = error.get() {
                    view! {
                        <div class="bg-red-100 dark:bg-red-900/30 border border-red-400 dark:border-red-600 text-red-700 dark:text-red-300 px-4 py-3 rounded-lg">
                            <p class="font-bold">"Error"</p>
                            <p>{err}</p>
                        </div>
                    }.into_view()
                } else if records.get().is_empty() {
                    view! {
                        <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm p-8 text-center">
                            <p class="text-gray-600 dark:text-gray-400">"No contact form submissions found."</p>
                        </div>
                    }.into_view()
                } else {
                    view! {
                        <div class="bg-white dark:bg-zinc-800 rounded-lg shadow-sm overflow-hidden">
                            <div class="p-6 border-b border-gray-200 dark:border-zinc-700">
                                <h2 class="text-xl font-semibold text-gray-900 dark:text-white">
                                    {move || format!("Contact Form Submissions ({})", records.get().len())}
                                </h2>
                            </div>

                            <div class="overflow-x-auto">
                                <table class="w-full">
                                    <thead class="bg-gray-50 dark:bg-zinc-900 border-b border-gray-200 dark:border-zinc-700">
                                        <tr>
                                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"ID"</th>
                                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Name"</th>
                                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Email"</th>
                                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Phone"</th>
                                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Service"</th>
                                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Message"</th>
                                            <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 dark:text-gray-400 uppercase tracking-wider">"Submitted"</th>
                                        </tr>
                                    </thead>
                                    <tbody class="bg-white dark:bg-zinc-800 divide-y divide-gray-200 dark:divide-zinc-700">
                                        <For
                                            each=move || records.get()
                                            key=|record| record.id
                                            children=move |record| {
                                                let message_preview = if record.message.len() > 50 {
                                                    format!("{}...", &record.message[..50])
                                                } else {
                                                    record.message.clone()
                                                };
                                                let message_full = record.message.clone();
                                                let dept = if record.department.is_empty() {
                                                    "-".to_string()
                                                } else {
                                                    record.department.clone()
                                                };

                                                view! {
                                                    <tr class="hover:bg-gray-50 dark:hover:bg-zinc-700/50 transition-colors">
                                                        <td class="px-6 py-4 whitespace-nowrap text-sm font-medium text-gray-900 dark:text-gray-100">
                                                            {record.id}
                                                        </td>
                                                        <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-gray-100">
                                                            {record.name}
                                                        </td>
                                                        <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-gray-100">
                                                            {record.raw_email}
                                                        </td>
                                                        <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-gray-100">
                                                            {record.phone}
                                                        </td>
                                                        <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-900 dark:text-gray-100">
                                                            {dept}
                                                        </td>
                                                        <td class="px-6 py-4 text-sm text-gray-900 dark:text-gray-100">
                                                            <div class="max-w-xs truncate" title={message_full}>
                                                                {message_preview}
                                                            </div>
                                                        </td>
                                                        <td class="px-6 py-4 whitespace-nowrap text-sm text-gray-500 dark:text-gray-400">
                                                            {record.created_at}
                                                        </td>
                                                    </tr>
                                                }
                                            }
                                        />
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    }.into_view()
                }
            }}
        </div>
    }
}
