#!/usr/bin/python
# -*- coding: utf-8 -*-


import os.path





csvfiles=[
          "data/prob_AXF128_120x120_60step.txt",
          "data/prob_BVF047_120x120_60step.txt",
          "data/prob_C3F334_120x120_60step.txt",
          ]

prange = [x/10. for x in range(1,10)]


for filename in csvfiles:
    print(filename)
    filenamebase, ext = os.path.splitext(filename)
        
    for p in prange:
        command  = "./segmentation --file " + filename
        command += " --p " + str(p)
        command += " --outfile " + filenamebase + str(p) + ".txt "
        command += " --outimg " + filenamebase + str(p) + ".png "
        
        print(command)
        os.system(command)
        
        command = "convert " + filenamebase + str(p) + ".{png,pdf} "

        print(command)
        os.system(command)
