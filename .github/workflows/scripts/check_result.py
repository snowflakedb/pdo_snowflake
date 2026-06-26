import os
import re
import sys
import fileinput


def read_test_dir(outputdir):
    if not os.path.isdir(outputdir):
        print("[ERROR] test output directory not found: " + outputdir)
        sys.exit(1)
    resultfiles = []
    for (dirpath, dirnames, filenames) in os.walk(outputdir):
        for file in filenames:
            _, file_extension = os.path.splitext(file)
            if file_extension in ('.out', '.log', '.diff'):
                resultfiles.append(file)
    return resultfiles


def main(argv):
    outputdir = argv[0]
    resultfiles = read_test_dir(outputdir)
    if not resultfiles:
        print ("=================== all tests pass ===================")
        exit(0)
    else:
        for file in resultfiles:
            print ("===================" + file + "===================")
            filepath = os.path.join(outputdir, file)
            f= open(filepath, "r")
            print (f.read())
        exit(-1)


if __name__ == "__main__":
    main(sys.argv[1:])
