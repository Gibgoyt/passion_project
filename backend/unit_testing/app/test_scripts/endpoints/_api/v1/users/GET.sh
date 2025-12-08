#!/bin/bash

source env.sh

curl \
	-k \
	-X GET \
	$BASE_URL/api/v1/users \
	| jq -C
