#!/bin/bash

userId=$1

source env.sh

curl \
	-k \
	-X GET \
	$BASE_URL/api/v1/users/$userId \
	| jq -C
