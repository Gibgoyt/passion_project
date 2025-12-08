/**
 * OAuth 2.1 PKCE Token Exchange Endpoint
 *
 * Fetches JSON data from POST /oauth/token
 *
 * @param grant_type Grant type, must be "authorization_code"
 * @param code Authorization code from completion step
 * @param code_verifier PKCE code verifier for validation
 * @param client_id OAuth client identifier
 * @param redirect_uri Callback URI, must match initialization
 * @returns A promise that resolves to the OAuth token response
 */

interface Response200 {
	status: 200
	data: {
		access_token: string
		refresh_token: string
		token_type: string
		expires_in: number
	}
}

interface Response400 {
	status: 400
	data: {
		error: string
		error_description: string
	}
}

interface Response401 {
	status: 401
	data: {
		error: string
		error_description: string
	}
}

interface Response405 {
	status: 405
	data: {
		error: string
	}
}

interface Response500 {
	status: 500
	data: {
		error: string
	}
}

type PostResponse = Response200 | Response400 | Response401 | Response405 | Response500

export async function POST(
	grant_type: string,
	code: string,
	code_verifier: string,
	client_id: string,
	redirect_uri: string
): Promise<PostResponse> {
	try {
		// Configuration
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"  // Disable SSL verification

		// Prepare request body
		const requestBody = {
			grant_type: grant_type,
			code: code,
			code_verifier: code_verifier,
			client_id: client_id,
			redirect_uri: redirect_uri
		}

		// Make fetch request
		const response = await fetch(
			BASE_URL + "/oauth/token", {
				method: "POST",
				headers: {
					"Content-Type": "application/json"
				},
				body: JSON.stringify(requestBody)
			}
		)

		// Parse response
		const responseData = await response.json()

		// Status-based switch statement
		switch (response.status) {
			case 200:
				return {
					status: 200,
					data: {
						access_token: responseData.access_token || '',
						refresh_token: responseData.refresh_token || '',
						token_type: responseData.token_type || 'Bearer',
						expires_in: responseData.expires_in || 0
					}
				}

			case 400:
				return {
					status: 400,
					data: {
						error: responseData.error || 'invalid_request',
						error_description: responseData.error_description || 'Bad Request'
					}
				}

			case 401:
				return {
					status: 401,
					data: {
						error: responseData.error || 'invalid_grant',
						error_description: responseData.error_description || 'Invalid authorization grant'
					}
				}

			case 405:
				return {
					status: 405,
					data: {
						error: responseData.error || 'Method not allowed'
					}
				}

			case 500:
				return {
					status: 500,
					data: {
						error: responseData.error || 'Internal server error'
					}
				}

			default:
				throw new Error(`Unexpected HTTP status: ${response.status}`)
		}

	} catch (error: unknown) {
		if (error instanceof Error) {
			console.log("failed to exchange authorization code for tokens")
			console.log("Error message: " + error.message)
		} else {
			console.log("an unknown error occurred")
		}
		throw error
	}
}