# ─────────────────────────────────────────────────────────────
#  week06 로컬 편의 계층
#
#  GNU make 는 Makefile 보다 GNUmakefile 을 먼저 읽는다. 이 저장소는 루트에
#  upstream Makefile 이 없고 malloc-lab/ 아래에 있다. 그래서 5주차와 달리
#  include 하지 않고, 그 Makefile 을 -C 로 불러서 쓴다.
#    1. 호스트  : make -C malloc-lab 으로 빌드한다. 산출물은 malloc-lab/ 에 생긴다.
#    2. 컨테이너: 소스를 build-docker/ 로 복사해 거기서 빌드한다. upstream 은
#                 in-tree 빌드라서, 같은 폴더에 두 환경이 쓰면 .o 가 섞인다.
#  upstream 파일은 손대지 않는다. git pull upstream master 가 충돌 없이 지나가도록.
#
#  아래 TR 은 손으로 적어 둔 목록이다. wildcard 로 계산하면 zsh 탭 완성이
#  이름을 읽지 못한다. 이름은 traces/<이름>-bal.rep 의 <이름> 이다.
#
#  주의: mdriver 는 오류가 있어도 종료 코드가 0 이다. 합격 판정은 make verify 가 한다.
# ─────────────────────────────────────────────────────────────

TR   := short1 short2 amptjp cccp cp-decl expr coalescing random random2 binary binary2 realloc realloc2
RTR  := $(addprefix r_,$(TR))
GTR  := $(addprefix g_,$(TR))
DRTR := $(addprefix dr_,$(TR))
DGTR := $(addprefix dg_,$(TR))

GDB ?= gdb

# 컨테이너 안에서는 IN_DOCKER=1 이다 (아래 DOCKER 변수가 넣어 준다).
# gdb 의 rebuild/rerun 은 실행 파일 위쪽에서 makefile 을 찾아 `make mdriver` 를 부른다.
# 컨테이너의 실행 파일은 build-docker/ 에 있으므로 이 파일이 잡히고, 아래 컨테이너용
# mdriver 규칙이 복사와 빌드를 다시 한다.
ifdef IN_DOCKER
MDRIVER := build-docker/mdriver
# malloc-lab/ 으로 cd 한 뒤의 경로. trace 가 ./traces/ 기준이라서 그 폴더에서 실행한다.
MDRIVER_FROM_LAB := ../build-docker/mdriver

mdriver:
	@mkdir -p build-docker
	@cp -pu malloc-lab/*.c malloc-lab/*.h build-docker/
	@cp -pu malloc-lab/Makefile build-docker/upstream.mk
	@$(MAKE) --no-print-directory -C build-docker -f upstream.mk mdriver
else
MDRIVER := malloc-lab/mdriver
MDRIVER_FROM_LAB := ./mdriver

mdriver:
	@$(MAKE) --no-print-directory -C malloc-lab mdriver
endif

# ─── 호스트 ───
#   make r_short1      mdriver -V -f traces/short1-bal.rep
#   make g_short1      같은 인자로 gdb
#   make score         기본 11개 trace 전체 → Perf index 줄 (-v)
#   make verify        13개 trace 전부 valid yes 인지 판정 (종료 코드 0 / 1)
$(RTR): r_%: mdriver
	@cd malloc-lab && $(MDRIVER_FROM_LAB) -V -f traces/$*-bal.rep || echo "   (종료 코드 $$?)"

$(GTR): g_%: mdriver
	cd malloc-lab && $(GDB) --args $(MDRIVER_FROM_LAB) -V -f traces/$*-bal.rep

score: mdriver
	@cd malloc-lab && $(MDRIVER_FROM_LAB) -v || echo "   (종료 코드 $$?)"

verify: mdriver
	@MDRIVER=$(abspath $(MDRIVER)) bash scripts/verify.sh

clean:
	$(MAKE) -C malloc-lab clean

# ─── 컨테이너 (upstream .devcontainer/Dockerfile 로 만든 이미지) ───
# --user       : 컨테이너가 쓴 파일이 호스트에서 root 소유로 남지 않게
# IN_DOCKER    : 위의 mdriver 규칙이 컨테이너용으로 바뀐다
# HOME=/tmp    : uid 1000 이 컨테이너 passwd 의 jungle 과 겹쳐도 HOME 을 고정한다
# ~/.gdbinit(ro), ~/.config/gdb(rw: 히스토리가 그 안에 있다) : 호스트 gdb 설정을 그 HOME 에
#                붙인다. 복사가 아니라 같은 파일이다.
# DOCKER_TTY   : 터미널이 아닌 곳(스크립트)에서 부를 때 make dcheck DOCKER_TTY=-i
# -e GDB       : make dg_… GDB='gdb -batch …' 처럼 준 gdb 명령을 컨테이너 안으로 넘긴다
DOCKER_TTY ?= -it
DOCKER_IMG ?= mallocdbg
DOCKER := docker run --rm $(DOCKER_TTY) -e IN_DOCKER=1 -e GDB \
  --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
  --user $(shell id -u):$(shell id -g) -e HOME=/tmp \
  -v "$(HOME)/.gdbinit":/tmp/.gdbinit:ro -v "$(HOME)/.config/gdb":/tmp/.config/gdb \
  -v "$(CURDIR)":/work -w /work \
  $(DOCKER_IMG)

dimage:                      ## 컨테이너 이미지 빌드 (최초 1회, 1~3분)
	docker build -t $(DOCKER_IMG) -f .devcontainer/Dockerfile .devcontainer

dcheck:                      ## 컨테이너에서 make verify (검증 기준)
	$(DOCKER) make verify

dscore:                      ## 컨테이너에서 make score
	$(DOCKER) make score

dshell:                      ## 컨테이너 셸
	$(DOCKER) bash

$(DRTR): dr_%:
	$(DOCKER) make r_$*

$(DGTR): dg_%:
	$(DOCKER) make g_$*

dclean:
	rm -rf build-docker

.PHONY: mdriver $(RTR) $(GTR) $(DRTR) $(DGTR) score verify clean dimage dcheck dscore dshell dclean help

.DEFAULT_GOAL := help
help:
	@echo "malloc lab 로컬 편의 (GNUmakefile)"; \
	echo "  make r_<이름> | g_<이름>      호스트: trace 하나 실행 | gdb (탭 완성 됨)"; \
	echo "  make score                   기본 11개 trace 전체 + Perf index"; \
	echo "  make verify                  13개 trace 전부 valid yes 인지 판정 (종료 코드 0 / 1)"; \
	echo "  make dcheck | dscore         컨테이너: verify | score"; \
	echo "  make dr_<이름> | dg_<이름>    컨테이너: 실행 | gdb"; \
	echo "  make dshell | dimage | dclean   컨테이너 셸 | 이미지 빌드 | build-docker 정리"; \
	echo "  make clean                   호스트 산출물 정리 (malloc-lab/)"; \
	echo; \
	echo "  이름: $(TR)"
