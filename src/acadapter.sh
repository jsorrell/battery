#!/bin/bash

#tell which display to show notifications
export DISPLAY=:0
export XAUTHORITY=/home/jsorrell/.Xauthority

case $1 in
	true)
		/home/jsorrell/Dropbox/Projects/battery/bin/battery &
	;;
	false)
		#always returns true
		(killall -q -USR1 battery) || true
	;;
esac

