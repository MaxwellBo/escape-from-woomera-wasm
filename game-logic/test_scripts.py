#!/usr/bin/env python3
"""Parse every Conversations/*.txt in public/woomera.zip."""
from __future__ import annotations

import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ZIP = ROOT / "public" / "woomera.zip"
LOGIC = ROOT / "game-logic"


def main() -> int:
    if not ZIP.is_file():
        print("missing public/woomera.zip", file=sys.stderr)
        return 1
    with tempfile.TemporaryDirectory() as td:
        td_path = Path(td)
        main_cc = td_path / "main.cc"
        main_cc.write_text(
            r"""
#include "efw_script.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
  if (argc < 3) return 2;
  FILE *f = fopen(argv[2], "rb");
  if (!f) return 3;
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buf = (char*)malloc((size_t)n + 1);
  if (!buf) return 5;
  fread(buf, 1, (size_t)n, f);
  buf[n] = 0;
  fclose(f);
  EfwScript s;
  int q = EfwScript_Parse(&s, argv[1], buf, (int)n);
  int greet = EfwScript_FindQuestion(&s, "GREET");
  int bye = EfwScript_FindQuestion(&s, "GOODBYE");
  printf("%s questions=%d greet=%d goodbye=%d first=%s\n",
         argv[1], q, greet, bye, q ? s.questions[0].topic : "-");
  free(buf);
  return q > 0 ? 0 : 4;
}
""",
            encoding="utf-8",
        )
        bin_path = td_path / "test_parse"
        subprocess.check_call(
            [
                "g++",
                "-std=c++11",
                "-O0",
                "-I",
                str(LOGIC),
                "-o",
                str(bin_path),
                str(main_cc),
                str(LOGIC / "efw_script.cpp"),
            ]
        )
        failed = 0
        count = 0
        amir_q = 0
        with zipfile.ZipFile(ZIP) as zf:
            for name in zf.namelist():
                if "/Conversations/" not in name or not name.endswith(".txt"):
                    continue
                raw = zf.read(name)
                script = Path(name).stem
                txt = td_path / f"{script}.txt"
                txt.write_bytes(raw)
                proc = subprocess.run(
                    [str(bin_path), script, str(txt)], capture_output=True, text=True
                )
                line = (proc.stdout or proc.stderr).strip()
                print(line)
                count += 1
                if proc.returncode != 0:
                    failed += 1
                    print("FAIL", script, proc.stderr, file=sys.stderr)
                if script == "Amir" and "questions=" in line:
                    amir_q = int(line.split("questions=")[1].split()[0])
        if amir_q < 8:
            print("Amir should have many questions, got", amir_q, file=sys.stderr)
            failed += 1
        print(f"parsed {count} scripts, failures={failed}")

    needed = {
        "efw_PliersMarker",
        "efw_kitchen_bin",
        "efw_hiding_place",
        "efw_IDTag_Position",
    }
    import re
    import struct

    with zipfile.ZipFile(ZIP) as zf:
        bsp = zf.read("EscapeFromWoomera_v084/maps/efw_prototype_level1.bsp")
    off, ln = struct.unpack_from("<ii", bsp, 4)
    ents = bsp[off : off + ln].decode("latin1", errors="replace")
    found = set(re.findall(r'"targetname"\s+"(efw_[^"]+)"', ents))
    missing = needed - found
    if missing:
        print("level1 missing markers", missing, file=sys.stderr)
        return 1
    print("level1 markers", " ".join(sorted(found & needed)))
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
