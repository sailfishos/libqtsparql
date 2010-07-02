#!/bin/sh
set -e

case $# in
0)
lim=100 ;;
*)
lim=$1 ;;
esac

i=1
while [ $i -le $lim ]
do
cat <<EOF
insert { _:c a nco:PersonContact ;
nco:nameGiven 'Name$i' ;
nco:nameFamily 'Family$i' .
'tel:$i' a nco:PhoneNumber ;
nco:phoneNumber '$i' .
_:c nco:hasPhoneNumber 'tel:$i' . }
EOF
i=$(($i+1))
done

