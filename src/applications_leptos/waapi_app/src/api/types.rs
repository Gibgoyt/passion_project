use serde::{Deserialize, Serialize};

// WhatsApp Account types
#[derive(Clone, Debug, Serialize, Deserialize, PartialEq)]
pub struct WhatsAppAccount {
    pub jid: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub phone_number: Option<String>,
    pub device_name: String,
    pub platform: String,
    pub connected: bool,
    pub authenticated: bool,
    pub created_at: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub last_seen: Option<String>,
}

// Dashboard stats
#[derive(Clone, Debug, Serialize, Deserialize, PartialEq)]
pub struct WaapiStats {
    pub total_accounts: i32,
    pub connected_accounts: i32,
    pub authenticated_accounts: i32,
}

// Initial data response
#[derive(Clone, Debug, Serialize, Deserialize, PartialEq)]
pub struct InitialData {
    pub accounts: Vec<WhatsAppAccount>,
    pub stats: WaapiStats,
}

// API Response wrapper
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ApiResponse<T> {
    pub success: bool,
    pub data: Option<T>,
    pub error: Option<String>,
    pub message: Option<String>,
}

// List Accounts Response
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ListAccountsResponse {
    pub success: bool,
    pub message: String,
    pub accounts: Vec<WhatsAppAccount>,
    pub count: i32,
}

// Create Account Request
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct CreateAccountRequest {
    pub phone_number: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub device_name: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub platform: Option<String>,
}

// Create Account Response
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct CreateAccountResponse {
    pub success: bool,
    pub message: String,
    pub session_id: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub jid: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub qr_code: Option<String>,
    pub authenticated: bool,
}

// QR Code Response
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct QRCodeResponse {
    pub success: bool,
    pub message: String,
    pub qr_code: String,
    pub authenticated: bool,
    pub jid: String,
}

// Send Message Request
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct SendMessageRequest {
    pub to: String,
    pub message: String,
}

// Send Message Response
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct SendMessageResponse {
    pub success: bool,
    pub message: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub to: Option<String>,
}

// Health Check Response
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct HealthResponse {
    pub status: String,
    pub timestamp: String,
    pub whatsapp_connected: bool,
    pub version: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub sessions: Option<Vec<WhatsAppAccount>>,
    pub total_sessions: i32,
}
