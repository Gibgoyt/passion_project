#!/bin/bash

userId=$1
name=$2
surname=$3
bodyFile="./_api/v1/users/:userId/PUT_BODY.json"

source env.sh

sed \
	-i \
	'' \
	-e "2s/\:.*\".*\"/\:\"$name\"/g" \
	-e "3s/\:.*\".*\"/\:\"$surname\"/g" \
	$bodyFile

curl \
	-k \
	-X PUT \
	 -H "Content-Type: application/json" \
	-d @$bodyFile \
	$BASE_URL/api/v1/users/$userId \
	| jq -C
