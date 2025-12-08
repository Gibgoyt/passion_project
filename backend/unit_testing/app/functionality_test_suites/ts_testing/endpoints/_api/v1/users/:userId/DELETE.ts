interface Response200 {
	status: 200
	data: {
		success: boolean
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

type DeleteResponse = Response200 | Response404 | Response500

/*
 * Delete a user
 *
 * Fetches JSON data from DELETE /api/v1/users/:userId
 *
 * @param userId The ID of the user to delete
 * @returns A promise that resolves to the expected data type
 */
export async function DELETE(userId: string): Promise<DeleteResponse> {
	try {
		const BASE_URL = process.env.BASE_URL || 'https://localhost:2053'
		process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = "0"

		const response = await fetch(
			`${BASE_URL}/api/v1/users/${userId}`, {
				method: "DELETE"
			}
		)

		const responseData = await response.json()

		switch (response.status) {
			case 200:
				return {
					status: 200,
					data: {
						success: responseData.success
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
			console.log("failed to delete user")
			console.log("Error message: " + error.message)
		}
		throw error
	}
}
