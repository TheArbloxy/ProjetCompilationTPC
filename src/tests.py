import subprocess
import os

EXE_DIR = ["./bin/tpcas"]
GOOD_FILES_DIR = "./test/good/"
ERROR_FILES_DIR = "./test/syn-err/"

TEXT_ARGS = ["-t"]
HELP_ARGS = ["-h"]

GOOD_RETURN_VALUE = 0
ERROR_RETURN_VALUE = 1

class GlobalTests:
    def __init__(self, number_files : int):
        self.number_files = number_files
        self.score = 0
        self.percent_sucess = 0.0

    def good_result_check(self, return_code : int) -> bool:
        if return_code == GOOD_RETURN_VALUE:
            self.score += 1
            return True
        return False

    def bad_result_check(self, return_code : int) -> bool:
        if return_code == ERROR_RETURN_VALUE:
            self.score += 1
            return True
        return False

    def calculate_percent_success(self):
        self.percent_sucess = round((self.score / self.number_files) * 100)

if __name__ == "__main__":
    # Get the name of the tpc test files (both from good and syn-err)
    onlyfiles = [f for f in os.listdir(GOOD_FILES_DIR) if os.path.isfile(os.path.join(GOOD_FILES_DIR, f))]
    onlyfiles.sort()

    onlyfailedfiles = [f for f in os.listdir(ERROR_FILES_DIR) if os.path.isfile(os.path.join(ERROR_FILES_DIR, f))]
    onlyfailedfiles.sort()

    gTests = GlobalTests(len(onlyfiles) + len(onlyfailedfiles)) 

    # Test check
    print("\nfailed good tests check\n----------------------------------------")
    for file in onlyfiles:
        with open(GOOD_FILES_DIR + file, "r") as f:
            result_exe = subprocess.run(EXE_DIR, capture_output = True, text = True, stdin=f) # Launch executables
            
            if (not gTests.good_result_check(result_exe.returncode)): # If the test failed, prints it
                print(f"{file} | return code : {result_exe.returncode} | expected : {GOOD_RETURN_VALUE}")

            f.close()

    # Test check
    print("\nfailed syn-err tests check\n----------------------------------------")
    for file in onlyfailedfiles:
        with open(ERROR_FILES_DIR + file, "r") as f:
            result_exe = subprocess.run(EXE_DIR, capture_output = True, text = True, stdin=f) # Launch executables

            if (not gTests.bad_result_check(result_exe.returncode)): # If the test failed, prints it
                print(f"{file} | return code : {result_exe.returncode} | expected : {ERROR_RETURN_VALUE}")

            f.close()

    gTests.calculate_percent_success()

    print(f"\nScore : {gTests.score} / {gTests.number_files}")
    print(f"{gTests.percent_sucess}% success rate")