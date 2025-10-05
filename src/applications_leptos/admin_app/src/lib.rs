use leptos::*;
use leptos_router::*;
use wasm_bindgen::prelude::*;
use wasm_bindgen::JsCast;

mod components;
mod pages;

use components::{Sidebar, Header, DarkMode};
use pages::{Dashboard, Database, DatabaseTable, Crm, Cicd, Settings};

fn initialize_dark_mode() -> bool {
    if let Some(window) = web_sys::window() {
        // Check localStorage first
        if let Ok(Some(storage)) = window.local_storage() {
            if let Ok(Some(stored)) = storage.get_item("darkMode") {
                return stored == "true";
            }
        }

        // Check document element class
        if let Some(document) = window.document() {
            if let Some(html) = document.document_element() {
                if html.class_list().contains("dark") {
                    return true;
                }
            }
        }

        // Check system preference
        if let Ok(media_query) = window.match_media("(prefers-color-scheme: dark)") {
            if let Some(query) = media_query {
                return query.matches();
            }
        }
    }
    false
}

#[component]
fn App() -> impl IntoView {
    // Initialize dark mode from localStorage/system preference/DOM
    let is_dark = initialize_dark_mode();
    let dark_mode = create_rw_signal(is_dark);
    provide_context(DarkMode(dark_mode));

    // Create derived signal for dynamic classes
    let bg_class = move || {
        if dark_mode.get() {
            "min-h-screen bg-zinc-900 text-zinc-100"
        } else {
            "min-h-screen bg-gray-50 text-gray-900"
        }
    };

    view! {
        <Router>
            <div class=bg_class>
                <div class="flex h-screen overflow-hidden">
                    <Sidebar />
                    <div class="flex flex-col flex-1 overflow-hidden">
                        <Header />
                        <main class="flex-1 overflow-y-auto">
                            <Routes>
                                <Route path="/admin" view=Dashboard/>
                                <Route path="/admin/dashboard" view=Dashboard/>
                                <Route path="/admin/database" view=Database/>
                                <Route path="/admin/database/:table" view=DatabaseTable/>
                                <Route path="/admin/crm" view=Crm/>
                                <Route path="/admin/ci-cd" view=Cicd/>
                                <Route path="/admin/settings" view=Settings/>
                            </Routes>
                        </main>
                    </div>
                </div>
            </div>
        </Router>
    }
}

#[wasm_bindgen]
pub fn mount_admin_app() -> Result<(), JsValue> {
    console_error_panic_hook::set_once();

    let container = web_sys::window()
        .ok_or_else(|| JsValue::from_str("No window found"))?
        .document()
        .ok_or_else(|| JsValue::from_str("No document found"))?
        .get_element_by_id("wasm-root")
        .ok_or_else(|| JsValue::from_str("No #wasm-root element found"))?
        .dyn_into::<web_sys::HtmlElement>()?;

    mount_to(container, || view! { <App/> });

    Ok(())
}
