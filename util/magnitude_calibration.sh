#!/usr/bin/env bash

if [ -f calib.txt ];then
 rm -f calib.txt
fi

if [ ! -f data.m_sigma ];then
 util/nopgplot.sh
fi

if [ -z "$1" ] ;then
 echo "Performing manual magnitude calibration."
 echo "Please specify the comparison stars and their magnitudes!
Opening the reference image..."
 if ! ./pgfv calib ;then
  echo "ERROR running './pgfv calib'"
  exit 1
 fi
else
 # Magnitude calibration for BVRIri bands using APASS magnitudes
 BAND="$1"
 case "$BAND" in
 "C")
 ;;
 "B")
 ;;
 "V")
 ;;
 "R")
 ;;
 "Rc")
 ;;
 "I")
 ;;
 "Ic")
 ;;
 "r")
 ;;
 "i")
 ;;
 *) 
  echo "ERROR: unknown band $BAND"
  exit 1
 esac
 
 echo "Performing automatic magnitude calibration using $BAND APASS magnitudes."
 
 if [ ! -f vast_summary.log ];then
  echo "ERROR: cannot find vast_summary.log file!"
  exit 1
 fi

 grep "Ref.  image: " vast_summary.log &>/dev/null
 if [ $? -ne 0 ];then
  echo "ERROR: cannot parse vast_summary.log file!"
  exit 1
 fi 
 
 REFERENCE_IMAGE=`grep "Ref.  image: " vast_summary.log  |awk '{print $6}'`

 # Test if the original image is already a calibrated one
 # (Test by checking the file name)
 TEST_SUBSTRING=`basename $REFERENCE_IMAGE`
 TEST_SUBSTRING="${TEST_SUBSTRING:0:4}"
 #TEST_SUBSTRING=`expr substr $TEST_SUBSTRING  1 4`
 if [ "$TEST_SUBSTRING" = "wcs_" ];then
  UCAC5_REFERENCE_IMAGE_MATCH_FILE=`basename $REFERENCE_IMAGE`.cat.ucac5
 else
  UCAC5_REFERENCE_IMAGE_MATCH_FILE=wcs_`basename $REFERENCE_IMAGE`.cat.ucac5
 fi
 # if the output catalog file is present and is not empty
 if [ ! -s "$UCAC5_REFERENCE_IMAGE_MATCH_FILE" ];then
  if ! util/solve_plate_with_UCAC5 $REFERENCE_IMAGE ;then
   echo "Sorry, the reference image could not be solved... :("
   exit 1
  fi
 fi
 # yeah, I know: util/solve_plate_with_UCAC5 is supposed to either produce a correct 
 # output file of fail. But still, let's check that.
 # Error check (just in case)
 if [ ! -f $UCAC5_REFERENCE_IMAGE_MATCH_FILE ];then
  echo "ERROR in $0: cannot find file $UCAC5_REFERENCE_IMAGE_MATCH_FILE" >> /dev/stderr
  exit 1
 fi
 if [ ! -s $UCAC5_REFERENCE_IMAGE_MATCH_FILE ];then
  echo "ERROR in $0: empty file $UCAC5_REFERENCE_IMAGE_MATCH_FILE" >> /dev/stderr
  exit 1
 fi

 # Parse the ctalog match file
 case "$BAND" in
 #"C")
 # cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{print $8" "$11" "sqrt($9*$9+$12*$12)}' | while read A B C ;do if [ ! -z $C ];then  if [ "$B" != "0.000" ];then echo "$A  $B  $C" ;fi  ;fi ;done | sort -n > calib.txt
 #;;
 "B")
  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{printf "out%05d.dat %f %f %f \n", $1, $8,$13,$14}' | while read OUTDATFILE A B C ;do 
   if [ -z $C ];then
    continue
   fi
   if [ "$B" == "0.000000" ];then
    continue
   fi
   # Check if this star is constant
   grep --quiet "$OUTDATFILE" vast_list_of_likely_constant_stars.log
   if [ $? -ne 0 ];then
    continue
   fi
   # Replace the magnitude and error measured at this image with the median mag and scatter from all images
   MEDIAN_MAG_AND_SCATTER=`grep "$OUTDATFILE" vast_lightcurve_statistics.log | awk '{print $1" "$2}'`
   MEDIAN_MAG=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $1}'`
   SCATTER=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $2}'`
   COMBINED_ERROR=`echo "$SCATTER $C" | awk '{print sqrt($1*$1+$2*$2)}'`
   # Write output
   echo "$MEDIAN_MAG  $B  $COMBINED_ERROR"
   #if [ ! -z $C ];then  
   # if [ "$B" != "0.000000" ];then 
   #  #echo "$A  $B  $C" 
   # fi
   #fi
  done | sort -n > calib.txt
 ;;
 "V")
