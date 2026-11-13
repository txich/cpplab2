#!/usr/bin/env python3
"""Build and test the module without leaving build artifacts in the project."""

import argparse
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

from model_test import test as test_model


TESTS = Path(__file__).resolve().parent
ROOT = TESTS.parent


def run(command):
    result = subprocess.run([str(part) for part in command], check=False)
    if result.returncode:
        raise SystemExit(result.returncode)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sanitize", action="store_true", help="enable UndefinedBehaviorSanitizer"
    )
    parser.add_argument(
        "--levels", type=int, nargs="+", default=[1, 2, 4, 16],
        help="positive LEVEL_MAX values (default: 1 2 4 16)",
    )
    parser.add_argument(
        "--stress", action="store_true", help="run the large-graph suite instead"
    )
    parser.add_argument(
        "--edges", type=int, default=100000,
        help="edges per base graph in the stress suite (default: 100000)",
    )
    args = parser.parse_args()
    if not __debug__:
        parser.error("run Python without -O; model tests require assertions")
    if any(level < 1 for level in args.levels):
        parser.error("all levels must be positive")
    if not 2 <= args.edges <= (2**31 - 3) // 2:
        parser.error("--edges must be between 2 and 1073741822")
    cpp = shlex.split(os.environ.get("CXX", "g++"))
    cc = shlex.split(os.environ.get("CC", "gcc"))
    options = ["-Wall", "-Wextra", "-O2"]
    if args.sanitize:
        options += ["-g", "-fsanitize=undefined", "-fno-sanitize-recover=all"]

    with tempfile.TemporaryDirectory(prefix="rgc-tests-") as directory:
        build = Path(directory)
        objects = {}

        def build_module(level):
            if level not in objects:
                obj = build / f"module_{level}.o"
                run(cpp + options + [
                    "-std=c++23", "-fPIC", f"-DLEVEL_MAX={level}", "-c",
                    ROOT / "recursive_graph_collections.cpp", "-o", obj,
                ])
                objects[level] = obj
            return objects[level]

        for level in dict.fromkeys(args.levels):
            print(f"Testing LEVEL_MAX={level}", flush=True)
            obj = build_module(level)
            binary = build / f"test_{level}"
            source = "stress_test.cpp" if args.stress else "recursive_graph_collections_test.cpp"
            run(cpp + options + [
                "-std=c++23", f"-DLEVEL_MAX={level}", "-I", ROOT,
                TESTS / source, obj,
                "-o", binary,
            ])
            run([binary, args.edges] if args.stress else [binary])
            if args.stress:
                continue
            library = build / f"lib_{level}.so"
            run(cpp + options + ["-shared", obj, "-o", library])
            test_model(library, level)

        if args.stress:
            print("All stress tests passed.", flush=True)
            return

        # The supplied examples expect levels 4 and 16, respectively.
        c_example = build / "example_c.o"
        run(cc + options + [
            "-std=c23", "-I", ROOT, "-c",
            TESTS / "examples" / "recursive_graph_collections_example_1.c", "-o", c_example,
        ])
        c_binary = build / "example_c"
        run(cpp + options + [c_example, build_module(4), "-o", c_binary])
        run([c_binary])
        print("PASS: supplied C example", flush=True)

        cpp_example = build / "example_cpp.o"
        run(cpp + options + [
            "-std=c++23", "-I", ROOT, "-c",
            TESTS / "examples" / "recursive_graph_collections_example_2.cpp", "-o", cpp_example,
        ])
        module = build_module(16)
        for index, order in enumerate(((cpp_example, module), (module, cpp_example))):
            binary = build / f"example_cpp_{index}"
            run(cpp + options + [*order, "-o", binary])
            run([binary])
        print("PASS: supplied C++ example in both link orders", flush=True)

        invalid = subprocess.run(
            [*cpp, "-std=c++23", "-DLEVEL_MAX=0", "-fsyntax-only",
             str(ROOT / "recursive_graph_collections.cpp")],
            capture_output=True, text=True,
        )
        if invalid.returncode == 0 or "static assertion" not in invalid.stderr.lower():
            raise SystemExit("FAIL: LEVEL_MAX=0 should fail its static assertion")
        print("PASS: LEVEL_MAX=0 rejected at compile time", flush=True)

    print("All tests passed.", flush=True)


if __name__ == "__main__":
    main()
