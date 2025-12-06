# Hybrid JWT/Session Authentication System

## Executive Summary
Stateless JWT validation with stateful session management only during token refresh operations. Combines Firebase/Auth0-level performance with enterprise session control capabilities.

## Core Principle
**Sessions are ONLY checked during refresh token operations, never during regular validation.**

- 99% of requests: Pure stateless JWT validation (maximum performance)
- 1% of requests: Session validation during refresh (security control point)

## Authentication Flow

### 1. Login (`POST /auth/login`)
```
Email/Password → JWT Pair + Session Creation
├── Generate access token (10s lifetime, unique JWT ID)
├── Generate refresh token (100s lifetime, unique JWT ID)
├── Create session record (linked to refresh token ID)
└── Return: accessToken, refreshToken, userId
```

### 2. Validation (`POST /auth/validate`)
```
Access Token → Stateless Validation
├── Verify JWT signature (RS256)
├── Check expiration timestamp
├── Check blacklist database
├── NO session lookup
└── Return: user data or error
```

### 3. Refresh (`POST /auth/refresh`)
```
Refresh Token → Session Validation + Token Rotation
├── Validate refresh token cryptographically
├── Load session by session ID (ONLY session check in system)
├── Verify session exists and not expired
├── Generate NEW access token (fresh 10s lifetime)
├── Generate NEW refresh token (OAuth 2.0 rotation)
├── Blacklist old refresh token immediately
├── Update session with new token IDs
└── Return: newAccessToken, newRefreshToken
```

### 4. Logout (`POST /auth/logout`)
```
Access Token → Session + Token Cleanup
├── Validate access token
├── Add access token to blacklist
├── Find session by session ID
├── Add refresh token to blacklist
├── Delete session record
└── Return: success
```

## Token Management

### JWT Structure
- **Access Token**: 10s lifetime, `"token_type": "access"`, unique `jti`
- **Refresh Token**: 100s lifetime, `"token_type": "refresh"`, unique `jti`
- **Session ID**: 64-char embedded in both tokens as `"sid"` claim

### Token Rotation (OAuth 2.0 Compliant)
- Every refresh generates completely new access + refresh tokens
- Old refresh token immediately blacklisted (prevents reuse attacks)
- Session tracking updated with new token IDs
- Configurable via `enable_refresh_token_rotation` (default: enabled)

## Security Features

### OAuth 2.0 Security Best Practices
- ✅ Refresh token rotation (mandatory for public clients)
- ✅ Unique JWT IDs for proper blacklisting
- ✅ Immediate token invalidation on refresh
- ✅ Session-based revocation capabilities

### Attack Prevention
- **Token theft**: Old refresh tokens become invalid immediately
- **Replay attacks**: Blacklisted tokens cannot be reused
- **Session hijacking**: Sessions can be revoked independently
- **Token confusion**: Unique JWT IDs prevent cross-contamination

## Performance Model

### Request Distribution
```
Regular API calls:    /auth/validate (stateless, ~0.1ms)
Token refresh:        /auth/refresh  (session check, ~1ms)
Login/logout:         Infrequent operations
```

### Database Access Patterns
- **Users DB**: Read on login/refresh (rare)
- **Blacklist DB**: Check on every validation (fast key lookup)
- **Sessions DB**: Read/write ONLY during refresh (rare)
- **Email Index DB**: Lookup on login only (rare)

### Scalability Characteristics
- Horizontal scaling: No session affinity required for validation
- Microservice ready: JWTs validated independently
- Database efficient: Sessions touched only during refresh
- Memory efficient: No in-memory session storage required

## Architecture Benefits

### Best of Both Worlds
- **Stateless Performance**: 99% of requests avoid session lookups
- **Stateful Control**: Sessions provide immediate revocation capability
- **Security Compliance**: Meets OAuth 2.0 Security Best Practices
- **Enterprise Ready**: Device management, logout-all capabilities

### Comparison to Alternatives
- **Pure JWT**: ❌ No immediate revocation, ❌ No device management
- **Pure Sessions**: ❌ Poor horizontal scaling, ❌ Microservice complexity
- **Hybrid (This System)**: ✅ Fast validation, ✅ Security controls, ✅ OAuth compliant

## Configuration

### Key Settings
```c
config.access_token_lifetime = 10;           // 10 seconds (configurable)
config.refresh_token_lifetime = 100;         // 100 seconds (configurable)
config.enable_refresh_token_rotation = 1;    // OAuth 2.0 security (default: enabled)
```

### Production Recommendations
- Access tokens: 15-60 minutes
- Refresh tokens: 7-30 days
- Rotation: Always enabled for public clients
- Sessions: Regular cleanup of expired entries

## Technical Implementation

### Database Schema (MDBX)
- **Users**: `userId → user_data_t` (Firebase-style user storage)
- **Sessions**: `sessionId → session_data_t` (device/token tracking)
- **Blacklist**: `jwtId → expiration` (revoked token tracking)
- **Email Index**: `email → userId` (fast login lookup)

### JWT Claims Structure
```json
{
  "iss": "simple-auth",
  "sub": "userId",
  "aud": "simple-auth-clients",
  "iat": 1763976918,
  "exp": 1763976928,
  "jti": "unique-32-char-id",
  "email": "user@example.com",
  "email_verified": false,
  "sid": "64-char-session-id",
  "token_type": "access"
}
```

### Session Data Structure
```c
typedef struct {
    char session_id[64];                     // Unique session identifier
    char user_id[29];                        // User this session belongs to
    char refresh_token_id[33];               // Current refresh token JWT ID
    char access_token_id[33];                // Current access token JWT ID
    time_t created_at, last_used, expires_at; // Timestamps
    char user_agent[256], ip_address[64];    // Client information
} session_data_t;
```

This system achieves **enterprise-grade security** with **consumer-grade performance** by strategically limiting stateful operations to only refresh scenarios.