#  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{print $8" "$15" "sqrt($9*$9+$16*$16)}' | while read A B C ;do if [ ! -z $C ];then  if [ "$B" != "0.000" ];then echo "$A  $B  $C" ;fi  ;fi ;done | sort -n > calib.txt
  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{printf "out%05d.dat %f %f %f \n", $1, $8,$15,$16}' | while read OUTDATFILE A B C ;do 
   if [ -z $C ];then
    continue
   fi
   if [ "$B" == "0.000000" ];then
    continue
   fi
   # Check if this star is constant
   grep --quiet "$OUTDATFILE" vast_list_of_likely_constant_stars.log
   if [ $? -ne 0 ];then
    continue
   fi
   # Replace the magnitude and error measured at this image with the median mag and scatter from all images
   MEDIAN_MAG_AND_SCATTER=`grep "$OUTDATFILE" vast_lightcurve_statistics.log | awk '{print $1" "$2}'`
   MEDIAN_MAG=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $1}'`
   SCATTER=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $2}'`
   COMBINED_ERROR=`echo "$SCATTER $C" | awk '{print sqrt($1*$1+$2*$2)}'`
   # Write output
   echo "$MEDIAN_MAG  $B  $COMBINED_ERROR"
  done | sort -n > calib.txt
 ;;
 "r")
#  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{print $8" "$17" "sqrt($9*$9+$18*$18)}' | while read A B C ;do if [ ! -z $C ];then  if [ "$B" != "0.000" ];then echo "$A  $B  $C" ;fi  ;fi ;done | sort -n > calib.txt
  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{printf "out%05d.dat %f %f %f \n", $1, $8,$17,$18}' | while read OUTDATFILE A B C ;do 
   if [ -z $C ];then
    continue
   fi
   if [ "$B" == "0.000000" ];then
    continue
   fi
   # Check if this star is constant
   grep --quiet "$OUTDATFILE" vast_list_of_likely_constant_stars.log
   if [ $? -ne 0 ];then
    continue
   fi
   # Replace the magnitude and error measured at this image with the median mag and scatter from all images
   MEDIAN_MAG_AND_SCATTER=`grep "$OUTDATFILE" vast_lightcurve_statistics.log | awk '{print $1" "$2}'`
   MEDIAN_MAG=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $1}'`
   SCATTER=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $2}'`
   COMBINED_ERROR=`echo "$SCATTER $C" | awk '{print sqrt($1*$1+$2*$2)}'`
   # Write output
   echo "$MEDIAN_MAG  $B  $COMBINED_ERROR"
  done | sort -n > calib.txt
 ;;
 "i")
#  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{print $8" "$19" "sqrt($9*$9+$20*$20)}' | while read A B C ;do if [ ! -z $C ];then  if [ "$B" != "0.000" ];then echo "$A  $B  $C" ;fi  ;fi ;done | sort -n > calib.txt
  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{printf "out%05d.dat %f %f %f \n", $1, $8,$19,$20}' | while read OUTDATFILE A B C ;do 
   if [ -z $C ];then
    continue
   fi
   if [ "$B" == "0.000000" ];then
    continue
   fi
   # Check if this star is constant
   grep --quiet "$OUTDATFILE" vast_list_of_likely_constant_stars.log
   if [ $? -ne 0 ];then
    continue
   fi
   # Replace the magnitude and error measured at this image with the median mag and scatter from all images
   MEDIAN_MAG_AND_SCATTER=`grep "$OUTDATFILE" vast_lightcurve_statistics.log | awk '{print $1" "$2}'`
   MEDIAN_MAG=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $1}'`
   SCATTER=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $2}'`
   COMBINED_ERROR=`echo "$SCATTER $C" | awk '{print sqrt($1*$1+$2*$2)}'`
   # Write output
   echo "$MEDIAN_MAG  $B  $COMBINED_ERROR"
  done | sort -n > calib.txt
 ;;
 "R"|"Rc")
