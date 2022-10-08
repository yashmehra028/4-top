#!/bin/bash

chkdir="$1"
if [[ ! -d ${chkdir} ]]; then
  echo "Watch directory ${chkdir} is not found."
  exit 1
fi
chkdir=$(readlink -f ${chkdir})
chkdir=${chkdir%/}

declare -i sleepdur=1800
declare -i proxy_valid_threshold=86400 # 1 day
check_opts=""
resubmit_opts=""
mymail=""
for farg in "$@"; do
  fargl="$(echo $farg | awk '{print tolower($0)}')"

  if [[ "$fargl" == "sleep="* ]]; then
    let sleepdur=${fargl//'sleep='}
  elif [[ "$fargl" == "check-opts="* ]]; then
    check_opts="${farg#*=}"
  elif [[ "$fargl" == "resubmit-opts="* ]]; then
    resubmit_opts="${farg#*=}"
  elif [[ "$fargl" == "email="* ]]; then
    mymail="${farg#*=}"
  fi
done

declare -i send_emails=0
if [[ ! -z "${mymail}" ]]; then
  send_emails=1
fi


launch_email(){
  if [[ ${send_emails} -eq 1 ]]; then
    theRecipient="$1"
    theSubject="$2"
    theBody="$3"

    command -v mail &> /dev/null
    if [[ $? -eq 0 ]]; then
      mail -s "${theSubject}" ${theRecipient} <<< "${theBody}"
    else
      rm -f tmpmail.txt
      echo "Subject: ${theSubject}" >> tmpmail.txt
      echo "${theBody}" >> tmpmail.txt
      sendmail ${theRecipient} < tmpmail.txt
      rm -f tmpmail.txt
    fi
  fi
}


# Set X509_USER_PROXY
eval $(python3 -c 'from getVOMSProxy import getVOMSProxy; getVOMSProxy(True)')

thehost=$(hostname)
echo "CondorWatch is launched for ${chkdir} by $(whoami):${thehost}."
launch_email ${mymail} "[CondorWatch] ($(whoami):${thehost}) BEGIN" "A watch on ${chkdir} is launched."

watchlog=${chkdir}/watchlog.txt
declare -i nTOTAL=0
declare -i nFAILED=0
declare -i proxytime=0
while [[ 1 ]]; do
  proxytime=$(voms-proxy-info --timeleft --file=${X509_USER_PROXY})
  if [[ ${proxytime} -lt ${proxy_valid_threshold} ]]; then
    launch_email ${mymail} "[CondorWatch] ($(whoami):${thehost}) WARNING" "Your VOMS proxy file ${proxy_file} will expire in less than 1 day. Please run ./setup.sh under $(pwd) urgently. Otherwise, this script will nag you every hour."
  fi

  rm -f ${watchlog}
  checkFTAnalysisJobs.sh ${chkdir} ${check_opts} &> ${watchlog}
  if [[ $? -ne 0 ]]; then
    launch_email ${mymail} "[CondorWatch] ($(whoami):${thehost}) ERROR" "Command 'checkFTAnalysisJobs.sh ${chkdir} ${check_opts} &> ${watchlog}' failed with error code $?. The script has aborted. Please check the file ${watchlog} for hints."
    exit 1
  fi

  nTOTAL=$(grep -e "Total jobs checked" ${watchlog} | awk '{print $4}')
  nSUCCESS=$(grep -e "ran successfully" ${watchlog} | wc -l)
  if [[ ${nTOTAL} -eq 0 ]] || [[ ${nSUCCESS} -eq ${nTOTAL} ]]; then
    break
  fi

  failed_dirs=( $(grep failed ${watchlog} | awk '{print $1}' | sort | uniq) )

  for d in "${failed_dirs[@]}"; do
    resubmitFTAnalysisJobs.sh ${d} ${resubmit_opts}
    if [[ $? -ne 0 ]]; then
      launch_email ${mymail} "[CondorWatch] ($(whoami):${thehost}) ERROR" "Command 'resubmitFTAnalysisJobs.sh ${d} ${resubmit_opts}' failed with error code $?. The script has aborted."
      exit 1
    fi
  done

  sleep ${sleepdur}
done

echo "CondorWatch on ${chkdir} has completed successfully."
launch_email ${mymail} "[CondorWatch] ($(whoami):${thehost}) FINAL REPORT" "All jobs under ${chkdir} have completed successfully. Please do not forget to exit your screen if you opened one for this watch."
rm -f ${watchlog}
