#!/bin/bash

email=$1
password=$2
bodyFile="/home/opc/splitdo/new/unit_testing/simple_auth/test_scripts/endpoints/auth/login/POST_BODY.json"
# there has to be a better way to say ./POST_BODY.json next to the current ./POST.sh

sed \
	-i \
	-e "2s/\:\".*\"/\:\"$email\"/g" \
	-e "3s/\:\".*\"/\:\"$password\"/g" \
	$bodyFile

source /home/opc/splitdo/new/unit_testing/simple_auth/test_scripts/endpoints/env.sh

echo "cat"
cat $bodyFile
curl \
	-k -X POST \
	-H "Content-Type:application/json" \
	-d @$bodyFile \
	"$BASE_URL/auth/login/ropc" \
	| jq -C
