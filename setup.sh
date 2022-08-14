#!/bin/bash

(

set -euo pipefail

cd $(dirname ${BASH_SOURCE[0]})

PKGDIR="$(readlink -f .)"
declare -i doPrintEnv=0
declare -i doPrintEnvInstr=0
declare -i needROOFITSYS_ROOTSYS=0
declare -a setupArgs=()

for farg in "$@"; do
  fargl="$(echo $farg | awk '{print tolower($0)}')"
  if [[ "$fargl" == "env" ]]; then
    doPrintEnv=1
  elif [[ "$fargl" == "envinstr" ]]; then
    doPrintEnvInstr=1
  else
    setupArgs+=( "$farg" ) 
  fi
done
declare -i nSetupArgs
nSetupArgs=${#setupArgs[@]}

if [[ -z "${CMSSW_BASE+x}" ]]; then
  echo "You must have a valid CMSSW_BASE environment variable."
  exit 1
fi

xgboost_cfg=${CMSSW_BASE}/config/toolbox/${SCRAM_ARCH}/tools/selected/py2-xgboost.xml
xgboost_version="$(grep -e 'tool name' ${xgboost_cfg} | grep 'py2-xgboost')"
xgboost_version="${xgboost_version##*=\"}"
xgboost_version="${xgboost_version%%\"*}"
xgboost_path="/cvmfs/cms.cern.ch/${SCRAM_ARCH}/external/py2-xgboost/${xgboost_version}/lib/python3.6/site-packages/xgboost"

printenv() {
  if [[ -d ${CMSSW_BASE}/src/IvyFramework/IvyDataTools ]]; then
    envopts="env standalone"
    ${CMSSW_BASE}/src/IvyFramework/IvyDataTools/setup.sh ${envopts}
    eval $(${CMSSW_BASE}/src/IvyFramework/IvyDataTools/setup.sh ${envopts})
  fi

  if [[ -d ${CMSSW_BASE}/src/IvyFramework/IvyMLTools ]]; then
    envopts="env xgboost_path=${xgboost_path}"
    ${CMSSW_BASE}/src/IvyFramework/IvyMLTools/setup.sh ${envopts}
    eval $(${CMSSW_BASE}/src/IvyFramework/IvyMLTools/setup.sh ${envopts})
  fi

  libappend="${PKGDIR}/lib"
  end=""
  if [[ ! -z "${LD_LIBRARY_PATH+x}" ]]; then
    end=":${LD_LIBRARY_PATH}"
  fi
  if [[ "${end}" != *"$libappend"* ]]; then
    echo "export LD_LIBRARY_PATH=${libappend}${end}"
    export LD_LIBRARY_PATH=${libappend}${end}
  fi

  pathappend="${PKGDIR}/executables"
  end=""
  if [[ ! -z "${PATH+x}" ]]; then
    end=":${PATH}"
  fi
  if [[ "${end}" != *"$pathappend"* ]]; then
    echo "export PATH=${pathappend}${end}"
    export PATH=${pathappend}${end}
  fi
}
doenv() {
  if [[ -d ${CMSSW_BASE}/src/IvyFramework/IvyDataTools ]]; then
    envopts="env standalone"
    eval $(${CMSSW_BASE}/src/IvyFramework/IvyDataTools/setup.sh ${envopts})
  fi

  if [[ -d ${CMSSW_BASE}/src/IvyFramework/IvyMLTools ]]; then
    envopts="env xgboost_path=${xgboost_path}"
    eval $(${CMSSW_BASE}/src/IvyFramework/IvyMLTools/setup.sh ${envopts})
  fi
}
printenvinstr () {
  echo
  echo "to use this repo, you must run:"
  echo
  echo 'eval $(./setup.sh env)'
  echo "or"
  echo 'eval `./setup.sh env`'
  echo
  echo "if you are using a bash-related shell, or you can do"
  echo
  echo './setup.sh env'
  echo
  echo "and change the commands according to your shell in order to do something equivalent to set up the environment variables."
  echo
}

if [[ $doPrintEnv -eq 1 ]]; then
    printenv
    exit
elif [[ $doPrintEnvInstr -eq 1 ]]; then
    printenvinstr
    exit
fi

if [[ $nSetupArgs -eq 0 ]]; then
    setupArgs+=( -j 1 )
    nSetupArgs=2
fi


if [[ "$nSetupArgs" -eq 1 ]] && [[ "${setupArgs[0]}" == *"clean"* ]]; then
    make clean

    exit $?
elif [[ "$nSetupArgs" -ge 1 ]] && [[ "$nSetupArgs" -le 2 ]] && [[ "${setupArgs[0]}" == *"-j"* ]]; then
    : ok
else
    echo "Unknown arguments:"
    echo "  ${setupArgs[@]}"
    echo "Should be nothing, env, envinstr, clean, or -j [Ncores]"
    exit 1
fi

doenv


# Run IvyFramework commands
${CMSSW_BASE}/src/IvyFramework/IvyDataTools/setup.sh standalone "${setupArgs[@]}" 1> /dev/null || exit $?
${CMSSW_BASE}/src/IvyFramework/IvyMLTools/setup.sh "xgboost_path=${xgboost_path}" "${setupArgs[@]}" 1> /dev/null || exit $?

# Compile this repository
make "${setupArgs[@]}"
compile_status=$?
if [[ ${compile_status} -ne 0 ]]; then
  echo "Compilation failed with status ${compile_status}."
  exit ${compile_status}
fi

printenvinstr

)
