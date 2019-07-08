#! /bin/bash
#
# @(#) t - z test
#
set -o errexit
set -o nounset
set -o noclobber

topdir="$(cd -- $(dirname -- "$0") ; pwd)"
t="$topdir/$(basename -- "$0")"
z="$topdir/z"

build="$(mktemp -d)"
trap 'cd -- "$topdir" ; rm -rf -- "$build"' EXIT
cd -- "$build"

diag () {
  printf "# %s\n" "$@"
} 1>&2

declare -i number=0
is () {
  local -r code="$1"
  local -i expected="$2"
  local -i got="$expected"
  local error='not '

  number+=1

  if printf '%s' "$code" | "$z" 1>|./a.s && cc -e _start ./a.s
  then
    ./a.out && :
    got=$?
    if ((got == expected))
    then
      error=''
    fi
  fi

  printf "%sok %d - %s\n" "$error" "$number" "'${code//$'\n'/\\n}'"
  if [[ $error ]]
  then
    diag "  Failed test at number $number."
    if ((got != expected))
    then
      diag "     got: $got"
      diag "expected: $expected"
    fi
  fi
}

printf "1..%d\n" "$(grep --count '^is[ ][^\(]' -- "$t")"

is '' 0
is '0' 0
is '42' 42
is '2 + 8 + 32' 42
is '49 - 7' 42
is '6 * 7' 42
is '252 / 6' 42
is '-6 * -7' 42
is '-(7 - 1) * -7' 42
is '0;42' 42
is '0;42;' 42
is 'x = 42' 42
is 'x = y = 42' 42
is 'x = y = 42;y = 0;x' 42
is 'x = y = 42;y = 0;y' 0

exit 0
