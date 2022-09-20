#!/bin/sh


EXTRAFILES=""
for fargo in "$@";do
  fcnargname=""
  farg="${fargo//\"}"
  fargl="$(echo $farg | awk '{print tolower($0)}')"
  if [[ "$fargl" == "addfile="* ]];then
    fcnargname="$farg"
    fcnargname="${fcnargname#*=}"
    fcnargname="$(readlink -e $fcnargname)"
    removename="${CMSSW_BASE}/"
    fcnargname="${fcnargname//$removename}"
    if [[ -z "$EXTRAFILES" ]];then
      EXTRAFILES="$fcnargname"
    else
      EXTRAFILES="$EXTRAFILES $fcnargname"
    fi
  fi
done


TARFILE="ftanalysis.tar"
echo "SCRAM_ARCH: ${SCRAM_ARCH}"

HERE=$(pwd)

pushd $CMSSW_BASE

extraTarFile=""
if [[ ! -z "$EXTRAFILES" ]];then
  extraTarFile="extras.tar"
  echo "Will include these files in ${extraTarFile}: ${EXTRAFILES}"
  tar Jcvf $extraTarFile $EXTRAFILES
  mv $extraTarFile extras.more # HACK: Hide file extension
  extraTarFile="extras.more"
fi

extraIncludes=""
extraExcludes=""
if [[ -d ${CMSSW_BASE}/src/HiggsAnalysis/CombinedLimit ]]; then
  extraIncludes="${extraIncludes} src/HiggsAnalysis/CombinedLimit"
  extraExcludes="${extraExcludes} --exclude=src/HiggsAnalysis/CombinedLimit/data --exclude=src/HiggsAnalysis/CombinedLimit/doc*"

  echo "extraIncludes is now ${extraIncludes}"
  echo "extraExcludes is now ${extraExcludes}"
fi

tar Jcvf ${TARFILE} ${extraTarFile} ${extraIncludes} \
lib \
biglib \
bin \
src/IvyFramework \
src/tttt \
--exclude=src/*{/*,}/obj \
--exclude=src/*{/*,}/src \
--exclude=src/*{/*,}/bin \
--exclude=src/*{/*,}/scripts ${extraExcludes} \
--exclude=src/*{/*,}/test/tasks \
--exclude=src/*{/*,}/test/Pdfdata \
--exclude=src/*{/*,}/test/br.sm* \
--exclude=src/*{/*,}/test/*.dat \
--exclude=src/*{/*,}/test/*.DAT \
--exclude=src/*{/*,}/test/tmp* \
--exclude=src/*{/*,}/test/temp* \
--exclude=src/tttt/test/*.root \
--exclude=src/tttt/test/*.txt \
--exclude=src/tttt/test/*.csv \
--exclude=src/tttt/test/*.sh \
--exclude=src/tttt{/test,}/output* \
--exclude=src/tttt{/test,}/manual* \
--exclude=src/tttt{/test,}/*.md \
--exclude=src/tttt{/test,}/*.d \
--exclude=src/tttt{/test,}/*.a \
--exclude=src/tttt{/test,}/*.o \
--exclude=src/tttt{/test,}/*.so \
--exclude=src/tttt{/test,}/*.pcm \
--exclude={.git,.gitignore,*.tar,*.tar.xz,*.tar.gz,*.pyc,*.mod,*.out,*.bkp,summary.json}


if [[ ! -z "$extraTarFile" ]];then
  rm -f $extraTarFile
fi

mv $TARFILE $HERE/

popd
