#!/usr/bin/env python3
import sys
try:
    program = sys.argv[1]
except IndexError:
    print("usage: %s <file>")
    exit()
count = 0
print("unsigned char sc[] = {")
sys.stdout.write("    \"")
with open(program, "rb") as fp:
    while True:
        count += 1
        x = fp.read(1)
        if not x:
            break
        sys.stdout.write("\\x%s, " % x.hex())
        if count % 15 == 0:
            sys.stdout.write("\n    ")
print("\"};")
