#!/bin/bash
set -xe

local_ip=$(ip addr | awk '/^[0-9]+: / {}; /inet.*global/ {print gensub(/(.*)\/(.*)/, "\\1", "g", $2)}' | head -n 1)
echo $local_ip
curl -X POST -H "Content-Type: application/json" -d @model_data.json http://$local_ip:10048/load_and_register
echo -e "check model status on http://$local_ip:10048/get_service_model_info"
