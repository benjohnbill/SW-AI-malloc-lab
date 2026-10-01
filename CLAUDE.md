# CLAUDE.md — SW-AI-malloc-lab (week 6 malloc lab)

The tutor spine is imported by the parent `CLAUDE.md`; this file fills its
slots. Solo week: no `Mode:` line.

## Purpose

One file, `malloc-lab/mm.c`, holds a dynamic memory allocator: `mm_init`,
`mm_malloc`, `mm_free`, `mm_realloc`. The `mdriver` program grades it on 13
traces (valid or not) and a performance index (60% space utilization, 40%
throughput). Implicit free list first; explicit list, segregated lists and
buddy system are optional. This is the `malloc` he only used last week. What
must remain after it: the ability to draw the heap as blocks, to say what state
the allocator is in between any two calls, and to name which cost (space or
time) a policy buys. The Tuesday quiz covers CS:APP chapter 9, open book, so the
concept questions below double as quiz practice.

## Slot 1 — step 2 and the ladder

Step 2 is **draw the heap as a row of blocks**: for each block, its address,
size and allocated bit, and its list links if the design has a free list. Draw
the row before and after each `mm_malloc` and `mm_free`; for `mm_free`, draw the
neighbor cases (which side is free). For `mm_realloc`: which block the old
pointer names after the call, and whether the payload moved.

The other steps in this week's terms:

1. Predict before running: the trace names say what each one stresses
   (`coalescing`, `realloc`, `binary`). Say which trace he expects to fail first
   and why.
2. Draw the heap (above).
3. Candidate strategies: fit policy (first, next, best), when to coalesce (at
   free or deferred), list structure (implicit, explicit, segregated), where to
   split a block. A strategy is a choice among these; the book's code is open
   material (slot 2).
4. Evidence: trace `short1-bal.rep` (12 operations) on paper first. Then a heap
   checker he writes himself, a function that walks the block row and asserts
   the invariants, called from `mm_malloc` and `mm_free`. Then `x/` and `watch`
   in gdb (`make g_<name>`).
5. Verify: `make verify`. Then: did the change remove the cause or only the
   symptom (a larger initial heap extension that lets one trace pass)?

After it passes, one at a time, in order:

1. The invariant the block row keeps between calls, in one sentence, and which
   call restores it after it is broken.
2. Where does his fit policy degrade? Build a malloc/free sequence with him
   that makes it waste space or time.
3. Compare with the book's code and with `mdriver`'s per-trace utilization table
   (`make score`). Let him predict the trace with the lowest utilization before
   he looks.

## Slot 2 — statement and spoilers

The statement is CS:APP section 9.9, the CMU writeup
(`http://csapp.cs.cmu.edu/3e/malloclab.pdf`), the header comments in `mm.c`, and
`./mdriver -h`. The 39 board issues give the order: macros → `mm_init` →
`extend_heap` → `mm_free` → `coalesce` → `mm_malloc` → `find_fit` → `place` →
`realloc`. First-fit or next-fit is required; explicit or segregated is optional.

No spoilers are declared. **User said** (2026-10-01): the book's text and code
listings are not spoilers, and this week needs knowledge taken from the book;
the course notice says the same (read the listing, understand it, retype it).
So he may read, quote and ask about the book. A question about what a listing
does is a direct request. The agent still does not write or paste his `mm.c`;
the spine's rules (one question per turn, review instead of rewrite) apply.

## Slot 3 — environment

Editor is **Zed** (Windows → WSL remote).