#  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{print $8" "$21" "sqrt($9*$9+$22*$22)}' | while read A B C ;do if [ ! -z $C ];then  if [ "$B" != "0.000" ];then echo "$A  $B  $C" ;fi  ;fi ;done | sort -n > calib.txt
  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{printf "out%05d.dat %f %f %f \n", $1, $8,$21,$22}' | while read OUTDATFILE A B C ;do 
   if [ -z $C ];then
    continue
   fi
   if [ "$B" == "0.000000" ];then
    continue
   fi
   # Check if this star is constant
   grep --quiet "$OUTDATFILE" vast_list_of_likely_constant_stars.log
   if [ $? -ne 0 ];then
    continue
   fi
   # Replace the magnitude and error measured at this image with the median mag and scatter from all images
   MEDIAN_MAG_AND_SCATTER=`grep "$OUTDATFILE" vast_lightcurve_statistics.log | awk '{print $1" "$2}'`
   MEDIAN_MAG=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $1}'`
   SCATTER=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $2}'`
   COMBINED_ERROR=`echo "$SCATTER $C" | awk '{print sqrt($1*$1+$2*$2)}'`
   # Write output
   echo "$MEDIAN_MAG  $B  $COMBINED_ERROR"
  done | sort -n > calib.txt
 ;;
 "I"|"Ic")
#  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{print $8" "$23" "sqrt($9*$9+$24*$24)}' | while read A B C ;do if [ ! -z $C ];then  if [ "$B" != "0.000" ];then echo "$A  $B  $C" ;fi  ;fi ;done | sort -n > calib.txt
  cat $UCAC5_REFERENCE_IMAGE_MATCH_FILE | awk '{printf "out%05d.dat %f %f %f \n", $1, $8,$23,$24}' | while read OUTDATFILE A B C ;do 
   if [ -z $C ];then
    continue
   fi
   if [ "$B" == "0.000000" ];then
    continue
   fi
   # Check if this star is constant
   grep --quiet "$OUTDATFILE" vast_list_of_likely_constant_stars.log
   if [ $? -ne 0 ];then
    continue
   fi
   # Replace the magnitude and error measured at this image with the median mag and scatter from all images
   MEDIAN_MAG_AND_SCATTER=`grep "$OUTDATFILE" vast_lightcurve_statistics.log | awk '{print $1" "$2}'`
   MEDIAN_MAG=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $1}'`
   SCATTER=`echo $MEDIAN_MAG_AND_SCATTER | awk '{print $2}'`
   COMBINED_ERROR=`echo "$SCATTER $C" | awk '{print sqrt($1*$1+$2*$2)}'`
   # Write output
   echo "$MEDIAN_MAG  $B  $COMBINED_ERROR"
  done | sort -n > calib.txt
 ;;
 *) 
  echo "ERROR: unknown band $BAND"
  exit 1
 esac

fi

if [ -f calib.txt ];then
 echo "Saving a copy of calib.txt in calib.txt_backup"
 cp calib.txt calib.txt_backup
fi
if [ -f calib.txt_param ];then
 echo "Saving a copy of calib.txt_param in calib.txt_param_backup"
 cp calib.txt_param calib.txt_param_backup
fi

if [ "$2" == "linear" ];then
 FIT_MAG_CALIB_RESULTING_PARARMETERS=`lib/fit_linear`
 if [ $? -ne 0 ];then
  echo "Error fitting the magnitude scale! :("
  exit 1
 fi
elif [ "$2" == "photocurve" ];then
 FIT_MAG_CALIB_RESULTING_PARARMETERS=`lib/fit_photocurve`
 if [ $? -ne 0 ];then
  echo "Error fitting the magnitude scale! :("
  exit 1
 fi
else
 FIT_MAG_CALIB_RESULTING_PARARMETERS=`lib/fit_mag_calib`
 if [ $? -ne 0 ];then
  echo "Error fitting the magnitude scale! :("
  exit 1
 fi
fi

if [ -z "$FIT_MAG_CALIB_RESULTING_PARARMETERS" ];then
 echo "Error in the output parameters of the magnitude scale fit! :("
 exit 1
fi
echo "Proceeding with the calibration..."
util/calibrate_magnitude_scale $FIT_MAG_CALIB_RESULTING_PARARMETERS
util/nopgplot.sh -q
echo "Magnitde calibration complete. :)"
