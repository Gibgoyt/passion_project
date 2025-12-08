#!/bin/bash

name=$1
surname=$2
bodyFile="./_api/v1/users/POST_BODY.json"

source env.sh

sed \
	-i \
	'' \
	-e "2s/\:.*\".*\"/\:\"$name\"/g" \
	-e "3s/\:.*\".*\"/\:\"$surname\"/g" \
	$bodyFile

curl \
	-k \
	-X POST \
	-H "Content-Type: application/json" \
	-d @$bodyFile \
	$BASE_URL/api/v1/users \
	| jq -C
