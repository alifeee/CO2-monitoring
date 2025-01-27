#!/bin/bash

tail -n100 env.log | awk -F'\t' '{print $2}' | eplot -d -t "CO2" 2> /dev/null
