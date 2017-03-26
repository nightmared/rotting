#!/bin/sh

DEV_DIR=~/Documents/development/firefox
SCRIPT_DIR=$(pwd)
DEBUG_INSTALL_DIR=~/firefox/ff-debug
RELEASE_INSTALL_DIR=~/firefox/ff

should_continue() {
  if [ $? != 0 ]
  then
    >&2 printf "Failed at step %s !"\\n $1
    exit 1
  fi
}

# usage: exec_step STEP_NAME COMMAND
exec_step() {
  eval "$2 &> $SCRIPT_DIR/$1.log"
  should_continue "$1"
}

BUILD=1

while getopts “unihd?” OPTION
do
  case $OPTION in
    u)
      UPDATE=1 ;;
    i)
      INSTALL=1 ;;
    n)
      BUILD=0 ;;
    d)
      DEBUG=1 ;;
    h | ?)
      echo "$0 [-u] [-n] [-i] [-d]"
      echo '-u --> update'
      echo '-n --> do not build'
      echo '-d --> use debug mozconfig'
      echo '-? | -h --> show this'
      exit ;;
     esac
done

if [ "$DEBUG" == 1 ]
then
  cp "$SCRIPT_DIR/.mozconfig-debug" $DEV_DIR/.mozconfig
  export INSTALL_DIR=$DEBUG_INSTALL_DIR
else
  cp "$SCRIPT_DIR/.mozconfig-release" $DEV_DIR/.mozconfig
  export INSTALL_DIR=$RELEASE_INSTALL_DIR
fi
echo "ac_add_options --prefix=$INSTALL_DIR" >> $DEV_DIR/.mozconfig

cd $DEV_DIR

if [ -n "$UPDATE" ]
then
  exec_step "update" "hg pull -u"
fi
if [ -n "$BUILD" ]
then
  exec_step "build" "./mach build"
fi
if [ -n "$INSTALL" ]
then
  exec_step "install" "./mach install"
fi
