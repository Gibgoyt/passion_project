use crate::api::types::*;
use gloo_net::http::Request;

/// Get the backend API base URL from window or default to empty string (same origin)
fn get_backend_url() -> String {
    // For now, we'll use a hardcoded backend URL
    // In production, this should come from env vars or window.location
    "http://localhost:8080".to_string()
}

/// List all WhatsApp accounts
pub async fn list_accounts() -> Result<ListAccountsResponse, String> {
    let url = format!("{}/accounts", get_backend_url());

    let response = Request::get(&url)
        .send()
        .await
        .map_err(|e| format!("Failed to send request: {}", e))?;

    if response.ok() {
        response
            .json::<ListAccountsResponse>()
            .await
            .map_err(|e| format!("Failed to parse response: {}", e))
    } else {
        Err(format!("Request failed with status: {}", response.status()))
    }
}

/// Create a new WhatsApp account
pub async fn create_account(
    phone_number: String,
    device_name: Option<String>,
    platform: Option<String>,
) -> Result<CreateAccountResponse, String> {
    let url = format!("{}/accounts", get_backend_url());

    let request = CreateAccountRequest {
        phone_number,
        device_name,
        platform,
    };

    let response = Request::post(&url)
        .json(&request)
        .map_err(|e| format!("Failed to serialize request: {}", e))?
        .send()
        .await
        .map_err(|e| format!("Failed to send request: {}", e))?;

    if response.ok() {
        response
            .json::<CreateAccountResponse>()
            .await
            .map_err(|e| format!("Failed to parse response: {}", e))
    } else {
        let error_text = response
            .text()
            .await
            .unwrap_or_else(|_| "Unknown error".to_string());
        Err(format!("Request failed: {}", error_text))
    }
}

/// Get QR code for an account
pub async fn get_qr_code(jid: String) -> Result<QRCodeResponse, String> {
    let url = format!("{}/accounts/{}/qr", get_backend_url(), urlencoding::encode(&jid));

    let response = Request::get(&url)
        .send()
        .await
        .map_err(|e| format!("Failed to send request: {}", e))?;

    if response.ok() {
        response
            .json::<QRCodeResponse>()
            .await
            .map_err(|e| format!("Failed to parse response: {}", e))
    } else {
        let error_text = response
            .text()
            .await
            .unwrap_or_else(|_| "Unknown error".to_string());
        Err(format!("Request failed: {}", error_text))
    }
}

/// Delete an account
pub async fn delete_account(jid: String) -> Result<SendMessageResponse, String> {
    let url = format!("{}/accounts/{}", get_backend_url(), urlencoding::encode(&jid));

    let response = Request::delete(&url)
        .send()
        .await
        .map_err(|e| format!("Failed to send request: {}", e))?;

    if response.ok() {
        response
            .json::<SendMessageResponse>()
            .await
            .map_err(|e| format!("Failed to parse response: {}", e))
    } else {
        let error_text = response
            .text()
            .await
            .unwrap_or_else(|_| "Unknown error".to_string());
        Err(format!("Request failed: {}", error_text))
    }
}

/// Send a message from a specific account
pub async fn send_message(
    jid: String,
    to: String,
    message: String,
) -> Result<SendMessageResponse, String> {
    let url = format!("{}/accounts/{}/send", get_backend_url(), urlencoding::encode(&jid));

    let request = SendMessageRequest { to, message };

    let response = Request::post(&url)
        .json(&request)
        .map_err(|e| format!("Failed to serialize request: {}", e))?
        .send()
        .await
        .map_err(|e| format!("Failed to send request: {}", e))?;

    if response.ok() {
        response
            .json::<SendMessageResponse>()
            .await
            .map_err(|e| format!("Failed to parse response: {}", e))
    } else {
        let error_text = response
            .text()
            .await
            .unwrap_or_else(|_| "Unknown error".to_string());
        Err(format!("Request failed: {}", error_text))
    }
}

/// Get health status
pub async fn get_health() -> Result<HealthResponse, String> {
    let url = format!("{}/healthz", get_backend_url());

    let response = Request::get(&url)
        .send()
        .await
        .map_err(|e| format!("Failed to send request: {}", e))?;

    if response.ok() {
        response
            .json::<HealthResponse>()
            .await
            .map_err(|e| format!("Failed to parse response: {}", e))
    } else {
        Err(format!("Request failed with status: {}", response.status()))
    }
}
