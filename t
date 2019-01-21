#! /bin/bash
#
# @(#) t - z test
#
set -o errexit
set -o nounset
set -o noclobber

printf "1..1\n"

if printf '' | ./z
then
  printf "ok 1\n"
else
  printf "not ok 1\n"
fi

exit 0
