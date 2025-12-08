#!/usr/bin/env -S node --experimental-strip-types

import { Endpoints } from "./endpoints/index.ts"
import * as fs from 'node:fs'
import * as path from 'node:path'
import { webcrypto } from 'node:crypto'

/*
 * main function, because JavaScript sucks, and I like main functions
*/
async function main() {
	console.log("we are inside main()!!!")

	try {
		// Step 1: Login
		console.log("SENDING POST /auth/login")
		const loginResult = await Endpoints.Auth.Ropc.Login.POST("testuser1@example.com", "Testuser1!")

		console.log("\nPOST /auth/login response:")
		console.log("===========================\n")
		console.log(JSON.stringify(loginResult, null, 2))
		console.log("===========================\n")

		if (loginResult.status === 200) {
			console.log("✅ Login successful!")

			// Step 2: Validate the access token
			console.log("SENDING POST /auth/validate")
			const validateResult = await Endpoints.Auth.Validate.POST(loginResult.data.accessToken)

			console.log("\nPOST /auth/validate response:")
			console.log("===========================\n")
			console.log(JSON.stringify(validateResult, null, 2))
			console.log("===========================\n")

			if (validateResult.status === 200) {
				console.log("✅ Token validation successful!")
				console.log(`User: ${validateResult.data.email} (ID: ${validateResult.data.userId})`)
				console.log(`Email verified: ${validateResult.data.emailVerified}`)

				// Step 3: Refresh the access token
				console.log("SENDING POST /auth/refresh")
				const refreshResult = await Endpoints.Auth.Refresh.POST(loginResult.data.refreshToken)

				console.log("\nPOST /auth/refresh response:")
				console.log("===========================\n")
				console.log(JSON.stringify(refreshResult, null, 2))
				console.log("===========================\n")

				if (refreshResult.status === 200) {
					console.log("✅ Token refresh successful!")
					console.log(`New access token received for user: ${refreshResult.data.userId}`)
					console.log(`Tokens rotated successfully`)
				} else if (refreshResult.status === 401) {
					console.log("❌ Token refresh failed:")
					console.log(`Error: ${refreshResult.data.error}`)
				} else if (refreshResult.status === 405) {
					console.log("❌ Method not allowed:")
					console.log(`Error: ${refreshResult.data.error}`)
				} else if (refreshResult.status === 500) {
					console.log("❌ Server error during refresh:")
					console.log(`Error: ${refreshResult.data.error}`)
				}
			} else if (validateResult.status === 401) {
				console.log("❌ Token validation failed:")
				console.log(`Error: ${validateResult.data.error}`)
			} else if (validateResult.status === 405) {
				console.log("❌ Method not allowed:")
				console.log(`Error: ${validateResult.data.error}`)
			} else if (validateResult.status === 500) {
				console.log("❌ Server error during validation:")
				console.log(`Error: ${validateResult.data.error}`)
			}
		} else if (loginResult.status === 400) {
			console.log("❌ Login failed - Bad Request:")
			console.log(`Error: ${loginResult.data.error}`)
		} else if (loginResult.status === 401) {
			console.log("❌ Login failed - Unauthorized:")
			console.log(`Error: ${loginResult.data.error}`)
		} else if (loginResult.status === 405) {
			console.log("❌ Login failed - Method Not Allowed:")
			console.log(`Error: ${loginResult.data.error}`)
		} else if (loginResult.status === 500) {
			console.log("❌ Login failed - Server Error:")
			console.log(`Error: ${loginResult.data.error}`)
		}
	} catch (error) {
		console.error("💥 Unexpected error (network/parsing):", error)
	}
}

// Sleep utility function
const sleep = (ms: number) => new Promise(resolve => setTimeout(resolve, ms))

/*
 * initialLogin_accessToken_lifecycle function - tests ONLY initial access token lifetime
 *
 * 1. Login once to get access token
 * 2. Validate access token every 500ms until it expires
 * 3. Log all responses to debug_logs/initialLogin_accessToken_lifecycle/NNNN.json files
 * 4. Exit when access token expires (no refresh attempt)
 */
