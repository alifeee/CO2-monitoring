#!/bin/bash

# co2
tail -n1 env.log \
  | awk -F'\t' '{printf "%s ppm.", $2}' \
  | figlet -t -c -f big
echo ""
tail -n1 env.log \
  | awk -F'\t|\n' '{printf "%s C - %s %%", $4, $6}' \
  | tr -d '\r\n' \
  | figlet -t -c -f standard
