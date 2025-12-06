#!/bin/bash

refreshToken=$1
bodyFile="/home/opc/splitdo/new/unit_testing/simple_auth/test_scripts/endpoints/auth/refresh/POST_BODY.json"

source /home/opc/splitdo/new/unit_testing/simple_auth/test_scripts/endpoints/env.sh

sed \
	-i \
	"2s/\:.*\".*\"/\:\"$refreshToken\"/g" \
	$bodyFile

curl \
	-k \
	-X POST \
	-d @$bodyFile \
	$BASE_URL/auth/refresh \
	| jq -C