async function initialLogin_accessToken_lifecycle() {
	console.log("Starting initial login access token lifecycle test...")

	// Check if debug directory exists
	const debugDir = 'debug_logs/initialLogin_accessToken_lifecycle'
	if (!fs.existsSync(debugDir)) {
		console.error(`❌ Directory ${debugDir}/ does not exist. Please create it first:`)
		console.error(`   mkdir -p ${debugDir}`)
		process.exit(1)
	}

	try {
		// Step 1: Login
		console.log("Attempting login...")
		const loginResult = await Endpoints.Auth.Ropc.Login.POST("testuser1@example.com", "Testuser1!")

		if (loginResult.status !== 200) {
			console.error("❌ Login failed!")
			console.error(`Status: ${loginResult.status}`)
			if ('error' in loginResult.data) {
				console.error(`Error: ${loginResult.data.error}`)
			}
			process.exit(1)
		}

		console.log("✅ Login successful! Starting validation loop...")
		const accessToken = loginResult.data.accessToken

		// Step 2: Validation loop
		let requestNumber = 1
		let keepGoing = true
		const startTime = Date.now()

		while (keepGoing) {
			const elapsedMs = (requestNumber - 1) * 500 // 0, 500, 1000, 1500...
			const filename = `${elapsedMs.toString().padStart(5, '0')}.json`
			const filepath = path.join(debugDir, filename)

			console.log(`Request ${requestNumber}...`)

			// Make validation request
			const validateResult = await Endpoints.Auth.Validate.POST(accessToken)

			// Create log entry
			const logEntry = {
				requestNumber: requestNumber,
				elapsedMs: elapsedMs,
				timestamp: new Date().toISOString(),
				accessToken: accessToken,
				response: validateResult
			}

			// Write to file
			fs.writeFileSync(filepath, JSON.stringify(logEntry, null, 2))

			// Check if we should continue
			if (validateResult.status !== 200) {
				console.log(`❌ Token validation failed at request ${requestNumber} (${elapsedMs}ms)`)
				console.log("Test complete. Logs saved to debug_logs/basic_lifecycle/")
				keepGoing = false
			} else {
				// Wait 500ms before next request
				await sleep(500)
				requestNumber++
			}
		}

	} catch (error) {
		console.error("💥 Unexpected error during access token lifecycle test:", error)
		process.exit(1)
	}
}

/*
 * initialLogin_refreshToken_lifecycle function - tests initial refresh token reuse capability
 *
 * 1. Login once to get access token and refresh token
 * 2. Validate access token every 500ms until it expires
 * 3. When access token expires, use SAME initial refresh token to get new access token
 * 4. Repeat cycle with new access token until initial refresh token expires
 * 5. Log all responses to debug_logs/initialLogin_refreshToken_lifecycle/C_NNNN.json files
 */
