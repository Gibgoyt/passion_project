interface Response200 {
	status: 200
	data: {
		success: true
		accessToken: string
		refreshToken: string
		userId: string
	}
}

interface Response401 {
	status: 401
	data: {
		success?: false
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
		error: string
	}
}

type PostResponse = Response200 | Response401 | Response405 | Response500

/*
 * refresh access token using refresh token
 *
 * Fetches JSON data from POST /auth/refresh
 *
 * @param refreshToken The refresh token to use for getting a new access token
 * @returns A promise that resolves to the response with new tokens or error
 */
export async function POST(refreshToken: string): Promise<PostResponse> {
	try {
		// Get BASE_URL from environment with fallback
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		
		console.log("@ts-testing -> BASE_URL is: " + BASE_URL)
		console.log("@ts-testing -> refreshToken is:" + refreshToken)
		// Disable SSL verification for self-signed certificates (like curl -k)
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const refreshResponse = await fetch(
			BASE_URL + "/auth/refresh", {
				method: "POST",
				headers: {
					"Content-Type": "application/json"
				},
				body: JSON.stringify({
					refreshToken: refreshToken
				})
			}
		)

		const responseData = await refreshResponse.json()

		switch (refreshResponse.status) {
			case 200:
				return {
					status: 200,
					data: {
						success: true,
						accessToken: responseData.accessToken,
						refreshToken: responseData.refreshToken,
						userId: responseData.userId
					}
				}

			case 401:
				return {
					status: 401,
					data: {
						success: false,
						error: responseData.error || 'Unauthorized - Invalid or expired refresh token'
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
						error: responseData.error || 'Internal Server Error'
					}
				}

			default:
				// Unexpected status code - throw error
				throw new Error(`Unexpected HTTP status: ${refreshResponse.status}`)
		}
	} catch (error: unknown) {
		if (error instanceof Error) {
			console.log("failed to refresh token")
			console.log("Error message: " + error.message)
		} else {
			console.log("an unknown error occurred")
		}

		throw error
	}
}
