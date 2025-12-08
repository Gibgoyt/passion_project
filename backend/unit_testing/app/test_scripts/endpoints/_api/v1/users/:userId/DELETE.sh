#!/bin/bash

userId=$1

source env.sh

curl \
	-k \
	-X DELETE \
	$BASE_URL/api/v1/users/$userId \
	| jq -C
