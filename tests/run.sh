#!/usr/bin/env bash

cd $(dirname $0)
SIMPLE="../simple $@"

indent() {
  c='s/^/  /'
  case $(uname) in
    Darwin) sed -l "$c";;
    *)      sed -u "$c";;
  esac
}

report_incorrect() {
  incorrect=$((incorrect + 1))
  tput setaf 1; tput bold
  echo -n '✘ '
  tput sgr0; tput setaf 1
  echo $test
  tput sgr0
  [ "$error" ] && echo "$error" | indent
}

report_correct() {
  correct=$((correct + 1))
  echo -n '✓ '
  tput setaf 2;
  echo $test
  tput sgr0
  [ "$error" ] && echo "$error" | indent
}

section() {
  echo
  tput setaf 8
  echo "$@"
  tput sgr0
}

incorrect=0
correct=0

section "Parser accepts valid programs"
for test in $(find good -name '*.simple' | sort); do
  error=$($SIMPLE < $test 2>&1)
  [ "$error" ] && report_incorrect || report_correct
done > >(indent)

section "Parser rejects invalid programs"
for test in $(find bad -name '*.simple' | sort); do
  error=$($SIMPLE < $test 2>&1)
  [ "$error" ] && report_correct || report_incorrect
done > >(indent)

echo
if [ $incorrect -gt 0 ]; then
  tput setaf 1; tput bold
  echo -n '✘ '
  tput sgr0; tput setaf 1
  echo -n 'FAIL'
  tput sgr0
else
  echo -n '✓ '
  tput setaf 2
  echo -n 'OK'
  tput sgr0
fi

echo -n ' » '
if [ $correct -gt 0 ]; then
  tput bold
  echo -n $correct
  tput sgr0
  echo -n ' correct '
fi
if [ $incorrect -gt 0 ]; then
  tput bold
  echo -n $incorrect
  tput sgr0
  echo -n ' incorrect '
fi
echo
