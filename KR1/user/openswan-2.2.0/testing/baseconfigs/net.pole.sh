#
# $Id: net.pole.sh,v 1.1 2004/12/24 07:17:33 rupert Exp $
#
if [ -n "$UML_north_CTL" ]
then
    net_eth0="eth0=daemon,10:00:00:de:cd:01,unix,$UML_east_CTL,$UML_east_DATA";
else
    net_eth0="eth0=mcast,10:00:00:de:cd:49,239.192.0.3,40810";
fi

net="$net_eth0"



