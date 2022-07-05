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

# How to submit jobs using Metis
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
