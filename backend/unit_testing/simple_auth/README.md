# Simple Authentication System

A Firebase/Cognito-style stateless JWT authentication system with minimal session tracking for revocation capabilities.

## Architecture

- **Stateless JWTs**: Primary authentication mechanism (like Firebase/Cognito)
- **RSA Signatures**: RS256 algorithm for cross-service validation
- **Public Key Distribution**: JWKS endpoint for other services
- **Minimal State**: Only users, refresh tokens, and JWT blacklist in libmdbx
- **Firebase-style User IDs**: 28-character URL-safe identifiers

## Features

### Authentication
- User registration with email/password
- Login with JWT token generation
- Token refresh mechanism
- Logout with token blacklisting
- Email validation and normalization

### Security
- PBKDF2-SHA256 password hashing (100,000 iterations)
- RSA-signed JWTs (RS256)
- Secure random user ID generation
- Token revocation via blacklist
- Public key validation for other services

### Cross-Service Support
- JWKS endpoint (`/.well-known/jwks.json`)
- Other services validate JWTs independently
- No shared database required between services

## API Endpoints

### Authentication
- `POST /auth/register` - Create new user account
- `POST /auth/login` - Authenticate and get JWT tokens
- `POST /auth/refresh` - Refresh access token
- `POST /auth/logout` - Blacklist JWT and logout
- `POST /auth/validate` - Validate JWT (local verification)

### Public Key Distribution
- `GET /.well-known/jwks.json` - JSON Web Key Set for other services

## JWT Structure

### Access Token Claims
```json
{
  "sub": "xK8fG2mNpQrS7vW9yB4cD6eH8jL",    // 28-char Firebase-style user ID
  "email": "user@example.com",             // User email
  "iat": 1516239022,                       // Issued at (Unix timestamp)
  "exp": 1516242622,                       // Expires at (Unix timestamp)
  "jti": "unique-jwt-id",                  // JWT ID for blacklisting
  "iss": "https://your-auth-server.com",   // Issuer
  "aud": ["service1", "service2"]          // Audience (authorized services)
}
```

### Token Types
- **Access Token**: Short-lived (15-60 minutes), contains user claims
- **Refresh Token**: Long-lived (30 days), for obtaining new access tokens

## Database Schema (libmdbx)

### 1. Users Database
- **Key**: `userId` (28 characters)
- **Value**: JSON `{"email": "...", "passwordHash": "...", "createdAt": 123456789}`

### 2. Email Index Database
- **Key**: `email` (lowercase)
- **Value**: `userId`
- **Purpose**: Fast email → userId lookup for login

### 3. Refresh Tokens Database
- **Key**: `tokenId` (unique identifier)
- **Value**: JSON `{"userId": "...", "tokenHash": "...", "expiresAt": 123456789}`
- **Purpose**: Validate refresh tokens and enable revocation

### 4. Blacklist Database
- **Key**: `jti` (JWT ID from claims)
- **Value**: `expirationTimestamp`
- **Purpose**: Revoke JWTs before natural expiration

## Usage

### Build and Setup
```bash
# Check dependencies
make check-deps

# Build all components
make all

# Generate RSA key pair
make setup-keys

# Run tests
make test
```

### Server Operation
```bash
# Start server
make run-server

# Or run directly
./server

# Stop server
make kill-server
```

### Testing
```bash
# Run automated test suite
./test_script.sh

# Run individual unit tests
./tests/test_userid
./tests/test_email
./tests/test_password
./tests/test_jwt
```

## Dependencies

- **libusockets**: HTTP server with SSL/TLS support
- **BoringSSL**: Cryptographic operations (RSA, PBKDF2, HMAC, random)
- **libmdbx**: High-performance ACID-compliant database
- **Standard libraries**: pthread, liburing, etc.

## File Structure

```
simple_auth/
├── Makefile              # Build configuration
├── README.md             # This documentation
├── test_script.sh        # Automated testing
├── server.c              # HTTP authentication server
├── client.c              # Test client
├── auth_lib/             # Core authentication library
│   ├── auth.h/c         # Main authentication API
│   ├── userid.h/c       # Firebase-style ID generation
│   ├── email.h/c        # Email validation
│   ├── password.h/c     # PBKDF2 password hashing
│   ├── jwt_rs256.h/c    # RSA-signed JWT implementation
│   ├── rsa_keys.h/c     # RSA key management
│   ├── jwks.h/c         # JWKS endpoint generation
│   └── base64url.h/c    # Base64URL encoding for JWTs
├── tests/                # Unit tests
├── keys/                 # RSA key storage
│   ├── private_key.pem  # RSA private key (server-only)
│   └── public_key.pem   # RSA public key (for verification)
└── data/                 # libmdbx databases
    ├── users*           # User accounts
    ├── email_index*     # Email → userId mapping
    ├── refresh_tokens*  # Refresh token storage
    └── blacklist*       # Revoked JWT IDs
```

## Security Considerations

- RSA private key must be protected (file permissions 600)
- Use HTTPS in production for token transmission
- Regular cleanup of expired blacklist entries
- Strong password requirements (8+ chars, mixed case, numbers)
- Rate limiting for login attempts (not implemented in this PoC)
- Token rotation for compromised keys

## Integration with Other Services

Other services can validate JWTs independently by:

1. Fetching the public key from `/.well-known/jwks.json`
2. Verifying the JWT signature using RS256
3. Validating claims (exp, iss, aud, etc.)
4. No database connection to auth service required

Example JWKS response:
```json
{
  "keys": [
    {
      "kty": "RSA",
      "kid": "auth-key-1",
      "use": "sig",
      "alg": "RS256",
      "n": "base64url-encoded-modulus...",
      "e": "AQAB"
    }
  ]
}
```