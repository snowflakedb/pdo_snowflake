import os
import re
import sys
import fileinput


def read_test_dir(outputdir):
    allfiles= []
    resultfiles = []
    try:
        for (dirpath, dirnames, filenames) in os.walk(outputdir):
            allfiles.extend(filenames)
        for file in allfiles:
            filename, file_extension = os.path.splitext(file)
            if file_extension == '.out' or file_extension == '.log':
                resultfiles.append(file)
        return resultfiles
    except:
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
