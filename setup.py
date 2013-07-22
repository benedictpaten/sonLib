#!/usr/bin/env python

from setuptools import setup
import subprocess
import distutils.command.build_py

class BuildWithMake(distutils.command.build_py.build_py):
    """
    Build using make.
    Then do the default build logic.
    
    """
    def run(self):
        # Call make.
        subprocess.check_call(["make"])
        
        # Keep building the Python stuff
        distutils.command.build_py.build_py.run(self)


setup(name="sonLib",
    version="1.0",
    description="Small general purpose library for C and Python with focus on "
    "bioinformatics.",
    author="Benedict Paten",
    author_email="benedict@soe.ucsc.edu",
    url="https://github.com/benedictpaten/sonLib",
    packages=["sonLib"],
    # Hook the build command to also build with make
    cmdclass={"build_py": BuildWithMake},
    # Install all the executable scripts and binaries somewhere on the PATH
    scripts=["bin/sonLibTests", "bin/sonLib_daemonize.py", 
    "bin/sonLib_kvDatabaseTest", "bin/sonLib_cigarTest", 
    "bin/sonLib_fastaCTest"])

    
