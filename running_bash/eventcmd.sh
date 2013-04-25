#!/bin/bash

# create variables
while read L; do
k="`echo "$L" | cut -d '=' -f 1`"
v="`echo "$L" | cut -d '=' -f 2`"
export "$k=$v"
done < <(grep -e '^\(title\|artist\|album\|stationName\|songStationName\|pRet\|pRetStr\|wRet\|wRetStr\|songDuration\|songPlayed\|rating\|coverArt\|stationCount\|station[0-9]*\)=' /dev/stdin) # don't overwrite $1...

case "$1" in
songstart)
rm /home/pi/pianobar_IR/running_bash/out
#cat > /home/pi/.config/pianobar/script/out
echo -e "$title\n$artist" >> /home/pi/pianobar_IR/running_bash/out
sudo /home/pi/pianobar_IR/running_bash/writelcd
#source /home/pi/pianobar_IR/running_bash/test.sh
#sleep 2
esac
