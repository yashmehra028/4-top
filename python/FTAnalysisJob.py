#!/bin/env python

import sys
import os
import socket
import argparse


class BatchManager:
   def __init__(self, custom_args=None):
      # define options and arguments ====================================
      self.parser = argparse.ArgumentParser()

      self.parser.add_argument("--batchqueue", type=str, help="HTCondor batch queue", required=True)
      self.parser.add_argument("--batchscript", type=str, help="Name of the HTCondor job script", required=True)
      self.parser.add_argument("--batchsites", type=str, help="Sites to run the jobs", required=True)
      self.parser.add_argument("--tarfile", type=str, help="Name of the tar file to upload", required=True)

      self.parser.add_argument("--condorsite", type=str, help="Output site to transfer files", required=True)
      self.parser.add_argument("--condoroutdir", type=str, help="Output directory in the specified 'condorsite'", required=True)

      self.parser.add_argument("--outdir", type=str, help="Name of the job output directory", required=True)
      self.parser.add_argument("--outlog", type=str, help="Name of the output log file", required=True)
      self.parser.add_argument("--errlog", type=str, help="Name of the output error file", required=True)

      self.parser.add_argument("--exe", type=str, help="Name of the executable", required=True)
      self.parser.add_argument("exe_args", help="Arguments passed to the executable", nargs="+")

      self.parser.add_argument("--required_memory", type=str, default="2048M", help="Required RAM for the job")
      self.parser.add_argument("--required_disk", type=str, default="5G", help="Required disk for the job")
      self.parser.add_argument("--required_ncpus", type=int, default=1, help="Required number of CPUs for the job")
      self.parser.add_argument("--job_flavor", type=str, default="tomorrow", help="Time limit for job (tomorrow = 1 day, workday = 8 hours, see https://batchdocs.web.cern.ch/local/submit.html#job-flavours for more)")

      self.parser.add_argument("--dry", action="store_true", default=False, help="Do not submit jobs, just set up the files")

      parser_args = None
      if custom_args is not None:
         if isinstance(custom_args, list):
            parser_args = custom_args
         elif isinstance(custom_args, str):
            parser_args = custom_args.split()
         else:
            raise RuntimeError("Unknown type for custom_args.")
      self.args = self.parser.parse_args(parser_args)

      if self.args.outdir.startswith("./"):
         self.args.outdir = self.opt.outdir.replace(".",os.getcwd(),1)

      if not os.path.isfile(self.args.batchscript):
         print("Batch script does not exist in current directory, will search for CMSSW_BASE/bin")
         sys.exit("Batch script {} does not exist. Exiting...".format(self.args.batchscript))

      #self.args.exe_args = translateFromPythonToShell(self.args.exe_args)

      self.submitJobs()


   def produceCondorScript(self):
      hostname = socket.gethostname()

      scramver = os.getenv("SCRAM_ARCH")
      singularityver = "cms:rhel6"
      if "slc7" in scramver:
         singularityver = "cms:rhel7"

      strproject = r''
      strrequirements = r'Requirements = (HAS_SINGULARITY=?=True && HAS_CVMFS_cms_cern_ch =?= True)'
      if "t2.ucsd.edu" in hostname or "uscms.org" in hostname:
         if "t2.ucsd.edu" in hostname:
            strproject = r'+project_Name = "cmssurfandturf"'
            strrequirements = r'Requirements = (HAS_SINGULARITY=?=True) && !(( regexp("(mh-epyc7662-1)\..*",TARGET.Machine) || regexp("(mh-epyc7662-5)\..*",TARGET.Machine) || regexp("(mh-epyc7662-6)\..*",TARGET.Machine) || regexp("(mh-epyc7662-9)\..*",TARGET.Machine) || regexp("(mh-epyc7662-10)\..*",TARGET.Machine) || regexp("(sdsc-84)\..*",TARGET.Machine) || regexp("(sdsc-3)\..*",TARGET.Machine) || regexp("(cabinet-0-0-29)\..*",TARGET.Machine) || regexp("(cabinet-0-0-23)\..*",TARGET.Machine) || regexp("(cabinet-0-0-21)\..*",TARGET.Machine) || regexp("(cabinet-11-11-3)\..*",TARGET.Machine) )=?=True)'
         else:
            strrequirements = r'Requirements = (HAS_SINGULARITY=?=True) || (NODE_MOUNTS_CVMFS =?= true)'

      scriptargs = {
         "home" : os.path.expanduser("~"),
         "uid" : os.getuid(),
         "SITES" : self.args.batchsites,
         "batchScript" : self.args.batchscript,
         "outDir" : self.args.outdir,
         "outLog" : self.args.outlog,
         "errLog" : self.args.errlog,
         "QUEUE" : self.args.batchqueue,
         "REQMEM" : self.args.required_memory,
         "REQCPUS" : self.args.required_ncpus,
         "REQDISK" : self.args.required_disk,
         "JOBFLAVOR" : self.args.job_flavor,
         "REQUIREMENTS" : strrequirements,
         "PROJECTNAME" : strproject,
         "SINGULARITYVERSION" : singularityver,
         "CMSSWVERSION" : os.getenv("CMSSW_VERSION"),
         "SCRAMARCH" : scramver,
         "TARFILE" : self.args.tarfile,
         "CONDORSITE" : self.args.condorsite,
         "CONDOROUTDIR" : self.args.condoroutdir,
         "RUNEXE" : self.args.exe,
         "RUNEXEARGS" : " ".join(self.args.exe_args)
      }

      scriptcontents = """
universe                = {QUEUE}
+DESIRED_Sites          = "{SITES}"
executable              = {batchScript}
arguments               = {CMSSWVERSION} {SCRAMARCH} {TARFILE} {CONDORSITE} {CONDOROUTDIR} {RUNEXE} {RUNEXEARGS}
Initialdir              = {outDir}
output                  = {outLog}.$(ClusterId).$(ProcId).txt
error                   = {errLog}.$(ClusterId).$(ProcId).err
log                     = $(ClusterId).$(ProcId).log
request_memory          = {REQMEM}
request_cpus            = {REQCPUS}
request_disk            = {REQDISK}
+JobFlavour             = "{JOBFLAVOR}"
x509userproxy           = {home}/x509up_u{uid}
# See also https://www-auth.cs.wisc.edu/lists/htcondor-users/2010-September/msg00009.shtml for periodic_remove.
periodic_remove         = JobStatus == 5
transfer_executable     = True
transfer_input_files    = {TARFILE}
transfer_output_files   = ""
+Owner                  = undefined
notification            = Never
should_transfer_files   = YES
when_to_transfer_output = ON_EXIT_OR_EVICT
+RequiresCVMFS          = true
+SingularityImage       = "/cvmfs/singularity.opensciencegrid.org/cmssw/{SINGULARITYVERSION}"
{REQUIREMENTS}
{PROJECTNAME}

queue

"""
      scriptcontents = scriptcontents.format(**scriptargs)

      self.condorScriptName = "condor.sub"
      condorScriptFile = open(self.args.outdir+"/"+self.condorScriptName,'w')
      condorScriptFile.write(scriptcontents)
      condorScriptFile.close()


   def submitJobs(self):
      self.produceCondorScript()

      jobcmd = "cd {}; condor_submit {}; cd -".format(self.args.outdir, self.condorScriptName)
      if self.args.dry:
         print("Job command: '{}'".format(jobcmd))
      else:
         ret = os.system( jobcmd )
