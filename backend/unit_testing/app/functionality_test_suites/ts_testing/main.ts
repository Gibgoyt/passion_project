#!/usr/bin/env -S node --experimental-strip-types

import { Endpoints } from "./endpoints/index.ts"

/*
 * Basic CRUD Flow Test
 * 
 * 1. GET all users
 * 2. Create new user "Ahmed Moti"
 * 3. GET all users
 * 4. GET created user
 * 5. Update user name to "Ahmed Hashim"
 * 6. GET user again
 * 7. DELETE user
 * 8. GET all users
 */
async function test_basicFlow() {
	console.log("🚀 Starting Basic Flow Test...")
	console.log("=============================\n")

	try {
		// 1. GET all users
		console.log("1. GET /api/v1/users")
		const listResult1 = await Endpoints._Api.V1.Users.GET()
		console.log(JSON.stringify(listResult1, null, 2))
		console.log("---------------------------------------\n")

		// 2. Create User
		console.log("2. POST /api/v1/users (Create Ahmed Moti)")
		const createResult = await Endpoints._Api.V1.Users.POST("Ahmed", "Moti")
		console.log(JSON.stringify(createResult, null, 2))
		
		if (createResult.status !== 201) {
			console.error("❌ Failed to create user")
			process.exit(1)
		}
		const userId = createResult.data.id
		console.log(`👉 Created User ID: ${userId}`)
		console.log("---------------------------------------\n")

		// 3. GET all users
		console.log("3. GET /api/v1/users")
		const listResult2 = await Endpoints._Api.V1.Users.GET()
		console.log(JSON.stringify(listResult2, null, 2))
		console.log("---------------------------------------\n")

		// 4. GET single user
		console.log(`4. GET /api/v1/users/${userId}`)
		const getResult1 = await Endpoints._Api.V1.Users[':UserId'].GET(userId)
		console.log(JSON.stringify(getResult1, null, 2))
		console.log("---------------------------------------\n")

		// 5. Update user
		console.log(`5. PUT /api/v1/users/${userId} (Change name to Ahmed Hashim)`)
		const updateResult = await Endpoints._Api.V1.Users[':UserId'].PUT(userId, "Ahmed Hashim", "Moti")
		console.log(JSON.stringify(updateResult, null, 2))
		console.log("---------------------------------------\n")

		// 6. GET user again
		console.log(`6. GET /api/v1/users/${userId}`)
		const getResult2 = await Endpoints._Api.V1.Users[':UserId'].GET(userId)
		console.log(JSON.stringify(getResult2, null, 2))
		console.log("---------------------------------------\n")

		// 7. DELETE user
		console.log(`7. DELETE /api/v1/users/${userId}`)
		const deleteResult = await Endpoints._Api.V1.Users[':UserId'].DELETE(userId)
		console.log(JSON.stringify(deleteResult, null, 2))
		console.log("---------------------------------------\n")

		// 8. GET all users
		console.log("8. GET /api/v1/users")
		const listResult3 = await Endpoints._Api.V1.Users.GET()
		console.log(JSON.stringify(listResult3, null, 2))
		console.log("---------------------------------------\n")

		console.log("🎉 Basic Flow Test Completed Successfully!")

	} catch (error) {
		console.error("💥 Unexpected Error:", error)
		process.exit(1)
	}
}

// Command-line argument routing
const args = process.argv.slice(2)

if (args[0] === 'test_basicFlow') {
	test_basicFlow().catch(console.error)
} else {
	console.log("Available tests:")
	console.log("  test_basicFlow")
	console.log("\nUsage:")
	console.log("  ./main.ts <functionName>")
}
