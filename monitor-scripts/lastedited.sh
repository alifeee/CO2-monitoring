#!/bin/bash

mod=$(date --date="$(stat env.log | grep Modify | awk -F' ' '{print $3}' | sed 's/\..*//')" "+%s")
now=$(date +%s)

echo "last data: $(( $now - $mod )) seconds ago"
