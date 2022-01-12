import subprocess
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

def run_test(dir, test) -> bool:
    output: subprocess.CompletedProcess = subprocess.run(["./forth.out", f"{dir}{test}.fth"], capture_output=True)
    stdout = output.stdout
    if not os.path.isfile(f"{dir}{test}.o"):
        print(f"Output file for `{test}` not found")
        return False
    with open(f"{dir}{test}.o", "rb") as expected:
        expectedout = expected.read()
    return stdout == expectedout


def main() -> None:
    dir = "tests/"
    tests = get_files(dir)
    passed = 0
    total = len(tests)

    for test in tests:
        if run_test(dir, test):
            passed += 1
        else:
            print(f"Test `{test}` failed.")

    print(f"Tests: {passed}/{total}")

if __name__ == "__main__":
    main()
