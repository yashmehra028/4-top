#!/bin/bash

getvarpaths(){
  for var in "$@"; do
    tmppath=${var//:/ }
    for p in $(echo $tmppath); do
      if [[ -e $p ]]; then
        echo $p
      fi
    done
  done  
}
searchfileinvar(){
  for d in $(getvarpaths $1); do
    for f in $(ls $d | grep $2); do
      echo "$d/$f"
    done
  done
}
getcmssw(){
  if [ -r "$OSGVO_CMSSW_Path"/cmsset_default.sh ]; then
    echo "Sourcing environment: source $OSGVO_CMSSW_Path/cmsset_default.sh"
    source "$OSGVO_CMSSW_Path"/cmsset_default.sh
  elif [ -r "$OSG_APP"/cmssoft/cms/cmsset_default.sh ]; then
    echo "Sourcing environment: source $OSG_APP/cmssoft/cms/cmsset_default.sh"
    source "$OSG_APP"/cmssoft/cms/cmsset_default.sh
  elif [ -r /cvmfs/cms.cern.ch/cmsset_default.sh ]; then
    echo "Sourcing environment: source /cvmfs/cms.cern.ch/cmsset_default.sh"
    source /cvmfs/cms.cern.ch/cmsset_default.sh
  else
    echo "ERROR! Couldn't find $OSGVO_CMSSW_Path/cmsset_default.sh or /cvmfs/cms.cern.ch/cmsset_default.sh or $OSG_APP/cmssoft/cms/cmsset_default.sh"
    exit 1
  fi
}

CMSSWVERSION="$1"
SCRAMARCH="$2"
TARFILE="$3"
CONDORSITE="$4"
CONDOROUTDIR="$5"
RUNEXE="$6"
RUNEXEARGS="${@:7}" # since args can have spaces, we take all at once


export SCRAM_ARCH=${SCRAMARCH}

echo -e "\n--- begin header output ---\n" #                     <----- section division
echo "CMSSWVERSION: $CMSSWVERSION"
echo "SCRAMARCH: $SCRAMARCH"
echo "CONDORSITE: $CONDORSITE"
echo "CONDOROUTDIR: $CONDOROUTDIR"
echo "RUNEXE: $RUNEXE"
echo "RUNEXEARGS: ${RUNEXEARGS}"

echo "GLIDEIN_CMSSite: $GLIDEIN_CMSSite"
echo "hostname: $(hostname)"
echo "uname -a: $(uname -a)"
echo "whoami: $(whoami)"
echo "time: $(date +%s)"
echo "args: $@"
echo -e "\n--- end header output ---\n" #                       <----- section division

echo -e "\n--- begin memory specifications ---\n" #                     <----- section division
ulimit -a
echo -e "\n--- end memory specifications ---\n" #                     <----- section division


INITIALDIR=$(pwd)
echo "Initial directory is ${INITIALDIR}"

mkdir -p rundir
cd rundir
RUNDIR=$(pwd)

getcmssw

# If the first file in the tarball filelist starts with CMSSW, it is a
# tarball made outside of the full CMSSW directory and must be handled
# differently
if [[ ! -s ${INITIALDIR}/${TARFILE} ]];then
  echo "Tar file ${INITIALDIR}/${TARFILE} does not exist"
fi
if [[ ! -z $(tar -tf ${INITIALDIR}/${TARFILE} | head -n 1 | grep "^CMSSW") ]]; then

  echo "This is a full cmssw tar file."

  mv ${INITIALDIR}/${TARFILE} $(pwd)/
  if [[ "${TARFILE}" == *".tgz" ]];then
    tar zxf ${TARFILE}
  else
    tar xf ${TARFILE}
  fi
  rm ${TARFILE}

  if [[ -e extras.more ]];then
    mv extras.more ${CMSSWVERSION}/extras.tar
  fi

  cd $CMSSWVERSION

  if [[ -e extras.more ]];then
    tar xf extras.tar
    rm extras.tar
  fi

  echo "Current directory ${PWD} =? ${CMSSWVERSION}"
  echo "Running ProjectRename"
  scramv1 b ProjectRename

else

  # Setup the CMSSW area
  echo "This is a selective CMSSW tar file."
  eval `scramv1 project CMSSW $CMSSWVERSION`
  cd $CMSSWVERSION

  mv ${INITIALDIR}/${TARFILE} $(pwd)/
  if [[ "${TARFILE}" == *".tgz" ]];then
    tar zxf ${TARFILE}
  else
    tar xf ${TARFILE}
  fi
  rm ${TARFILE}

  if [[ -e extras.more ]];then
    mv extras.more extras.tar
    tar xf extras.tar
    rm extras.tar
  fi

fi


# Setup the CMSSW environment and cd into ${CMSSW_BASE}/src
eval `scramv1 runtime -sh`
echo "CMSSW_BASE: ${CMSSW_BASE}"

cd src
eval $(./tttt/setup.sh env)


# Check CMS_PATH and site configuration in case the executable internally uses CMS site-specific configurations
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

cd ${RUNDIR}

echo "Run directory before running: ls -lrth"
ls -lrth

##############
# ACTUAL RUN #
##############
echo -e "\n--- Begin RUN ---\n"

# Create this file in order for IvyDataTools to detect that a Condor job is running
touch RUNNING_ON_CONDOR

cmdRun="${RUNEXE} ${RUNEXEARGS}"
echo "Running: ${cmdRun}"
${cmdRun}
RUN_STATUS=$?
if [[ ${RUN_STATUS} != 0 ]]; then
  echo "Run has crashed with exit code ${RUN_STATUS}"
  exit 1
fi

echo -e "\n--- End RUN ---\n"

echo "Run directory after running all steps before file transfers: ls -lrth"
ls -lrth

##################
# TRANSFER FILES #
##################
if [[ -f "EXTERNAL_TRANSFER_LIST.LST" ]]; then
  cat EXTERNAL_TRANSFER_LIST.LST | sort | uniq >> EXTERNAL_TRANSFER_LIST.LST.NEW
  mv EXTERNAL_TRANSFER_LIST.LST.NEW EXTERNAL_TRANSFER_LIST.LST

  echo -e "\n--- Begin EXTERNAL TRANSFER ---\n"
  while IFS='' read -r line || [[ -n "$line" ]]; do
    OUTFILENAME=${line}
    # If there is an instruction to compress, convert the file/directory name into a tar file.
    if [[ "${OUTFILENAME}" == "compress:"* ]]; then
      OUTFILENAME=${OUTFILENAME/'compress:'}
      if [[ "${OUTFILENAME}" == *"/" ]]; then
        OUTFILENAME=${OUTFILENAME%?}
      fi
      tar Jcf ${OUTFILENAME}.tar ${OUTFILENAME}
      OUTFILENAME=${OUTFILENAME}.tar
    fi
    # Begin copying the file
    echo "Copying output file ${OUTFILENAME}"
    copyFromCondorToSite.sh ${RUNDIR} ${OUTFILENAME} ${CONDORSITE} ${CONDOROUTDIR}
    TRANSFER_STATUS=$?
    if [ $TRANSFER_STATUS != 0 ]; then
      echo " - Transfer crashed with exit code ${TRANSFER_STATUS}"
    fi
  done < "EXTERNAL_TRANSFER_LIST.LST"
  echo -e "\n--- End EXTERNAL TRANSFER ---\n"
fi
##############


echo "Run directory after running all steps: ls -lrth"
ls -lrth

echo "time at end: $(date +%s)"
