#!/usr/bin/env bash
#
# This script will try to calibrate the current field using V magnitudes (transformed to Johnson, not vt)
# of Tycho-2 stars in the field. This is useful mostly for wide-field images with blue-sensitive CCD chips.
#
#########################################################
# SET APPROXIMAE FIELD OF VIEW IN ARCMINUTES HERE
#FIELD_OF_VIEW=120
# NOT NEEDED ANYMORE
#########################################################

# Find SExtractor
SEXTRACTOR=`command -v sex 2>/dev/null`
if [ "" = "$SEXTRACTOR" ];then
 SEXTRACTOR=lib/bin/sex
fi


# Check if a local copy of Tycho-2 is available?
VASTDIR=$PWD
TYCHO_PATH=lib/catalogs/tycho2
if [ ! -f $TYCHO_PATH/tyc2.dat.00 ];then
 echo "Tycho-2 catalog was not found at $TYCHO_PATH"
 echo "Would you like to download it now from VizieR (it's big, ~160M)? (y/n)"
 read ANSWER
 if [ "$ANSWER" = "n" ];then
  echo "Well, maybe next time..."
  exit 1
 else
  if [ ! -d $TYCHO_PATH ];then
   mkdir $TYCHO_PATH
  fi
  cd $TYCHO_PATH 
  #wget -nH --cut-dirs=4 --no-parent -r -l0 -c -R 'guide.*,*.gif' "ftp://cdsarc.u-strasbg.fr/pub/cats/I/259/"
  wget -nH --cut-dirs=4 --no-parent -r -l0 -c -A 'ReadMe,*.gz,robots.txt' "http://scan.sai.msu.ru/~kirx/data/tycho2/"
  echo "Download complete. Unpacking..."
  for i in tyc2.dat.*gz ;do
   gunzip $i
  done
  cd $VASTDIR
 fi 
else
 echo "Tycho-2 catalog is found at $TYCHO_PATH"
 # Make sure the catalog is fully downloaded and unpacked
 if [ ! -f $TYCHO_PATH/tyc2.dat.19 ] ;then
  echo "WARNING! One of the catalog files was not found! Will attempt to re-download the catalog."
  cd $TYCHO_PATH 
  #wget -nH --cut-dirs=4 --no-parent -r -l0 -c -R 'guide.*,*.gif' "ftp://cdsarc.u-strasbg.fr/pub/cats/I/259/"
  wget -nH --cut-dirs=4 --no-parent -r -l0 -c -R 'guide.*,*.gif' "http://scan.sai.msu.ru/~kirx/data/tycho2/"
  echo "Download complete. Unpacking..."
  for i in tyc2.dat.*gz ;do
   gunzip $i
  done
  cd $VASTDIR  
 fi
fi

if [ ! -s lib/catalogs/list_of_bright_stars_from_tycho2.txt ];then
 # Create a list of stars brighter than mag 9.5 for filtering transient candidates
 lib/catalogs/create_tycho2_list_of_bright_stars_to_exclude_from_transient_search 9.5
fi

# WCS-calibrate the reference image if it has not been done before (util/wcs_image_calibration.sh will check that)
REFERENCE_IMAGE=`cat vast_summary.log | grep "Ref.  image:" |awk '{print $6}'`
util/wcs_image_calibration.sh $REFERENCE_IMAGE 
if [ $? -ne 0 ];then
 echo "ERROR in $0 : cannot plate-solve the reference image $REFERENCE_IMAGE" >> /dev/stderr
 exit 1
fi
TEST_SUBSTRING=`basename $REFERENCE_IMAGE`
TEST_SUBSTRING="${TEST_SUBSTRING:0:4}"
#TEST_SUBSTRING=`expr substr $TEST_SUBSTRING  1 4`
if [ "$TEST_SUBSTRING" = "wcs_" ];then
 cp $REFERENCE_IMAGE .
 WCS_CALIBRATED_REFERENCE_IMAGE=`basename $REFERENCE_IMAGE`
else
 WCS_CALIBRATED_REFERENCE_IMAGE=wcs_`basename $REFERENCE_IMAGE`
fi

SEXTRACTOR_CATALOG_NAME="$WCS_CALIBRATED_REFERENCE_IMAGE".cat
if [ ! -s "$SEXTRACTOR_CATALOG_NAME" ];then
 echo "ERROR in $0 : cannot find the catalog file $SEXTRACTOR_CATALOG_NAME which was supposed to be created by util/wcs_image_calibration.sh"
 exit 1
fi
cp -v "$SEXTRACTOR_CATALOG_NAME" wcsmag.cat

#valgrind -v --tool=memcheck --leak-check=full  --show-reachable=yes --track-origins=yes lib/catalogs/read_tycho2
util/calibrate_magnitude_scale `lib/catalogs/read_tycho2`
