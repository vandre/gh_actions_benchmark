#!/bin/sh

test_if_program_runnable () {
	local PROGRAM=$1
	$(${PROGRAM} >/dev/null 2>&1)
	if [ $? -eq 127 ]; then
		echo $PROGRAM not available.>&2
		echo false
	fi
	echo true
}

$(test_if_program_runnable awk) || exit 1
$(test_if_program_runnable unzip) || exit 1

SCRIPTDIR=$(dirname "$0" 2>/dev/null)
if [ -z "${SCRIPTDIR}" ]; then
	SCRIPTDIR=$(pwd)
fi

SEARCHDIR=$(realpath "${SCRIPTDIR}")
SUBDIR=data
DATADIR=

cd "$SEARCHDIR"

while [ -z "$DATADIR" ]; do
	CURRENT=`pwd`
	CANDIDATE=${CURRENT}/${SUBDIR}
	[ -d $CANDIDATE ] && DATADIR=$CANDIDATE && break
	cd ..
	[ "$(pwd)" == "${CURRENT}" ] && break
done

if [ -z "$DATADIR" ]; then
	echo No data subdirectory ${SUBDIR} found along path from ${SEARCHDIR} to ${CURRENT}. >&2
	exit 1
fi

cd "${DATADIR}"
(echo "[Info]    Found data directory at ${DATADIR}")>&2

for zipfile in $(ls *.zip); do
	for source in $(unzip -l "${zipfile}" | awk 'BEGIN {G=0} ($4=="----") {G=1; next} {if (G>0) print $4}'); do
		if [ -f "${DATADIR}/${source}" ]; then
			(echo "[Warning] Skipping ${zipfile} because contents already exist in data directory...")>&2
		else
			(echo "[Info]    Unzipping ${zipfile} to data directory...")>&2
			unzip -nqq "${zipfile}"
		fi
	done
done
