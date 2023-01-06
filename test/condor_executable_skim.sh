#!/bin/bash

OUTPUTDIR=$1
OUTPUTNAME=$2
INPUTFILENAMES=$3
IFILE=$4
CMSSWVERSION=$5
SCRAMARCH=$6
SKIMARGS="${@:7}" # since args can have spaces, we take all at once first

# Make sure OUTPUTNAME doesn't have .root since we add it manually
OUTPUTNAME=$(echo $OUTPUTNAME | sed 's/\.root//')

export SCRAM_ARCH=${SCRAMARCH}

function getjobad {
  if [[ ! -z "${_CONDOR_JOB_AD+x}" ]]; then
    grep -i "^$1" "${_CONDOR_JOB_AD}" | cut -d= -f2- | xargs echo
  fi
}
function setup_chirp {
  if [[ -e condor_chirp ]]; then
    mkdir chirpdir
    mv condor_chirp chirpdir/
    export PATH="${PATH}:$(pwd)/chirpdir"
    echo "[chirp] Found and put condor_chirp into $(pwd)/chirpdir"
  elif [[ -e /usr/libexec/condor/condor_chirp ]]; then
    export PATH="${PATH}:/usr/libexec/condor"
    echo "[chirp] Found condor_chirp in /usr/libexec/condor"
  else
    echo "[chirp] No condor_chirp :("
  fi
}
function chirp {
  command -v condor_chirp &> /dev/null
  if [[ $? -ne 0 ]]; then
    exit 0 # Just exit normally
  fi
  # Note, $1 (the classad name) must start with Chirp
  condor_chirp set_job_attr_delayed $1 $2
  ret=$?
  echo "[chirp] Chirped $1 => $2 with exit code $ret"
}

setup_chirp

echo -e "\n--- begin header output ---\n" #                     <----- section division
echo "OUTPUTDIR: $OUTPUTDIR"
echo "OUTPUTNAME: $OUTPUTNAME"
echo "INPUTFILENAMES: $INPUTFILENAMES"
echo "IFILE: $IFILE"
echo "CMSSWVERSION: $CMSSWVERSION"
echo "SCRAMARCH: $SCRAMARCH"
echo "SKIMARGS: $SKIMARGS"
# echo  CLASSAD: $(cat "$_CONDOR_JOB_AD")

if [[ ! -z ${GLIDEIN_CMSSite+x} ]]; then
  echo "GLIDEIN_CMSSite: $GLIDEIN_CMSSite"
fi
echo "hostname: $(hostname)"
echo "uname -a: $(uname -a)"
echo "time: $(date +%s)"
echo "args: $@"
echo "tag: $(getjobad tag)"
echo "taskname: $(getjobad taskname)"

echo -e "\n--- end header output ---\n" #                       <----- section division

if [ -r "$OSGVO_CMSSW_Path"/cmsset_default.sh ]; then
  echo "sourcing environment: source $OSGVO_CMSSW_Path/cmsset_default.sh"
  source "$OSGVO_CMSSW_Path"/cmsset_default.sh
elif [ -r "$OSG_APP"/cmssoft/cms/cmsset_default.sh ]; then
  echo "sourcing environment: source $OSG_APP/cmssoft/cms/cmsset_default.sh"
  source "$OSG_APP"/cmssoft/cms/cmsset_default.sh
elif [ -r /cvmfs/cms.cern.ch/cmsset_default.sh ]; then
  echo "sourcing environment: source /cvmfs/cms.cern.ch/cmsset_default.sh"
  source /cvmfs/cms.cern.ch/cmsset_default.sh
else
  echo "ERROR! Couldn't find $OSGVO_CMSSW_Path/cmsset_default.sh or /cvmfs/cms.cern.ch/cmsset_default.sh or $OSG_APP/cmssoft/cms/cmsset_default.sh"
  exit 1
fi

# Set up CMSSW.
# Note that SCRAM_ARCH is already set above, so there is no need to set it again.
eval $(scramv1 project CMSSW $CMSSWVERSION)
cd $CMSSWVERSION
eval $(scramv1 runtime -sh)

# Untar all dependencies
for tarfile in $(find ../ -maxdepth 1 -name '*.tar*'); do
  mv ../${tarfile} ${tarfile}
  tar xf ${tarfile}
done

echo "CMSSW_BASE: ${CMSSW_BASE}"

# Set up the analysis package and go back to the main directory
eval $(./src/tttt/setup.sh env)
cd -

# # logging every 45 seconds gives ~100kb log file/3 hours
# dstat -cdngytlmrs --float --nocolor -T --output dsout.csv 180 >& /dev/null &


