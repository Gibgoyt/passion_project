interface User {
	id: string
	name: string
	surname: string
	createdAt: string
	updatedAt: string
}

interface Response200 {
	status: 200
	data: User[]
}

interface Response500 {
	status: 500
	data: {
		error: string
	}
}

type GetResponse = Response200 | Response500

/*
 * Get all users
 *
 * Fetches JSON data from GET /api/v1/users
 *
 * @returns A promise that resolves to the expected data type
 */
export async function GET(): Promise<GetResponse> {
	try {
		// Get BASE_URL from environment with fallback
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'

		// Disable SSL verification for self-signed certificates
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const response = await fetch(
			BASE_URL + "/api/v1/users", {
				method: "GET"
			}
		)

		const responseData = await response.json()

		switch (response.status) {
			case 200:
				return {
					status: 200,
					data: responseData
				}

			case 500:
				return {
					status: 500,
					data: {
						error: responseData.error || 'Internal Server Error'
					}
				}

			default:
				throw new Error(`Unexpected HTTP status: ${response.status}`)
		}
	} catch (error: unknown) {
		if (error instanceof Error) {
			console.log("failed to fetch users")
			console.log("Error message: " + error.message)
		} else {
			console.log("an unknown error occurred")
		}

		throw error
	}
}
