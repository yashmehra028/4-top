#!/bin/bash


CMSSW_release=CMSSW_10_6_26 # CMSSW version to be used
SCRAM_ARCH_name=slc7_amd64_gcc820 # SCRAM_ARCH that goes along with the CMSSW version
CMSSW_release_name="${CMSSW_release}" # Possibly a different directory name in case you have multiple identical directories

declare -i printhelp=0
for fargo in "$@"; do
  fcnargname=""
  farg="${fargo//\"}"
  fargl="$(echo $farg | awk '{print tolower($0)}')"
  if [[ "${fargl}" == "dirname="* ]]; then
    fcnargname="$farg"
    fcnargname="${fcnargname#*=}"
    CMSSW_release_name="$fcnargname"
  elif [[ "$fargl" == "cmssw="* ]]; then
    fcnargname="$farg"
    fcnargname="${fcnargname#*=}"
    CMSSW_release="$fcnargname"
  elif [[ "$fargl" == "scram_arch="* ]]; then
    fcnargname="$farg"
    fcnargname="${fcnargname#*=}"
    SCRAM_ARCH_name="$fcnargname"
  elif [[ "$fargl" == "help" ]]; then
    let printhelp=1
  fi
done

if [[ $printhelp -eq 1 ]] || [[ -z "${CMSSW_release}" ]] || [[ -z "${SCRAM_ARCH_name}" ]]; then
  echo "$0 usage:"
  echo '(Options are case-insensitive, but arguments are.)'
  echo " - help: Print this help"
  echo " - cmssw: CMSSW version to be used"
  echo " - scram_arch: SCRAM_ARCH that goes along with the CMSSW version"
  echo " - dirname: Possibly a different directory name in case you have multiple identical directories"
  exit 0
fi


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
if [[ ! -d ${CMSSW_release_name} ]]; then
  scramv1 p -n ${CMSSW_release_name} CMSSW ${CMSSW_release}
fi
cd ${CMSSW_release_name}/src
eval $(scramv1 runtime -sh)

cd ${CMSSW_BASE}/src

if [[ ! -d IvyFramework/IvyDataTools ]]; then
  git clone git@github.com:IvyFramework/IvyDataTools.git IvyFramework/IvyDataTools
fi
if [[ ! -d IvyFramework/IvyMLTools ]]; then
  git clone git@github.com:IvyFramework/IvyMLTools.git IvyFramework/IvyMLTools
fi
if [[ ! -d ProjectMetis ]]; then
  git clone git@github.com:usarica/ProjectMetis.git
fi
if [[ ! -d tttt ]]; then
  git clone git@github.com:joseph-crowley/tttt.git
fi

# Download external content
./tttt/downloadExternal.sh

)