async function initialLogin_refreshToken_lifecycle() {
	console.log("Starting initial login refresh token lifecycle test...")

	// Check if debug directory exists
	const debugDir = 'debug_logs/initialLogin_refreshToken_lifecycle'
	if (!fs.existsSync(debugDir)) {
		console.error(`❌ Directory ${debugDir}/ does not exist. Please create it first:`)
		console.error(`   mkdir -p ${debugDir}`)
		process.exit(1)
	}

	try {
		// Step 1: Login
		console.log("Attempting login...")
		const loginResult = await Endpoints.Auth.Ropc.Login.POST("testuser1@example.com", "Testuser1!")

		if (loginResult.status !== 200) {
			console.error("❌ Login failed!")
			console.error(`Status: ${loginResult.status}`)
			if ('error' in loginResult.data) {
				console.error(`Error: ${loginResult.data.error}`)
			}
			process.exit(1)
		}

		console.log("✅ Login successful! Starting refresh token cycling test...")

		// Store tokens - keep initial refresh token, update access token after each refresh
		const initialRefreshToken = loginResult.data.refreshToken
		let currentAccessToken = loginResult.data.accessToken
		let cycleNumber = 0

		// Main refresh cycling loop
		while (true) {
			console.log(`=== CYCLE ${cycleNumber} ===`)

			// Validation loop within current cycle
			let requestNumber = 1
			let cycleActive = true

			while (cycleActive) {
				const elapsedMs = (requestNumber - 1) * 500 // 0, 500, 1000, 1500...
				const filename = `${cycleNumber}_${elapsedMs.toString().padStart(5, '0')}.json`
				const filepath = path.join(debugDir, filename)

				console.log(`Request ${requestNumber}...`)

				// Make validation request
				const validateResult = await Endpoints.Auth.Validate.POST(currentAccessToken)

				// Create enhanced log entry
				const logEntry = {
					cycleNumber: cycleNumber,
					requestNumber: requestNumber,
					elapsedMs: elapsedMs,
					timestamp: new Date().toISOString(),
					accessToken: currentAccessToken,
					refreshToken: initialRefreshToken,
					response: validateResult
				}

				// Write to file
				fs.writeFileSync(filepath, JSON.stringify(logEntry, null, 2))

				// Check if access token expired
				if (validateResult.status !== 200) {
					console.log(`❌ Access token expired at request ${requestNumber} (${elapsedMs}ms)`)
					console.log("Attempting refresh...")

					// Try to refresh using SAME initial refresh token
					const refreshResult = await Endpoints.Auth.Refresh.POST(initialRefreshToken)

					if (refreshResult.status === 200) {
						console.log("✅ Refresh successful! Starting new cycle...")
						// Update access token, keep same refresh token
						currentAccessToken = refreshResult.data.accessToken
						cycleNumber++
						cycleActive = false // Exit validation loop, start new cycle
					} else {
						console.log(`❌ Refresh failed! Refresh token expired.`)
						console.log(`Status: ${refreshResult.status}`)
						if ('error' in refreshResult.data) {
							console.log(`Error: ${refreshResult.data.error}`)
						}
						console.log(`Test complete. Total cycles: ${cycleNumber + 1}`)
						console.log("Logs saved to debug_logs/initialLogin_refreshToken_lifecycle/")
						return // Exit completely
					}
				} else {
					// Access token still valid, continue validation loop
					await sleep(500)
					requestNumber++
				}
			}
		}

	} catch (error) {
		console.error("💥 Unexpected error during refresh token lifecycle test:", error)
		process.exit(1)
	}
}

/*
 * rotated_refreshToken_lifecycle function - tests OAuth 2.0 refresh token rotation
 *
 * 1. Login once to get access token and refresh token
 * 2. Validate access token every 500ms until it expires
 * 3. When access token expires, use current refresh token to get NEW tokens
 * 4. Update BOTH access token AND refresh token from refresh response
 * 5. Repeat cycle with rotated tokens until refresh token chain expires
 * 6. Log all responses to debug_logs/rotated_refreshToken_lifecycle/C_NNNN.json files
 */
