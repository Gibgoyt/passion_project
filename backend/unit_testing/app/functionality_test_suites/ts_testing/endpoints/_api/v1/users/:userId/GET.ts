interface User {
	id: string
	name: string
	surname: string
	createdAt: string
	updatedAt: string
}

interface Response200 {
	status: 200
	data: User
}

interface Response404 {
	status: 404
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

type GetResponse = Response200 | Response404 | Response500

/*
 * Get a single user
 *
 * Fetches JSON data from GET /api/v1/users/:userId
 *
 * @param userId The ID of the user to retrieve
 * @returns A promise that resolves to the expected data type
 */
export async function GET(userId: string): Promise<GetResponse> {
	try {
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const response = await fetch(
			`${BASE_URL}/api/v1/users/${userId}`, {
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

			case 404:
				return {
					status: 404,
					data: {
						error: responseData.error || 'Not Found'
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
				throw new Error(`Unexpected HTTP status: ${response.status}`)
		}
	} catch (error: unknown) {
		if (error instanceof Error) {
			console.log("failed to get user")
			console.log("Error message: " + error.message)
		}
		throw error
	}
}
