interface User {
	id: string
	name: string
	surname: string
	createdAt: string
	updatedAt: string
}

interface Response201 {
	status: 201
	data: User
}

interface Response400 {
	status: 400
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

type PostResponse = Response201 | Response400 | Response500

/*
 * Create a new user
 *
 * Fetches JSON data from POST /api/v1/users
 *
 * @param name The name of the user
 * @param surname The surname of the user
 * @returns A promise that resolves to the expected data type
 */
export async function POST(name: string, surname: string): Promise<PostResponse> {
	const requestBody = {
		"name": name,
		"surname": surname
	}

	try {
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const response = await fetch(
			BASE_URL + "/api/v1/users", {
				method: "POST",
				headers: {
					"Content-Type": "application/json"
				},
				body: JSON.stringify(requestBody)
			}
		)

		const responseData = await response.json()

		switch (response.status) {
			case 201:
				return {
					status: 201,
					data: responseData
				}

			case 400:
				return {
					status: 400,
					data: {
						error: responseData.error || 'Bad Request'
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
			console.log("failed to create user")
			console.log("Error message: " + error.message)
		}
		throw error
	}
}
