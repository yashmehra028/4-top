# tttt - Four Top Analysis

# Run 2 Repo
https://github.com/cmstas/FTAnalysis

# Setup
```
git clone git@github.com:joseph-crowley/tttt.git
git clone git@github.com:usarica/ProjectMetis.git
git clone git@github.com:IvyFramework/IvyDataTools.git IvyFramework/IvyDataTools
cd tttt
./setup.sh -j [Ncores]
# To clean: ./setup.sh clean
```

# How to submit jobs using Metis to produce skims
In a screen, run the following:
```
cd ../ProjectMetis
. setup.sh
cd -
cd test
mtarfile tarball.tar.xz --xz --xz_level 3 -x NanoTools HiggsAnalysis/CombinedLimit
```
This will make a tarball to be uploaded to Condor.
You can then run the skim submission script as follows:
```
python submit_skims.py --tarfile tarball.tar.xz --tag [TAG] [CSV FILES]
```

# How to submit jobs to run on the skims
Standalone scripts are used to submit these jobs. The first step is to run configureFTAnalysisJobs.py, e.g.,
```
configureFTAnalysisJobs.py --exe analyze_cutbased --nthreads=4 --nevts 500000 \
--tag 220920_TopLeptonMVA \
output_tag=220920_TopLeptonMVA input_tag=220705 muon_id=TopMVA_Run2 electron_id=TopMVA_Run2 \
test/samples_MC.csv
```
for the MC, or
```
configureFTAnalysisJobs.py --exe analyze_cutbased --nthreads=4 --nevts 500000 \
--tag 220920_TopLeptonMVA --group_eras \
output_tag=220920_TopLeptonMVA input_tag=220705 muon_id=TopMVA_Run2 electron_id=TopMVA_Run2 \
test/samples_Data.csv
```
for the data (type --help to see which arguments are optional).

Notice that arguments that are preceded by '--' are arguments to modify how the python script runs,
and the other arguments control how the executable ("analyze_cutbased" in this example) should run.
The last argument in this example (but it doesn't have to be the last) is the csv file to acquire the data sets.
The arguments to the executable also play the purpose of filtering the data sets if you specify any entry already in the csv
(e.g., 'short_name=TT_2l2nu'), or 'output_file', which is different from 'short_name' only if real data processing is grouped into eras.

After you run this first step, all submission folders are created, but the jobs are not yet submitted.
In order to submit them, following the examples above, you need to run
```
watchFTAnalysisJobs.sh test/output/tasks/220920_TopLeptonMVA # Add sleep=N in seconds to modify sleep duration
```
This script will trigger the checking and resubmission of jobs until all jobs in the folder you specified are completed.
Internally, this script runs checkFTAnalysisJobs.sh and resubmitFTAnalysisJobs.sh, and sleeps for a while before checking again.
When all jobs are done, the watch will terminate.
Make sure to run the watch in a screen! Otherwise, you terminal will be kept busy.
