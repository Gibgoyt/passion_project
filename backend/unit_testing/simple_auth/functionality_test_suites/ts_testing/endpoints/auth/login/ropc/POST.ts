interface Response200 {
	status: 200
	data: {
		success: true
		accessToken: string
		refreshToken: string
		userId: string
	}
}

interface Response400 {
	status: 400
	data: {
		error: string
	}
}

interface Response401 {
	status: 401
	data: {
		success: false
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

type PostResponse = Response200 | Response400 | Response401 | Response405 | Response500

/* 
 * first login to the system
 *
 * Fetches JSON data from POST /auth/login/ropc
 *
 * @param email The email used to login
 * @param password The password used to login
 * @returns A promise that resolves to the expected data type
*/
export async function POST(email: string, password: string): Promise<PostResponse> {
	// current mock data
	const loginRequestBody = {
		"email": email,
		"password": password
	}

	try {
		// Get BASE_URL from environment with fallback
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'

		// Disable SSL verification for self-signed certificates (like curl -k)
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const loginResponse = await fetch(
			BASE_URL + "/auth/login/ropc", {
				method: "POST",
				headers: {
					"Content-Type": "application/json"
				},
				body: JSON.stringify(loginRequestBody)
			}
		)

		const responseData = await loginResponse.json()

		switch (loginResponse.status) {
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

			case 400:
				return {
					status: 400,
					data: {
						error: responseData.error || 'Bad Request'
					}
				}

			case 401:
				return {
					status: 401,
					data: {
						success: false,
						error: responseData.error || 'Unauthorized'
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
				throw new Error(`Unexpected HTTP status: ${loginResponse.status}`)
		}
	} catch (error: unknown) {
		if (error instanceof Error) {
			console.log("failed to fetch data")
			console.log("Error message: " + error.message)
		} else {
			console.log("an unknown error occurred")
		}

		throw error
	}
}
