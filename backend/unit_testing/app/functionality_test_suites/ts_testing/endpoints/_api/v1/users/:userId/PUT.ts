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

interface Response400 {
	status: 400
	data: {
		error: string
	}
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

type PutResponse = Response200 | Response400 | Response404 | Response500

/*
 * Update a user
 *
 * Fetches JSON data from PUT /api/v1/users/:userId
 *
 * @param userId The ID of the user to update
 * @param name The new name
 * @param surname The new surname
 * @returns A promise that resolves to the expected data type
 */
export async function PUT(userId: string, name: string, surname: string): Promise<PutResponse> {
	const requestBody = {
		"name": name,
		"surname": surname
	}

	try {
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const response = await fetch(
			`${BASE_URL}/api/v1/users/${userId}`, {
				method: "PUT",
				headers: {
					"Content-Type": "application/json"
				},
				body: JSON.stringify(requestBody)
			}
		)

		const responseData = await response.json()

		switch (response.status) {
			case 200:
				return {
					status: 200,
					data: responseData
				}

			case 400:
				return {
					status: 400,
					data: {
						error: responseData.error || 'Bad Request'
					}
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
			console.log("failed to update user")
			console.log("Error message: " + error.message)
		}
		throw error
	}
}
