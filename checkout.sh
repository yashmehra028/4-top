#!/bin/bash


CMSSW_release=CMSSW_10_6_26
SCRAM_ARCH_name=slc7_amd64_gcc820
CMSSW_release_name="$1"    # Could leave this blank if you don't know what it is. It's just a directory name in case you have multiple identical directories.


(

getcmssw(){
  if [[ ! -z ${OSGVO_CMSSW_Path+x} ]] && [[ -r "${OSGVO_CMSSW_Path}/cmsset_default.sh" ]]; then
    source "${OSGVO_CMSSW_Path}/cmsset_default.sh"
  elif [[ ! -z ${OSG_APP+x} ]] && [[ -r "${OSG_APP}/cmssoft/cms/cmsset_default.sh" ]]; then
    source "${OSG_APP}/cmssoft/cms/cmsset_default.sh"
  elif [[ -r /cvmfs/cms.cern.ch/cmsset_default.sh ]]; then
    source /cvmfs/cms.cern.ch/cmsset_default.sh
  else
    echo "Couldn't find a valid path for cmsset_default.sh"
    exit 1
  fi
}


cd $(dirname ${BASH_SOURCE[0]})

export SCRAM_ARCH=${SCRAM_ARCH_name}
getcmssw


set -euo pipefail


if [[ "${CMSSW_release_name}" == "" ]]; then
  CMSSW_release_name=${CMSSW_release}
fi
scramv1 p -n ${CMSSW_release_name} CMSSW ${CMSSW_release}
cd ${CMSSW_release_name}/src
eval $(scramv1 runtime -sh)

cd ${CMSSW_BASE}/src

git clone git@github.com:IvyFramework/IvyDataTools.git IvyFramework/IvyDataTools
git clone git@github.com:cmstas/NanoTools.git
git clone git@github.com:joseph-crowley/tttt.git

)