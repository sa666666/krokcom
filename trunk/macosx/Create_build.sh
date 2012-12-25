#!/bin/bash
#
# Creates a KrokCom disk image (dmg) from the command line.
# usage:
#    Create_build.sh <version>
#
# The result will be a file called ~/Desktop/KrokCom-<ver>-macosx.dmg

if [ $# != 1 ]; then
	echo "usage: Create_build.sh version"
	exit 0
fi

VER="$1"
DMG="KrokCom-${VER}-macosx.dmg"
DISK="/Volumes/KrokComOSX"
DEST=~/Desktop/${DMG}

if [ -d "${DISK}" ]; then
    echo "Volume already mounted, unmount before proceeding ..."
    exit 0
fi

echo "Creating ${DMG} file ..."
gunzip -c template.dmg.gz > "${DMG}"

echo "Mounting ${DMG} file ..."
hdiutil attach "${DMG}"

echo "Adding Qt framework ..."
macdeployqt ../KrokCom.app/

echo "Copying documentation ..."
ditto ../Announce.txt ../Changes.txt ../Copyright.txt ../License.txt ../Readme.txt "${DISK}"

echo "Copying application ..."
cp -r ../KrokCom.app "${DISK}"

echo "Ejecting ${DMG} ..."
hdiutil eject "${DISK}"

if [ -f "${DEST}" ]; then
	echo "Removing duplicate image found on desktop ..."
    rm -f "${DEST}"
fi

echo "Compressing image, moving to Desktop ..."
hdiutil convert "${DMG}" -format UDZO -imagekey zlib-level=9 -o "${DEST}"
rm -f "${DMG}"
