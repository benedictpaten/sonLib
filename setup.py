#!/usr/bin/env python

from distutils.core import setup
import subprocess
import distutils.command.build

class BuildWithC(distutils.command.build.build):
    """
    Build binaries that we need to call, which are written in C, using make.
    Then do the default build logic.
    
    """
    def run(self):
        # Build the C stuff, since we call some of those binaries. Use make.
        subprocess.check_call(["make"])
        
        # Keep installing the Python stuff
        distutils.command.build.build.run(self)


setup(name="sonLib",
    version="1.0",
    description="Small general purpose library for C and Python with focus on "
    "bioinformatics.",
    author="Benedict Paten",
    author_email="benedict@soe.ucsc.edu",
    url="https://github.com/benedictpaten/sonLib",
    packages=["sonLib"],
    # The directory we're in is the module directory. It had better be named
    # "sonLib".
    package_dir= {"": ".."},
    # Hook the build command to also build C stuff
    cmdclass={"build": BuildWithC},
    # Install all the executable scripts and binaries somewhere on the PATH
    scripts=["bin/sonLibTests", "bin/sonLib_daemonize.py", 
    "bin/sonLib_kvDatabaseTest", "bin/sonLib_cigarTest", 
    "bin/sonLib_fastaCTest"])

    
