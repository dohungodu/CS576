#!/bin/sh
for i in ` ipcs -m |   awk '{print $2}'  `
do 
  ipcrm -m $i 2> /dev/null
done
