#!/bin/sh

#nova delete $vm-uuid on the nova controller node

#UUID='aaa7b51d-9072-463c-96db-58661e9572a7'
#UUID='0ded4d08-0e39-4664-b288-89827a57c7a4'
#UUID='5ca7df8e-1b8a-4328-9552-ea832610cdb5'
UUID='f1576164-4807-4946-a26b-122f801e8006'

CONTROLLER_IP='10.64.2.193'

curl -i -X POST -H "Content-Type: application/json" -H "X-Auth-Token:191532849a034f2cb555d8d8fa08027a:49e049ab04f846a48e5f0fd152a729fc" -d '{"forceDelete":null}' http://$CONTROLLER_IP:8774/v2/49e049ab04f846a48e5f0fd152a729fc/servers/$UUID/action
