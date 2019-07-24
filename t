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

  if printf '%s' "$code" | "$z" 1>|./a.s && as ./a.s && ld -e _start -lSystem ./a.out
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
is 'x = function () {42};x()' 42
is 'x = y = function () {7};-x() * -x() - y()' 42
is 'x = function () {x = function () {42};x + 7};(x() - 7)()' 42
is 'square = function (x) {x = x * x};x = 7;square(x) - x' 42
is 'inc = function () {function (x) {x + 1}};(inc())(41)' 42
is 'inc = function () {f = function (x) {x + 1}};(inc())(41)' 42
is 'x = y = function () {7};f = function (a, b) {a() * a() - b()};f(x, y)' 42
is 'function f() {42};exit(f())' 42

exit 0
