#!/usr/bin/ksh

datafile=$1
tr -d '\015' < $datafile > TEMPFILE
mv -f TEMPFILE $datafile

