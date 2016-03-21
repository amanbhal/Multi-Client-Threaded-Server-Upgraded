#!/usr/bin/env python2

import subprocess
import os
import sys
import uuid

class TestSubmission(object):

    def __init__(self):
        self.targz_name = "epochlabs_submission.tar.gz"
        self.tmp_dir = "/tmp/epochlabs_testsubmission.%s" % uuid.uuid4()
        self.sub_dir = "epochlabs_submission"
        self.verbose = False
        if self.verbose:
            self.stdout = sys.stdout
            self.stderr = sys.stderr
        else:
            self.stdout = open(os.devnull, 'wb')
            self.stderr = open(os.devnull, 'wb')

    def __del__(self):
        if not self.verbose:
            self.stdout.close()
            self.stderr.close()

    def test(self):
        if not os.path.isfile(self.targz_name):
            raise Exception("the submission needs to be named %s" % self.targz_name)
        os.mkdir(self.tmp_dir)
        if subprocess.call(["tar", "-xzf", self.targz_name, "--directory", self.tmp_dir], stdout=self.stdout, stderr=self.stderr) != 0:
            raise Exception("could not extract the submission. needs to be a gzip compressed tar.")
        if not os.path.isdir(os.path.join(self.tmp_dir, self.sub_dir)):
            raise Exception("submission must contain directory %s" % self.sub_dir)
        if not os.path.isfile(os.path.join(self.tmp_dir, self.sub_dir, "makefile")):
            raise Exception("submission should have a makefile per README")
        if not os.path.isfile(os.path.join(self.tmp_dir, self.sub_dir, "submission.txt")):
            raise Exception("submission should have a submission.txt per README")
        if subprocess.call(
                ["make"],
                cwd=os.path.join(self.tmp_dir, self.sub_dir),
                stdout=self.stdout,
                stderr=self.stderr) != 0:
            raise Exception("'make' on submission was unsuccessful. are all necessary files included?")
        print "submission format is correct. be sure to run test_server.py to verify the submission actually works."


if __name__ == "__main__":
    TestSubmission().test()
