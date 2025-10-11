use leptos::*;
use crate::api::types::*;

/// Global app state shared across all pages
#[derive(Clone, Debug)]
pub struct WaapiStore {
    pub accounts: RwSignal<Vec<WhatsAppAccount>>,
    pub selected_account: RwSignal<Option<String>>, // JID of selected account
    pub qr_code: RwSignal<Option<String>>,
    pub stats: RwSignal<WaapiStats>,
    pub loading: RwSignal<bool>,
    pub error: RwSignal<Option<String>>,
}

impl WaapiStore {
    /// Create a new store with empty data
    pub fn new() -> Self {
        Self {
            accounts: create_rw_signal(Vec::new()),
            selected_account: create_rw_signal(None),
            qr_code: create_rw_signal(None),
            stats: create_rw_signal(WaapiStats::default()),
            loading: create_rw_signal(false),
            error: create_rw_signal(None),
        }
    }

    /// Initialize store from window.waapiInitialData
    pub fn init_from_window(&self) {
        if let Some(initial_data) = get_initial_data_from_window() {
            self.accounts.set(initial_data.accounts);
            self.stats.set(initial_data.stats);

            logging::log!("✅ Store initialized from window.waapiInitialData");
        } else {
            logging::warn!("⚠️ No initial data found in window - this is expected since we load from backend API");
        }
    }

    /// Update store with new data from API
    pub fn update(&self, data: InitialData) {
        self.accounts.set(data.accounts);
        self.stats.set(data.stats);
    }

    /// Update accounts list
    pub fn update_accounts(&self, accounts: Vec<WhatsAppAccount>) {
        // Calculate stats from accounts
        let total = accounts.len() as i32;
        let connected = accounts.iter().filter(|a| a.connected).count() as i32;
        let authenticated = accounts.iter().filter(|a| a.authenticated).count() as i32;

        self.accounts.set(accounts);
        self.stats.set(WaapiStats {
            total_accounts: total,
            connected_accounts: connected,
            authenticated_accounts: authenticated,
        });
    }

    /// Set selected account by JID
    pub fn set_selected_account(&self, jid: Option<String>) {
        self.selected_account.set(jid);
    }

    /// Set QR code
    pub fn set_qr_code(&self, qr_code: Option<String>) {
        self.qr_code.set(qr_code);
    }

    /// Set loading state
    pub fn set_loading(&self, loading: bool) {
        self.loading.set(loading);
    }

    /// Set error state
    pub fn set_error(&self, error: Option<String>) {
        self.error.set(error);
    }
}

impl Default for WaapiStats {
    fn default() -> Self {
        Self {
            total_accounts: 0,
            connected_accounts: 0,
            authenticated_accounts: 0,
        }
    }
}

/// Get initial data from window.waapiInitialData (set by server-side render)
fn get_initial_data_from_window() -> Option<InitialData> {
    use wasm_bindgen::JsValue;

    let window = web_sys::window()?;
    let waapi_initial_data = js_sys::Reflect::get(&window, &JsValue::from_str("waapiInitialData"))
        .ok()?;

    if waapi_initial_data.is_undefined() || waapi_initial_data.is_null() {
        return None;
    }

    serde_wasm_bindgen::from_value(waapi_initial_data).ok()
}
