import os
import sys
import csv
import glob
import re
from pprint import pprint
import argparse

from metis.Sample import DBSSample, DirectorySample
from metis.CMSSWTask import CMSSWTask
from metis.CondorTask import CondorTask
from metis.StatsParser import StatsParser
from metis.Utils import send_email, interruptible_sleep
from metis.Optimizer import Optimizer

# To make tarfile use command:  mtarfile tarball_v1.tar.xz --xz --xz_level 3 -x "JHUGenMELA/MELA/data/Pdfdata" "*JHUGenMELA/MELA/data/*.root"

def get_tasks(args):
   csvs=args.csvs
   tarfile=args.tarfile
   tag=args.tag
   doTestRun=args.testrun
   doXsecRun=args.xsec
   skipDNE=args.skip_dne
   localSearchDir=None
   if hasattr(args, "localSearchDir"):
      localSearchDir=args.localSearchDir

   metis_exe="condor_executable_skim.sh"
   metis_pset=""
   if doXsecRun:
      metis_exe="condor_executable_xsec.sh"
      metis_pset=os.path.expandvars("${CMSSW_BASE}/src/IvyFramework/IvyDataTools/test/xsec_cfg.py")

   if not os.path.exists(tarfile):
      raise RuntimeError("{} doesn't exist!".format(tarfile))

   scram_arch = os.getenv("SCRAM_ARCH")
   cmssw_version = os.getenv("CMSSW_VERSION")

   # Make list of sample objects, one per CSV line
   samples = []
   for fname in csvs:
      with open(fname) as fh:
         reader = csv.DictReader(fh)
         for row in reader:
            dataset = row["dataset"]
            if dataset.strip().startswith("#"):
               continue
            sample = None
            if 'private' in dataset:
               if localSearchDir is None:
                  raise RuntimeError("The collection of samples contains a private sample {}, but no local search directory is specified.".format(dataset))

               dset_local_input_dir = None
               for search_dir in localSearchDir:
                  if os.path.exists(search_dir+dataset):
                     dset_local_input_dir = search_dir+dataset
               if dset_local_input_dir is None:
                  if skipDNE:
                     continue
                  else:
                     raise RuntimeError("Cannot find {} in the specified local search directories.".format(dataset))
               else:
                  print("Files for {} are found under {}".format(dataset, dset_local_input_dir))

               sample = DirectorySample(dataset=dataset, xsec=row["xsec"], efact=row["BR"], location=dset_local_input_dir)
            else:
               sample = DBSSample(dataset=dataset, xsec=row["xsec"], efact=row["BR"])
            sample.info["shortname"] = row["short_name"] # Add another entry into the dictionary of the sample
            sample.info["period"] = row["period"]
            opts=row["extra_arguments"]
            for key in row:
               if "dataset" in key or "extra_arguments" in key:
                  continue
               if opts == "":
                  opts="{}={}".format(key,row[key])
               else:
                  opts="{}={} {}".format(key,row[key],opts)
            sample.info["options"] = opts
            samples.append(sample)
            if doTestRun:
               break # only do one sample per file... FIXME delete this after testing

   tasks = []
   for sample in samples:
      isdata = "Run201" in sample.info["dataset"]

      events_per_output = (150e3 if isdata else 150e3)
      pset_args = sample.info["options"]

      outputfilenamecore = sample.info["shortname"]
      if not outputfilenamecore:
         outputfilenamecore = sample.info["dataset"]
         outputfilenamecore = outputfilename.replace('/NANOAODSIM','')
         outputfilenamecore = outputfilename.replace('/NANOAOD','')
         outputfilenamecore = outputfilename.lstrip('/')
         outputfilenamecore = outputfilename.replace('/','_')
      else:
         outputfilenamecore = sample.info["shortname"]
      outputfilename = outputfilenamecore
      if not doXsecRun:
         outputfilename = outputfilename + ".root"
         pset_args += " dataset={} short_name={} period={}".format(sample.info["dataset"], sample.info["shortname"], sample.info["period"])
      else:
         outputfilename = outputfilename + ".txt"
         if isdata:
            continue
      print("Output file: ",outputfilename)

      # build output directory
      griduname = os.environ.get("GRIDUSER","").strip()  # Set by Metis. Can be different from $USER for some people.
      if not griduname:
         griduname = os.environ.get("USER")
      part1 = sample.info["period"]
      part2 = outputfilenamecore
      output_dir = "/ceph/cms/store/group/tttt/Skims/{}/{}/{}/".format(
         tag, part1, part2
      )

      taskArgs = dict()
      taskArgs["sample"]=sample
      taskArgs["tarfile"]=tarfile
      taskArgs["tag"]=tag
      taskArgs["scram_arch"]=scram_arch
      taskArgs["cmssw_version"]=cmssw_version
      taskArgs["output_name"]=outputfilename
      taskArgs["output_dir"]=output_dir
      taskArgs["executable"]=metis_exe
      taskArgs["is_tree_output"]=False
      taskArgs["dont_check_tree"]=True
      taskArgs["global_tag"]="DUMMY"
      taskArgs["pset"]=metis_pset
      taskArgs["pset_args"]=pset_args
      taskArgs["arguments"]=pset_args
      #taskArgs["no_load_from_backup"]=True
      if 'private' in sample.get_datasetname():
         taskArgs["files_per_output"]=20
      else:
         taskArgs["events_per_output"]=events_per_output
      if doTestRun:
         taskArgs["max_jobs"] = 2 # 2 condor jobs per sample... FIXME delete this after testing
         taskArgs["max_nevents_per_job"] = 200 # 200 events per job... FIXME delete this after testing

      TaskClass = CMSSWTask if doXsecRun else CondorTask
      task = TaskClass(**taskArgs)
      tasks.append(task)

   return tasks


if __name__ == "__main__":
   parser = argparse.ArgumentParser()
   parser.add_argument("csvs", help="csv files with samples", nargs="+")
   parser.add_argument("--tarfile", help="Path to tarball", type=str, required=True)
   parser.add_argument("--tag", help="Production tag", type=str, required=True)
   parser.add_argument("--testrun", help="Run a test", action='store_true', required=False, default=False)
   parser.add_argument("--localSearchDir", help="Search directory for local files. Can specify multiple.", action='append', required=False)
   parser.add_argument("--xsec", help="Compute the xsec instead of making ntuples", action='store_true', required=False, default=False)
   parser.add_argument("--skip_dne", help="Skip folders that do not exist or are empty", action='store_true', required=False, default=False)
   args = parser.parse_args()

   tasks = get_tasks(args)

   total_summary = {}
   optimizer = Optimizer()

   for i in range(10000):
      for task in tasks:
         #task.process(optimizer=optimizer)
         task.process()
         total_summary[task.get_sample().get_datasetname()] = task.get_task_summary()

      StatsParser(data=total_summary, webdir="~/public_html/dump/metis_tttt/").do()
      interruptible_sleep(2*3600)
