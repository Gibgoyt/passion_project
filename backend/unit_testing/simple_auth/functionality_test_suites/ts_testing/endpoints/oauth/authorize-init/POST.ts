/**
 * OAuth 2.1 PKCE Authorization Initialization Endpoint
 *
 * Fetches JSON data from POST /oauth/authorize/init
 *
 * @param client_id OAuth client identifier
 * @param redirect_uri Callback URI for OAuth flow
 * @param code_challenge PKCE code challenge (SHA256 hash of code_verifier)
 * @param code_challenge_method PKCE method, must be "S256"
 * @param state CSRF protection parameter
 * @returns A promise that resolves to the OAuth initialization response
 */

interface Response200 {
	status: 200
	data: {
		session_id: string
		authorization_url: string
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

type PostResponse = Response200 | Response400 | Response405 | Response500

export async function POST(
	client_id: string,
	redirect_uri: string,
	code_challenge: string,
	code_challenge_method: string,
	state: string
): Promise<PostResponse> {
	try {
		// Configuration
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"  // Disable SSL verification

		// Prepare request body
		const requestBody = {
			client_id: client_id,
			redirect_uri: redirect_uri,
			code_challenge: code_challenge,
			code_challenge_method: code_challenge_method,
			state: state
		}

		// Make fetch request
		const response = await fetch(
			BASE_URL + "/oauth/authorize/init", {
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
						session_id: responseData.session_id || '',
						authorization_url: responseData.authorization_url || '',
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
			console.log("failed to initialize OAuth PKCE flow")
			console.log("Error message: " + error.message)
		} else {
			console.log("an unknown error occurred")
		}
		throw error
	}
}