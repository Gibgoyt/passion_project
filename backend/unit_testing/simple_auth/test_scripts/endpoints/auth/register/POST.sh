#!/bin/bash

email=$1
password=$2
bodyFile="/home/opc/splitdo/new/unit_testing/simple_auth/test_scripts/endpoints/auth/register/POST_BODY.json"

echo "editing file"
# Use jq to properly handle JSON with special characters
jq --arg email "$email" --arg password "$password" \
   '.email = $email | .password = $password' \
   "$bodyFile" > "${bodyFile}.tmp" && mv "${bodyFile}.tmp" "$bodyFile"
echo ""

source /home/opc/splitdo/new/unit_testing/simple_auth/test_scripts/endpoints/env.sh

curl \
	-k -X POST \
	-H "Content-Type: application/json" \
	-d @$bodyFile \
	"$BASE_URL/auth/register" \
	| jq -C
