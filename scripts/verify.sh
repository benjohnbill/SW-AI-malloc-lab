#!/usr/bin/env bash
# make verify 의 본체. 13개 trace 가 전부 valid 인지 판정한다.
#
# mdriver 는 오류가 있어도 종료 코드가 0 이라서 종료 코드로는 판정할 수 없다.
# 대신 -g 가 찍는 "correct:N" 줄을 읽는다. trace 하나만 돌리므로 N 은 0 또는 1 이다.
# (mdriver.c 에는 디버그 출력 "getopt returned: …" 가 섞여 있어서, 줄 전체 일치로만 읽는다.)
#
# 사용법:
#   MDRIVER=<mdriver 의 절대 경로> scripts/verify.sh [이름 ...]
#   이름을 주지 않으면 13개 전부. 이름은 traces/<이름>-bal.rep 의 <이름>.
#
# 전부 valid 일 때만 종료 코드 0. Perf index 는 보고만 하고 합격 조건에 넣지 않는다.
set -u

: "${MDRIVER:?MDRIVER 에 mdriver 의 절대 경로를 주세요}"

ALL=(short1 short2 amptjp cccp cp-decl expr coalescing random random2 binary binary2 realloc realloc2)
if [ $# -gt 0 ]; then traces=("$@"); else traces=("${ALL[@]}"); fi

# trace 경로(traces/…)가 cwd 기준이라서 malloc-lab/ 에서 실행한다.
cd "$(dirname "$0")/../malloc-lab" || exit 2

ok=0
bad=0
for t in "${traces[@]}"; do
  out=$(timeout 60 "$MDRIVER" -g -f "traces/$t-bal.rep" 2>&1)
  rc=$?
  if grep -qx 'correct:1' <<<"$out"; then
    printf '  ✅ %-12s valid yes\n' "$t"
    ok=$((ok + 1))
  else
    why=$(grep -m1 -E '^(ERROR|Terminated)' <<<"$out")
    [ -n "$why" ] || why="correct 줄 없음 (종료 코드 $rc)"
    printf '  ❌ %-12s %s\n' "$t" "$why"
    bad=$((bad + 1))
  fi
done

# 전체 검증일 때만 Perf index 를 보고한다. 기본 11개 trace 기준이다.
if [ $# -eq 0 ]; then
  perf=$(timeout 120 "$MDRIVER" -v 2>&1 | grep -E '^(Perf index|Terminated with)')
  echo
  echo "  Perf index (보고만): ${perf:-출력 없음}"
fi

echo
total=$((ok + bad))
if [ "$bad" -eq 0 ]; then
  echo "  ✅ $ok/$total valid"
  exit 0
fi
echo "  ❌ $ok/$total valid"
exit 1
