#!/bin/env python

#import socket
from ROOT import TChain
from subprocess import check_output

def getSkimNEvents(skim_tag, period, short_name):
   #hostname = socket.gethostname()
   skimdir="/ceph/cms/store/group/tttt/Skims/{}/{}/{}".format(skim_tag, period, short_name)

   # Get the standard path first
   cmdout = check_output(["ExecuteCompiledCommand GetStandardHostPathToStore {} t2.ucsd.edu".format(skimdir)], shell=True)
   if cmdout=='':
      raise RuntimeError("Checking the standard host path to {} failed.".format(skimdir))
   skimdir = cmdout.split()[0]

   # Get the directory contents
   cmdout = check_output(["ExecuteCompiledCommand lsdir {}".format(skimdir)], shell=True)
   if cmdout=='':
      raise RuntimeError("Checking the files in {} failed.".format(skimdir))
   skimfiles = cmdout.split()

   # Count the number of events and return the value
   tc = TChain("Events")
   for ff in skimfiles:
      tc.Add(skimdir+'/'+ff)
   tc.SetBranchStatus("*",0)
   nevents = tc.GetEntries()

   return nevents
