#!/bin/bash

accessToken=$1

source /Users/ahmed/Projects/Astro/passion_project/backend/unit_testing/simple_auth/test_scripts/endpoints/env.sh

curl \
	-k \
	-X GET \
	-H "Authorization: Bearer $accessToken" \
	$BASE_URL/auth/validate \
	| jq -C
