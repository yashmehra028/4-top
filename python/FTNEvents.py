import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

import os
from subprocess import check_output
import csv


def getSkimNEvents(skim_tag, period, short_name):
   skimdir="/ceph/cms/store/group/tttt/Skims/{}/{}/{}".format(skim_tag, period, short_name)

   skim_summary=os.path.expandvars("${CMSSW_BASE}/src/tttt/test/skim_summary.tmp")
   if os.path.exists(skim_summary):
      with open(skim_summary) as fh:
         reader = csv.DictReader(fh)
         for row in reader:
            if row["input_tag"]==skim_tag and row["period"]==period and row["short_name"]==short_name:
               return int(row["nevents"])

   fout = None
   if not os.path.exists(skim_summary):
      fout = open(skim_summary,"w")
      fout.write("input_tag,period,short_name,nevents\n")
      fout.close()

   # Get the standard path first
   cmdout = check_output(["ExecuteCompiledCommand GetStandardHostPathToStore {} t2.ucsd.edu".format(skimdir)], shell=True)
   if cmdout=='':
      raise RuntimeError("Checking the standard host path to {} failed.".format(skimdir))
   skimdir = cmdout.split()[0]

   # Get the directory contents
   nevents = -1
   cmdout = check_output(["ExecuteCompiledCommand DirectoryExists {}".format(skimdir)], shell=True)
   cmdout = cmdout.split()[0]
   if cmdout=="true":
      cmdout = check_output(["ExecuteCompiledCommand lsdir {}".format(skimdir)], shell=True)
      if cmdout=='':
         raise RuntimeError("Checking the files in {} failed.".format(skimdir))
      skimfiles = cmdout.split()

      # Count the number of events and return the value
      tc = ROOT.TChain("Events")
      for ff in skimfiles:
         tc.Add(skimdir+'/'+ff)
      tc.SetBranchStatus("*",0)
      nevents = tc.GetEntries()
   else:
      print("getSkimNEvents: WARNING! {} is not found.".format(skimdir))
      return nevents

   fout = open(skim_summary,"a")
   fout.write(",".join([skim_tag,period,short_name,str(nevents)])+'\n')
   fout.close()

   return nevents
