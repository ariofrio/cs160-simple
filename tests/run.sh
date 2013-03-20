#!/usr/bin/env bash

SIMPLE="./simple $@"

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
  return 0
}

report_skipped() {
  skipped=$((skipped + 1))
  tput setaf 3; tput bold
  echo -n '- '
  tput sgr0; tput setaf 3
  echo $test
  tput sgr0
  [ "$error" ] && echo "$error" | indent
  return 0
}

report_correct() {
  correct=$((correct + 1))
  echo -n '✓ '
  tput setaf 2;
  echo $test
  tput sgr0
  [ "$error" ] && echo "$error" | indent
  return 0
}

section() {
  echo
  tput setaf 8
  echo "$@"
  tput sgr0
}

incorrect=0
correct=0
skipped=0

section "Parser rejects invalid programs"
for test in $(find tests/bad -name '*.simple' | sort); do
  error=$(bash -c "$SIMPLE < $test" 2>&1 1>/dev/null)
  test="$SIMPLE < $test"
  [ "$error" ] && report_correct || report_incorrect
done > >(indent)

section "Parser accepts valid programs"
for test in $(find tests/good -name '*.simple' | sort); do
  error=$(bash -c "$SIMPLE < $test" 2>&1 1>/dev/null)
  test="$SIMPLE < $test"
  [ "$error" ] && report_incorrect || report_correct
done > >(indent)

function simplec() {
  ./simple < $test > $sfile &&
  gcc -m32 -c -o $ofile $sfile &&
  gcc -m32 -c -o tests/good/start.o tests/good/start.c &&
  gcc -m32 -o $runfile tests/good/start.o $ofile &&
  bash -c "$runfile"
}

section "Valid programs compile and run correctly"
for test in $(find tests/good -name '*.simple' | sort); do
  runfile=${test%.simple}.run
  ofile=${test%.simple}.o
  sfile=${test%.simple}.s
  outfile=${test%.simple}.out
  if [ -f $outfile ]; then
    tempfile=$(mktemp)
    error=$(simplec 2>&1 > $tempfile)
    test="$SIMPLE < $test > $sfile"
    if [ "$error" ]; then
      report_incorrect
    else
      diff $tempfile $outfile &> /dev/null && report_correct || report_incorrect
      colordiff $tempfile $outfile | indent
    fi
    rm $tempfile
  fi
done > >(indent)

echo
if [ $incorrect -gt 0 ] || [ $skipped -gt 0 ]; then
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
if [ $skipped -gt 0 ]; then
  tput bold
  echo -n $skipped
  tput sgr0
  echo -n ' skipped '
fi
echo
