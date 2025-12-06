#!/bin/bash

accessToken=$1

source /home/opc/splitdo/new/unit_testing/simple_auth/test_scripts/endpoints/env.sh

curl \
	-k \
	-X GET \
	-H "Authorization: Bearer $accessToken" \
	$BASE_URL/auth/validate \
	| jq -C