Host: Ubuntu 26.04 · gcc 15 · glibc 2.43 · gdb 17 with his own
`~/.config/gdb/*` (repo `benjohnbill/gdb-config`).
Container (`mallocdbg`, built from upstream `.devcontainer/Dockerfile`, which
uses `ubuntu:latest`): Ubuntu 26.04.1 · gcc 15.2 · glibc 2.43 · gdb 17.1 ·
valgrind 3.26. Measured 2026-10-01: the same versions as the host, so the
container is upstream's reference environment, not a different toolchain. The
same `~/.config/gdb` is bind-mounted read-write, so his gdb commands work there.
Before you describe any gdb command grammar, read
`~/.claude/docs/tool-guides/gdb-commands.md`; it routes to the defining file and
lists what stock-gdb reasoning gets wrong here. The command table from week 5 is
not repeated.

Build facts and `mdriver` quirks:

- Upstream `malloc-lab/Makefile`: `CFLAGS = -Wall -O2 -g`, no `-std`, no `-m32`.
  The CMU original builds 32-bit; here pointers are 8 bytes. `-std=c11` cannot
  build `mdriver.c` (`getopt`, `strdup` undeclared), so `.clangd` uses `gnu23`.
- `-O2` means gdb can show `<optimized out>`. Observed on `argc` in `main`.
- `mdriver` exits 0 even when it reports errors. `-g` prints `correct:N` and
  `perfidx:N`; `perfidx` is 0 when any error occurred.
- `short1` and `short2` are not in the default list (`config.h`); `-f` paths are
  relative to the current directory, so run from `malloc-lab/`.
- The copy of `mdriver.c` here prints `getopt returned: …` (an upstream debug
  line). Read output by whole-line match only.
- `MAX_HEAP` is 20 MB and `AVG_LIBC_THRUPUT` is 600 Kops/s (`config.h`). The
  starter `mm.c` runs out of memory on 5 traces; that is the expected first
  failure. Perf index = 60 × average utilization + 40 × min(1, throughput / 600
  Kops/s). On this host libc malloc runs about 19,000 Kops/s on the default
  traces, so the throughput half is likely to cap early.

`GNUmakefile` is ours and wraps the upstream `Makefile` without editing it:

```
make r_<name> | g_<name>      host: one trace, mdriver -V -f traces/<name>-bal.rep | gdb (tab-completes)
make score                    host: the 11 default traces, mdriver -v → ends with the Perf index line
make verify                   host: 13 traces, exit 0 only if every one is valid (Perf index reported, not gated)
make dcheck | dscore          container: verify | score (reference environment)
make dr_<name> | dg_<name>    container run | gdb
make dshell | dimage | dclean
```

Names: `short1 short2 amptjp cccp cp-decl expr coalescing random random2 binary
binary2 realloc realloc2`.

Host output lives in `malloc-lab/` (upstream's `.gitignore` covers it). The
container copies the sources into `build-docker/` and builds there, so the two
environments never mix objects; `build-docker/` is ignored. Inside the container
gdb lists the **copy** in `build-docker/`, not `malloc-lab/mm.c`; edit the
original. `rebuild` / `rerun` work in both: on the host gdb finds
`malloc-lab/Makefile`, in the container it finds the root `GNUmakefile`, which
copies and rebuilds. Upstream `make` inside `malloc-lab/` still works.

Repo layout: `upstream` = `krafton-jungle/malloc_lab_docker` (pull only),
`origin` = `benjohnbill/SW-AI-malloc-lab`. Sync with `git pull upstream master`.
The CMU repo `krafton-jungle/malloc-lab` is not a remote: the docker repo's
`malloc-lab/` is a copy of it with 64-bit fixes.

Verify: `make verify` on the host, then `make dcheck DOCKER_TTY=-i`; both exit
0 (13/13 valid). The Perf index is reported, not gated.

Board: 5

## Slot 4 — do not touch

`mm.c` is the only file he edits (the handout README says so). Everything else
under `malloc-lab/`, `.vscode/`, `.devcontainer/` and the upstream `README.md`
is upstream-tracked: leave it, and never recommend F5 or "Reopen in Container".
`.gitignore` has two appended lines; keep upstream's. Do not fix the
`getopt returned` debug line in `mdriver.c`.