async function rotated_refreshToken_lifecycle() {
	console.log("Starting rotated refresh token lifecycle test...")

	// Check if debug directory exists
	const debugDir = 'debug_logs/rotated_refreshToken_lifecycle'
	if (!fs.existsSync(debugDir)) {
		console.error(`❌ Directory ${debugDir}/ does not exist. Please create it first:`)
		console.error(`   mkdir -p ${debugDir}`)
		process.exit(1)
	}

	try {
		// Step 1: Login
		console.log("Attempting login...")
		const loginResult = await Endpoints.Auth.Ropc.Login.POST("testuser1@example.com", "Testuser1!")

		if (loginResult.status !== 200) {
			console.error("❌ Login failed!")
			console.error(`Status: ${loginResult.status}`)
			if ('error' in loginResult.data) {
				console.error(`Error: ${loginResult.data.error}`)
			}
			process.exit(1)
		}

		console.log("✅ Login successful! Starting OAuth 2.0 token rotation test...")

		// Store tokens - CRITICAL: Track current refresh token and update after each refresh
		let currentRefreshToken = loginResult.data.refreshToken  // MUTABLE - updates each cycle
		let currentAccessToken = loginResult.data.accessToken
		let cycleNumber = 0
		let rotationCount = 0  // Track total rotations performed

		// Main refresh cycling loop
		while (true) {
			console.log(`=== CYCLE ${cycleNumber} (Rotations: ${rotationCount}) ===`)

			// Validation loop within current cycle
			let requestNumber = 1
			let cycleActive = true

			while (cycleActive) {
				const elapsedMs = (requestNumber - 1) * 500 // 0, 500, 1000, 1500...
				const filename = `${cycleNumber}_${elapsedMs.toString().padStart(5, '0')}.json`
				const filepath = path.join(debugDir, filename)

				console.log(`Request ${requestNumber}...`)

				// Make validation request
				const validateResult = await Endpoints.Auth.Validate.POST(currentAccessToken)

				// Create enhanced log entry with rotation metadata
				const logEntry = {
					cycleNumber: cycleNumber,
					requestNumber: requestNumber,
					elapsedMs: elapsedMs,
					timestamp: new Date().toISOString(),
					accessToken: currentAccessToken,
					refreshToken: currentRefreshToken,
					isRotatedToken: cycleNumber > 0,  // false for cycle 0, true for rotated cycles
					rotationCount: rotationCount,
					response: validateResult
				}

				// Write to file
				fs.writeFileSync(filepath, JSON.stringify(logEntry, null, 2))

				// Check if access token expired
				if (validateResult.status !== 200) {
					console.log(`❌ Access token expired at request ${requestNumber} (${elapsedMs}ms)`)
					console.log("Attempting refresh with current refresh token...")

					// Try to refresh using CURRENT refresh token (the key difference!)
					const refreshResult = await Endpoints.Auth.Refresh.POST(currentRefreshToken)

					if (refreshResult.status === 200) {
						console.log("✅ Refresh successful! Tokens rotated.")

						// CRITICAL: Update BOTH tokens from refresh response
						currentAccessToken = refreshResult.data.accessToken
						currentRefreshToken = refreshResult.data.refreshToken  // ← THIS IS THE KEY FIX

						cycleNumber++
						rotationCount++
						cycleActive = false // Exit validation loop, start new cycle
					} else {
						console.log(`❌ Refresh failed! Token rotation chain expired.`)
						console.log(`Status: ${refreshResult.status}`)
						if ('error' in refreshResult.data) {
							console.log(`Error: ${refreshResult.data.error}`)
						}
						console.log(`Test complete.`)
						console.log(`Total cycles: ${cycleNumber + 1}`)
						console.log(`Total rotations: ${rotationCount}`)
						console.log(`Duration: ~${(cycleNumber + 1) * 10} seconds`)
						console.log("Logs saved to debug_logs/rotated_refreshToken_lifecycle/")
						return // Exit completely
					}
				} else {
					// Access token still valid, continue validation loop
					await sleep(500)
					requestNumber++
				}
			}
		}

	} catch (error) {
		console.error("💥 Unexpected error during rotated refresh token lifecycle test:", error)
		process.exit(1)
	}
}

/*
 * PKCE utility functions for OAuth 2.1 flow
 */

// Generate PKCE code verifier (43-128 characters, URL-safe)
function generateCodeVerifier(): string {
	const array = new Uint8Array(32)
	webcrypto.getRandomValues(array)
	return base64URLEncode(array)
}

// Generate PKCE code challenge from verifier (SHA256 + base64url)
async function generateCodeChallenge(verifier: string): Promise<string> {
	const encoder = new TextEncoder()
	const data = encoder.encode(verifier)
	const digest = await webcrypto.subtle.digest('SHA-256', data)
	return base64URLEncode(new Uint8Array(digest))
}

// Generate random state parameter for CSRF protection
function generateState(): string {
	const array = new Uint8Array(32)
	webcrypto.getRandomValues(array)
	return base64URLEncode(array)
}

