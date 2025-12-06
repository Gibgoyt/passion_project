interface Response200 {
	status: 200
	data: {
		valid: true
		userId: string
		email: string
		emailVerified: boolean
	}
}

interface Response401 {
	status: 401
	data: {
		valid: false
		error: string
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
		valid?: false
		error: string
	}
}

type PostResponse = Response200 | Response401 | Response405 | Response500

/*
 * validate access token
 *
 * Fetches JSON data from POST /auth/validate
 *
 * @param accessToken The access token to validate
 * @returns A promise that resolves to the raw JSON response
*/
export async function POST(accessToken: string): Promise<PostResponse> {
	try {
		// Get BASE_URL from environment with fallback
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'

		// Disable SSL verification for self-signed certificates (like curl -k)
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const validateResponse = await fetch(
			BASE_URL + "/auth/validate", {
				method: "POST",
				headers: {
					"Authorization": `Bearer ${accessToken}`
				}
			}
		)

		const responseData = await validateResponse.json()

		switch (validateResponse.status) {
			case 200:
				return {
					status: 200,
					data: {
						valid: true,
						userId: responseData.userId,
						email: responseData.email,
						emailVerified: responseData.emailVerified
					}
				}

			case 401:
				return {
					status: 401,
					data: {
						valid: false,
						error: responseData.error || 'Unauthorized - Invalid or expired token'
					}
				}

			case 405:
				return {
					status: 405,
					data: {
						error: responseData.error || 'Method Not Allowed'
					}
				}

			case 500:
				return {
					status: 500,
					data: {
						valid: false,
						error: responseData.error || 'Internal Server Error'
					}
				}

			default:
				// Unexpected status code - throw error
				throw new Error(`Unexpected HTTP status: ${validateResponse.status}`)
		} 
	} catch (error: unknown) {
		if (error instanceof Error) {
			console.log("failed to validate token")
			console.log("Error message: " + error.message)
		} else {
			console.log("an unknown error occurred")
		}

		throw error
	}
}

