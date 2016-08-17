#!/bin/sh

#set the udp rss.
ETHLIST=`ls /sys/class/net/ | grep eth`;

for i in ${ETHLIST};
do
	DRV=`ethtool -i ${i} | grep driver | awk '{print $2}'`
	if [ "${DRV}" == "ixgbe" ]
	then
		ethtool -N ${i} rx-flow-hash udp4 sdfn;
		/sbin/irq_affinity ${i}
	fi
done