// Base64URL encoding (no padding)
function base64URLEncode(array: Uint8Array): string {
	return Buffer.from(array)
		.toString('base64')
		.replace(/\+/g, '-')
		.replace(/\//g, '_')
		.replace(/=/g, '')
}

/*
 * OAuth 2.1 PKCE flow test function
 */
async function test_pkceFlow() {
	console.log("🧪 Starting OAuth 2.1 PKCE flow test...")

	try {
		// Step 1: Generate PKCE parameters
		const codeVerifier = generateCodeVerifier()
		const codeChallenge = await generateCodeChallenge(codeVerifier)
		const state = generateState()

		console.log(`📋 PKCE Parameters generated:`)
		console.log(`   Code verifier: ${codeVerifier.substring(0, 10)}...`)
		console.log(`   Code challenge: ${codeChallenge.substring(0, 10)}...`)
		console.log(`   State: ${state.substring(0, 10)}...`)

		// Step 2: Initialize OAuth flow
		console.log("\nSENDING POST /oauth/authorize/init")
		const initResult = await Endpoints.OAuth.AuthorizeInit.POST(
			"astro-pkce-test",
			"http://localhost:3000/callback",
			codeChallenge,
			"S256",
			state
		)

		console.log("\nPOST /oauth/authorize/init response:")
		console.log("===========================\n")
		console.log(JSON.stringify(initResult, null, 2))
		console.log("===========================\n")

		if (initResult.status === 200) {
			console.log("✅ PKCE initialization successful!")
			console.log(`   Session ID: ${initResult.data.session_id}`)
			console.log(`   Authorization URL: ${initResult.data.authorization_url}`)

			// Step 3: Complete authorization (simulate user login)
			console.log("\nSENDING POST /oauth/authorize/complete")
			const completeResult = await Endpoints.OAuth.AuthorizeComplete.POST(
				initResult.data.session_id,
				"testuser1@example.com",
				"Testuser1!",
				true
			)

			console.log("\nPOST /oauth/authorize/complete response:")
			console.log("===========================\n")
			console.log(JSON.stringify(completeResult, null, 2))
			console.log("===========================\n")

			if (completeResult.status === 200) {
				console.log("✅ User authorization successful!")
				console.log(`   Authorization code: ${completeResult.data.authorization_code.substring(0, 10)}...`)

				// Step 4: Exchange authorization code for tokens
				console.log("\nSENDING POST /oauth/token")
				const tokenResult = await Endpoints.OAuth.Token.POST(
					"authorization_code",
					completeResult.data.authorization_code,
					codeVerifier,
					"astro-pkce-test",
					"http://localhost:3000/callback"
				)

				console.log("\nPOST /oauth/token response:")
				console.log("===========================\n")
				console.log(JSON.stringify(tokenResult, null, 2))
				console.log("===========================\n")

				if (tokenResult.status === 200) {
					console.log("✅ Token exchange successful!")
					console.log(`   Access token: ${tokenResult.data.access_token.substring(0, 20)}...`)
					console.log(`   Refresh token: ${tokenResult.data.refresh_token.substring(0, 20)}...`)
					console.log(`   Token type: ${tokenResult.data.token_type}`)
					console.log(`   Expires in: ${tokenResult.data.expires_in} seconds`)

					// Step 5: Validate the access token
					console.log("\nSENDING POST /auth/validate")
					const validateResult = await Endpoints.Auth.Validate.POST(tokenResult.data.access_token)

					console.log("\nPOST /auth/validate response:")
					console.log("===========================\n")
					console.log(JSON.stringify(validateResult, null, 2))
					console.log("===========================\n")

					if (validateResult.status === 200) {
						console.log("✅ Token validation successful!")
						console.log(`   User: ${validateResult.data.email} (ID: ${validateResult.data.userId})`)
						console.log(`   Email verified: ${validateResult.data.emailVerified}`)

						// Step 6: Test refresh token
						console.log("\nSENDING POST /auth/refresh")
						const refreshResult = await Endpoints.Auth.Refresh.POST(tokenResult.data.refresh_token)

						console.log("\nPOST /auth/refresh response:")
						console.log("===========================\n")
						console.log(JSON.stringify(refreshResult, null, 2))
						console.log("===========================\n")

						if (refreshResult.status === 200) {
							console.log("✅ Token refresh successful!")
							console.log(`   New access token: ${refreshResult.data.accessToken.substring(0, 20)}...`)
							console.log(`   New refresh token: ${refreshResult.data.refreshToken.substring(0, 20)}...`)

							console.log("\n🎉 PKCE flow completed successfully!")
							console.log("✅ All OAuth 2.1 PKCE steps passed:")
							console.log("   1. ✅ OAuth flow initialization")
							console.log("   2. ✅ User authentication and consent")
							console.log("   3. ✅ PKCE code exchange")
							console.log("   4. ✅ Token validation")
							console.log("   5. ✅ Token refresh")
						} else if (refreshResult.status === 401) {
							console.log("❌ Token refresh failed - Unauthorized:")
							console.log(`Error: ${refreshResult.data.error}`)
							process.exit(1)
						} else {
							console.log(`❌ Token refresh failed with status ${refreshResult.status}`)
							process.exit(1)
						}

					} else if (validateResult.status === 401) {
						console.log("❌ Token validation failed - Unauthorized:")
						console.log(`Error: ${validateResult.data.error}`)
						process.exit(1)
					} else {
						console.log(`❌ Token validation failed with status ${validateResult.status}`)
						process.exit(1)
					}

				} else if (tokenResult.status === 400) {
					console.log("❌ Token exchange failed - Bad Request:")
					console.log(`Error: ${tokenResult.data.error}`)
					console.log(`Description: ${tokenResult.data.error_description}`)
					process.exit(1)
				} else if (tokenResult.status === 401) {
					console.log("❌ Token exchange failed - PKCE verification failed:")
					console.log(`Error: ${tokenResult.data.error}`)
					console.log(`Description: ${tokenResult.data.error_description}`)
					process.exit(1)
				} else {
					console.log(`❌ Token exchange failed with status ${tokenResult.status}`)
					process.exit(1)
				}

			} else if (completeResult.status === 401) {
				console.log("❌ User authorization failed - Invalid credentials:")
				console.log(`Error: ${completeResult.data.error}`)
				console.log(`Description: ${completeResult.data.error_description}`)
				process.exit(1)
			} else if (completeResult.status === 400) {
				console.log("❌ User authorization failed - Bad Request:")
				console.log(`Error: ${completeResult.data.error}`)
				console.log(`Description: ${completeResult.data.error_description}`)
				process.exit(1)
			} else {
				console.log(`❌ User authorization failed with status ${completeResult.status}`)
				process.exit(1)
			}

		} else if (initResult.status === 400) {
			console.log("❌ PKCE initialization failed - Bad Request:")
			console.log(`Error: ${initResult.data.error}`)
			console.log(`Description: ${initResult.data.error_description}`)
			process.exit(1)
		} else {
			console.log(`❌ PKCE initialization failed with status ${initResult.status}`)
			process.exit(1)
		}

	} catch (error) {
		console.error("💥 Unexpected error during PKCE flow test:", error)
		process.exit(1)
	}
}

// Command-line argument routing
const args = process.argv.slice(2)
if (args[0] === 'initialLogin_accessToken_lifecycle') {
	initialLogin_accessToken_lifecycle().catch(console.error)
} else if (args[0] === 'initialLogin_refreshToken_lifecycle') {
	initialLogin_refreshToken_lifecycle().catch(console.error)
} else if (args[0] === 'rotated_refreshToken_lifecycle') {
	rotated_refreshToken_lifecycle().catch(console.error)
} else if (args[0] === 'test_pkceFlow') {
	test_pkceFlow().catch(console.error)
} else if (args[0] === 'basic_lifecycle') {
	// Backward compatibility
	initialLogin_accessToken_lifecycle().catch(console.error)
} else {
	main().catch(console.error)
}
