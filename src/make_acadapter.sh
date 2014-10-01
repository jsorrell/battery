DIR="$( cd "$( dirname "$0" )" && pwd )"
parentdir="$(dirname "$DIR")"
script="#!/bin/bash\n
\n
#tell which display to show notifications\n
export DISPLAY=:0\n
export XAUTHORITY=/home/$USER/.Xauthority\n
\n
case \$1 in\n
\ttrue)\n
\t\t$parentdir/bin/battery &\n
\t;;\n
\tfalse)\n
\t\t#always returns true\n
\t\t(killall -q -USR1 battery) || true\n
\t;;\n
esac"
echo $script > "$parentdir/bin/acadapter.sh"