echo "Checking CMS_PATH and site configuration..."
if [[ ! -z ${GLIDEIN_CMSSite+x} ]]; then
  declare -i hasSiteConf=1
  if [[ ! -z ${SITECONFIG_PATH+x} ]]; then
    if [[ ! -e ${SITECONFIG_PATH}/JobConfig/site-local-config.xml ]]; then
      echo "${SITECONFIG_PATH}/JobConfig/site-local-config.xml does not exist."
      hasSiteConf=0
    fi
  else
    if [[ ! -e ${CMS_PATH}/SITECONF/local/JobConfig/site-local-config.xml ]]; then
      echo "${CMS_PATH}/SITECONF/local/JobConfig/site-local-config.xml does not exist."
      hasSiteConf=0
    fi
  fi
  if [[ ${hasSiteConf} -eq 0 ]] && [[ -e ${CMS_PATH}/SITECONF/${GLIDEIN_CMSSite}/JobConfig/site-local-config.xml ]]; then
     echo "But ${CMS_PATH}/SITECONF/${GLIDEIN_CMSSite}/JobConfig/site-local-config.xml does exist. Copying it locally."
     mkdir -p ${CMSSW_BASE}/test/SITECONF/local/JobConfig
     cp ${CMS_PATH}/SITECONF/${GLIDEIN_CMSSite}/JobConfig/* ${CMSSW_BASE}/test/SITECONF/local/JobConfig/
     export CMS_PATH=${CMSSW_BASE}/test
     export SITECONFIG_PATH=${CMS_PATH}/SITECONF/local
     if [[ -f ${SITECONFIG_PATH}/JobConfig/cmsset_local.sh ]]; then
       source ${SITECONFIG_PATH}/JobConfig/cmsset_local.sh
     fi
  fi
fi



echo "before running: ls -lrth"
ls -lrth

echo -e "\n--- begin running ---\n" #                           <----- section division

chirp ChirpMetisExpectedNevents $EXPECTEDNEVTS

chirp ChirpMetisStatus "before_cmsRun"

# Change file names to standard paths
if [[ "${INPUTFILENAMES}" == *"/hadoop/cms"* ]] || [[ "${INPUTFILENAMES}" == *"/ceph/cms"* ]]; then
  echo "Need to change input file names for local files on UCSD. Old list of input files: ${INPUTFILENAMES}"
  if [[ "$(hostname)" != *"t2.ucsd.edu"* ]]; then
    INPUTFILENAMES=${INPUTFILENAMES//'/hadoop/cms'/'root://redirector.t2.ucsd.edu:1094/'}
    INPUTFILENAMES=${INPUTFILENAMES//'/ceph/cms'/'root://redirector.t2.ucsd.edu:1095/'}
  else
    INPUTFILENAMES=${INPUTFILENAMES//'/hadoop/cms'/'file:///hadoop/cms'}
    INPUTFILENAMES=${INPUTFILENAMES//'/ceph/cms'/'file:///ceph/cms'}
  fi
  echo "New list of input files: ${INPUTFILENAMES}"
fi

# Create this file in order to detect that a Condor job is running
touch RUNNING_ON_CONDOR

cmdRun="skim_UL inputs=${INPUTFILENAMES} output=${OUTPUTNAME}.root ${SKIMARGS}"
echo "Running: ${cmdRun}"
${cmdRun}
CMSRUN_STATUS=$?

chirp ChirpMetisStatus "after_cmsRun"

echo "after running: ls -lrth"
ls -lrth

if [[ $CMSRUN_STATUS != 0 ]]; then
  echo "Removing output file because cmsRun crashed with exit code $?"
  rm ${OUTPUTNAME}.root
  exit 1
fi

CheckFileIntegrity ${OUTPUTNAME}.root
INTEGRITY_STATUS=$?
if [[ $INTEGRITY_STATUS != 0 ]]; then
  echo "Removing output file because integrity check failed with exit code $?"
  rm ${OUTPUTNAME}.root
  exit 1
fi

echo -e "\n--- end running ---\n" #                             <----- section division

echo "Sending output file ${OUTPUTNAME}.root"

if [[ ! -e "${OUTPUTNAME}.root" ]]; then
  echo "ERROR! Output ${OUTPUTNAME}.root doesn't exist"
  exit 1
fi

echo "time before copy: $(date +%s)"
chirp ChirpMetisStatus "before_copy"

copyFromCondorToSite.sh $(pwd) ${OUTPUTNAME}.root t2.ucsd.edu ${OUTPUTDIR} ${OUTPUTNAME}_${IFILE}.root

echo -e "\n--- begin dstat output ---\n" #                      <----- section division
# cat dsout.csv
echo -e "\n--- end dstat output ---\n" #                        <----- section division

echo "time at end: $(date +%s)"

chirp ChirpMetisStatus "done"
