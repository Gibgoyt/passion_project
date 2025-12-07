#!/bin/bash

expiredRefreshToken=$1
bodyFile="/Users/ahmed/Projects/Astro/passion_project/backend/unit_testing/simple_auth/test_scripts/endpoints/auth/recover_session/POST_BODY.json"

source /Users/ahmed/Projects/Astro/passion_project/backend/unit_testing/simple_auth/test_scripts/endpoints/env.sh

sed \
	-i \
	"2s/\:.*\".*\"/\:\"$expiredRefreshToken\"/g" \
	$bodyFile

curl \
	-k \
	-X POST \
	-d @$bodyFile \
	$BASE_URL/auth/recover-session \
	| jq -C
