#!/bin/sh
source ./env.sh
image='d45637a0-7d10-4da8-977c-aad96a8c60ba'
flavor='npe-test-007'
vpc_vnet='5dc128af-99e3-477e-b32e-5aae2ac7fa7b'
vpc_sg='32366547-1759-4f30-be7e-68f4a32c5d6a'
# If you need to use specific ip, please add replace ${vpc_ip} with \"${vpc_ip}\" in the curl command
vpc_ip=null
storage_location='ssd'
vm_name=$1
az='ksc_tjwq_zone1001_aggregate_raidssd_1001:tjwq01-cp-compute008200.tjwq01.ksyun.com'
password='123'
is_vpc_instance=1
#create vpc vm
curl -i http://${nova_ip}:8774/v2/${tenant_id}/servers -X POST -H "X-Auth-Project-Id: admin" -H "User-Agent: python-novaclient" -H "Content-Type: application/json" -H "Accept: application/json" -H "X-Auth-Token: ${token}" -d "{\"server\": {\"min_count\": 1, \"flavorRef\": \"${flavor}\", \"name\": \"${vm_name}\", \"imageRef\": \"${image}\", \"max_count\": 1, \"is_vpc_instance\": ${is_vpc_instance}, \"vpc_vnet_id\": \"${vpc_vnet}\", \"vpc_security_group\": \"${vpc_sg}\", \"availability_zone\":\"${az}\", \"metadata\":{\"storage_location\":\"${storage_location}\"}, \"adminPass\":\"${password}\",\"vpc_dhcp_domain\":\"kingsoft.com\", \"vpc_ip\":${vpc_ip}}}"
