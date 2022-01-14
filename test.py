#!/usr/bin/env python3

import subprocess
import sys
import os


def get_files(dir) -> list[str]:
    result = []

    directory = os.fsencode(dir)
    ext = ".fth"

    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        if filename.endswith(ext):
            result.append(filename[:-4])

    return result


def run_test(dir: str, test: str, verbose: bool) -> bool:
    output: subprocess.CompletedProcess = subprocess.run(["./forth", f"{dir}{test}.fth"], capture_output=True)
    stdout = output.stdout

    # exit(1) will show up as 1 here
    returncode = output.returncode
    if verbose and returncode != 0:
        print(f"Exit code {returncode} while running `{test}`. Logging standard output.")
        print(stdout.decode("utf-8"))

    if not os.path.isfile(f"{dir}{test}.o"):
        print(f"Output file for `{test}` not found")
        return False
    with open(f"{dir}{test}.o", "rb") as expected:
        expectedout = expected.read()
    return stdout == expectedout

def parse_argv(dir: str, tests: list[str]) -> tuple[bool]:
    flags = []
    verbose = False
    for arg in sys.argv:
        if arg.startswith("-"):
            flags.extend(f for f in arg[1:])
    for f in flags:
        if f == "f":
            print("Overriding all `.o` files.\n")
            for test in tests:
                output: subprocess.CompletedProcess = subprocess.run(["./forth", f"{dir}{test}.fth"], capture_output=True)
                stdout = output.stdout
                with open(f"{dir}{test}.o", "wb+") as o:
                    o.write(stdout)
        elif f == "v":
            verbose = True
        else:
            print(f"Unknown flag: `{f}`")
    return (verbose, )


def main() -> None:
    dir: str = "tests/"
    tests: list[str] = get_files(dir)
    flags: tuple[bool] = parse_argv(dir, tests)
    verbose = flags[0]
    passed: int = 0
    total: int = len(tests)

    for test in tests:
        if run_test(dir, test, verbose):
            passed += 1
        else:
            print(f"Test `{test}` failed.")

    print(f"Tests: {passed}/{total}")
    if passed == total:
        print("All tests passed! :D")


if __name__ == "__main__":
    main()
