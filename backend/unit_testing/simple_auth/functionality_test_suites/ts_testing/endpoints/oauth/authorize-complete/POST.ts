/**
 * OAuth 2.1 PKCE Authorization Completion Endpoint
 *
 * Fetches JSON data from POST /oauth/authorize/complete
 *
 * @param session_id OAuth session identifier from initialization
 * @param email User email address for authentication
 * @param password User password for authentication
 * @param consent_granted Whether user granted authorization consent
 * @returns A promise that resolves to the OAuth completion response
 */

interface Response200 {
	status: 200
	data: {
		authorization_code: string
		redirect_uri: string
		state: string
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
	session_id: string,
	email: string,
	password: string,
	consent_granted: boolean
): Promise<PostResponse> {
	try {
		// Configuration
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"  // Disable SSL verification

		// Prepare request body
		const requestBody = {
			session_id: session_id,
			email: email,
			password: password,
			consent_granted: consent_granted
		}

		// Make fetch request
		const response = await fetch(
			BASE_URL + "/oauth/authorize/complete", {
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
						authorization_code: responseData.authorization_code || '',
						redirect_uri: responseData.redirect_uri || '',
						state: responseData.state || ''
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
						error_description: responseData.error_description || 'Invalid credentials'
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
			console.log("failed to complete OAuth authorization")
			console.log("Error message: " + error.message)
		} else {
			console.log("an unknown error occurred")
		}
		throw error
	}
}