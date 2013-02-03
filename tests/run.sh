#!/bin/sh

cd $(dirname $0)/..

report_incorrect() {
  incorrect=$((incorrect + 1))
  tput setaf 1; tput bold
  echo -n '✘ '
  tput sgr0; tput setaf 1
  echo $test
  tput sgr0
  [ "$error" ] && echo "  $error"
}

report_correct() {
  correct=$((correct + 1))
  echo -n '✓ '
  tput setaf 2;
  echo $test
  tput sgr0
  [ "$error" ] && echo "  $error"
}

incorrect=0
correct=0
for test in $(find tests/good -name '*.simple'); do
  error=$(./simple < $test 2>&1)
  [ "$error" ] && report_incorrect || report_correct
done

for test in $(find tests/bad -name '*.simple'); do
  error=$(./simple < $test 2>&1)
  [ "$error" ] && report_correct || report_incorrect
done

echo
if [ $incorrect -gt 0 ]; then
  tput setaf 1; tput bold
  echo -n '✘ '
  tput sgr0; tput setaf 1
  echo -n 'FAIL'
  tput sgr0
else
  echo -n '✓ '
  tput setaf 1
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
