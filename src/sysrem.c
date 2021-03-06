/*

 This implementation of SysRem is obsolete!
 Please see src/sysrem2.c instead.

*/

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <gsl/gsl_statistics_float.h>
#include <gsl/gsl_sort_float.h>
#include <gsl/gsl_errno.h>

#include "vast_limits.h"

#include "lightcurve_io.h"

void change_number_of_sysrem_iterations_in_log_file() {
 FILE *logfilein;
 FILE *logfileout;
 int number_of_iterations= 0;
 char str[2048];
 logfilein= fopen( "vast_summary.log", "r" );
 if ( logfilein != NULL ) {
  logfileout= fopen( "vast_summary.log.tmp", "w" );
  if ( logfileout == NULL ) {
   fclose( logfilein );
   return;
  }
  while ( NULL != fgets( str, 2048, logfilein ) ) {
   if ( str[0] == 'N' && str[1] == 'u' && str[2] == 'm' && str[10] == 'S' && str[13] == 'R' ) {
    sscanf( str, "Number of SysRem iterations: %d", &number_of_iterations );
    sprintf( str, "Number of SysRem iterations: %d\n", number_of_iterations + 1 );
   }
   fputs( str, logfileout );
  }
  fclose( logfileout );
  fclose( logfilein );
  if ( 0 != system( "mv vast_summary.log.tmp vast_summary.log" ) ) {
   fprintf( stderr, "ERROR in  change_number_of_sysrem_iterations_in_log_file()\n" );
  }
 }
 return;
}

void write_fake_log_file( double *jd, int *Nobs ) {
 int i;
 FILE *logfile;
 fprintf( stderr, "Writing fake vast_image_details.log ... " );
 logfile= fopen( "vast_image_details.log", "w" );
 if ( logfile == NULL ) {
  fprintf( stderr, "ERROR: Couldn't open file vast_image_details.log\n" );
  exit( 1 );
 };
 for ( i= 0; i < ( *Nobs ); i++ )
  fprintf( logfile, "JD= %.5lf\n", jd[i] );
 fclose( logfile );
 fprintf( stderr, "done\n" );
 return;
}

void get_dates_from_lightcurve_files( double *jd, int *Nobs ) {
 DIR *dp;
 struct dirent *ep;
 FILE *lightcurvefile;
 double _jd, mag, merr, x, y, app;
 char string[FILENAME_LENGTH];
 int i, date_found;

 ( *Nobs )= 0;

 dp= opendir( "./" );
 if ( dp != NULL ) {
  fprintf( stderr, "Extracting list of Julian Days from lightcurves... " );
  //while( ep = readdir(dp) ){
  while ( ( ep= readdir( dp ) ) != NULL ) {
   if ( strlen( ep->d_name ) < 8 )
    continue; // make sure the filename is not too short for the following tests
   if ( ep->d_name[0] == 'o' && ep->d_name[1] == 'u' && ep->d_name[2] == 't' && ep->d_name[strlen( ep->d_name ) - 1] == 't' && ep->d_name[strlen( ep->d_name ) - 2] == 'a' && ep->d_name[strlen( ep->d_name ) - 3] == 'd' ) {
    lightcurvefile= fopen( ep->d_name, "r" );
    if ( NULL == lightcurvefile ) {
     fprintf( stderr, "ERROR: Can't open file %s\n", ep->d_name );
     exit( 1 );
    }
    //while(-1<fscanf(lightcurvefile,"%lf %lf %lf %lf %lf %lf %s",&_jd,&mag,&merr,&x,&y,&app,string)){
    while ( -1 < read_lightcurve_point( lightcurvefile, &_jd, &mag, &merr, &x, &y, &app, string, NULL ) ) {
     if ( _jd == 0.0 )
      continue; // if this line could not be parsed, try the next one
     date_found= 0;
     for ( i= 0; i < ( *Nobs ); i++ ) {
      if ( _jd == jd[i] ) {
       date_found= 1;
       break;
      }
     }
     if ( date_found == 0 ) {
      jd[( *Nobs )]= _jd;
      ( *Nobs )+= 1;
     }
    }
    fclose( lightcurvefile );
   }
  }
  (void)closedir( dp );
  fprintf( stderr, "done\n" );
 } else
  perror( "Couldn't open the directory\n" );

 /* Write a fake log file so we don't need to read all the lightcurves next time */
 write_fake_log_file( jd, Nobs );

 return;
}

void get_dates( double *jd, int *Nobs ) {
 FILE *vastlogfile;
 char str[MAX_LOG_STR_LENGTH];
 char jd_str[MAX_LOG_STR_LENGTH];
 unsigned int i, j, k;
 ( *Nobs )= 0;
 vastlogfile= fopen( "vast_image_details.log", "r" );
 if ( NULL == vastlogfile ) {
  fprintf( stderr, "WARNING: Can't open vast_image_details.log\n" );
  get_dates_from_lightcurve_files( jd, Nobs );
 } else {
  while ( NULL != fgets( str, MAX_LOG_STR_LENGTH, vastlogfile ) ) {
   for ( i= 0; i < strlen( str ) - 3; i++ )
    if ( str[i] == 'J' && str[i + 1] == 'D' && str[i + 2] == '=' ) {
     for ( j= i + 4, k= 0; str[j] != ' '; j++, k++ ) {
      jd_str[k]= str[j];
     }
     jd[( *Nobs )]= atof( jd_str );
     break;
    }
   ( *Nobs )+= 1;
  }
  fclose( vastlogfile );
  fprintf( stderr, "Total number of observations (from log file) %d\n", ( *Nobs ) );
  return;
 }
 fprintf( stderr, "Total number of observations %d\n", ( *Nobs ) );
 return;
}

//int main(int argc, char **argv){
int main() {
 FILE *lightcurvefile;
 FILE *outlightcurvefile;

 float **mag_err;
 float **r;

 float *c;
 float *a;

 float *old_c;
 float *old_a;

 double *jd;

 int i, j, iter;

 long k;

 double djd;
 //float dmag,dmerr,x,y,app;
 double dmag, dmerr, x, y, app;
 char string[FILENAME_LENGTH];

 char star_number_string[FILENAME_LENGTH];

 int Nobs;
 int Nstars;

 float *data;
 float mean, median, sigma, sum1, sum2;

 char system_command_str[1024 + MAX_STRING_LENGTH_IN_LIGHTCURVE_FILE]; // should be big enough to accomodate lightcurvefilename

 FILE *datafile;
 char lightcurvefilename[MAX_STRING_LENGTH_IN_LIGHTCURVE_FILE];

 int stop_iterations= 0;

 int *bad_stars;
 char **star_numbers;

 float tmpfloat; // for faster computation

 /* Protection against strange free() crashes */
 //setenv("MALLOC_CHECK_", "0", 1);

 /* If there is no input star list - make it */
 if ( 0 != system( "lib/select_sysrem_input_star_list" ) ) {
  fprintf( stderr, "ERROR running  lib/select_sysrem_input_star_list\n" );
  return 1;
 }

 /* Count stars we want to process */
 Nstars= 0;
 datafile= fopen( "sysrem_input_star_list.lst", "r" );
 if ( NULL == datafile ) {
  fprintf( stderr, "ERROR! Can't open file sysrem_input_star_list.lst\n" );
  exit( 1 );
 }
 while ( -1 < fscanf( datafile, "%f %f %f %f %s", &mean, &mean, &mean, &mean, lightcurvefilename ) )
  Nstars++;
 fclose( datafile );
 fprintf( stderr, "Number of stars in sysrem_input_star_list.lst %d\n", Nstars );
 if ( Nstars < 100 ) {
  fprintf( stderr, "Too few stars!\n" );
  exit( 1 );
 }

 bad_stars= malloc( Nstars * sizeof( int ) );
 if ( bad_stars == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for bad_stars\n" );
 };
 star_numbers= malloc( Nstars * sizeof( char * ) );
 if ( star_numbers == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for star_numbers\n" );
 };
 //for(i=0;i<Nstars;i++){
 for ( i= Nstars; i--; ) {
  star_numbers[i]= malloc( OUTFILENAME_LENGTH * sizeof( char ) );
  if ( star_numbers[i] == NULL ) {
   fprintf( stderr, "ERROR: Couldn't allocate memory for star_numbers[i]\n" );
  };
  bad_stars[i]= 0;
 }

 /* Read the log file */
 jd= malloc( MAX_NUMBER_OF_OBSERVATIONS * sizeof( double ) );
 if ( jd == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for jd\n" );
 };
 get_dates( jd, &Nobs );
 if ( Nobs <= 0 ) {
  fprintf( stderr, "ERROR: Trying allocate zero or negative bytes amount(Nobs<=0)\n" );
  exit( 1 );
 };

 /* Allocate memory */
 mag_err= malloc( Nstars * sizeof( float * ) );
 if ( mag_err == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for mag_err\n" );
  exit( 1 );
 }
 r= malloc( Nstars * sizeof( float * ) );
 if ( r == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for r\n" );
  exit( 1 );
 }

 data= malloc( Nstars * Nobs * sizeof( float ) );
 if ( data == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for data\n" );
  exit( 1 );
 }

 //for(i=0;i<Nstars;i++){
 for ( i= Nstars; i--; ) {
  mag_err[i]= malloc( Nobs * sizeof( float ) );
  r[i]= malloc( Nobs * sizeof( float ) ); // is it correct ?????
  if ( r[i] == NULL || mag_err[i] == NULL ) {
   fprintf( stderr, "ERROR: Couldn't allocate memory for r[i] or mag_err[i] (i=%d)\n", i );
   exit( 1 );
  }
 }

 //for(i=0;i<Nstars;i++)
 for ( i= Nstars; i--; )
  //for(j=0;j<Nobs;j++)
  for ( j= Nobs; j--; )
   r[i][j]= 0.0;

 c= malloc( Nstars * sizeof( float ) );
 if ( c == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for c\n" );
  exit( 1 );
 }
 a= malloc( Nobs * sizeof( float ) );
 if ( a == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for a\n" );
  exit( 1 );
 }

 old_c= malloc( Nstars * sizeof( float ) );
 if ( old_c == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for old_c\n" );
  exit( 1 );
 }
 old_a= malloc( Nobs * sizeof( float ) );
 if ( old_a == NULL ) {
  fprintf( stderr, "ERROR: Couldn't allocate memory for old_a\n" );
  exit( 1 );
 }

 //for(i=0;i<Nstars;i++){
 for ( i= Nstars; i--; ) {
  c[i]= 0.1;
  old_c[i]= 0.1;
 }

 //for(i=0;i<Nobs;i++){
 for ( i= Nobs; i--; ) {
  a[i]= 1.0;
  old_a[i]= 1.0;
 }

 /* Read the data */
 i= j= 0;
 datafile= fopen( "sysrem_input_star_list.lst", "r" );
 if ( NULL == datafile ) {
  fprintf( stderr, "ERROR! Can't open file sysrem_input_star_list.lst\n" );
  exit( 1 );
 }
 while ( -1 < fscanf( datafile, "%f %f %f %f %s", &mean, &mean, &mean, &mean, lightcurvefilename ) ) {
  // Get star number from the lightcurve file name
  for ( k= 3; k < (long)strlen( lightcurvefilename ); k++ ) {
   star_number_string[k - 3]= lightcurvefilename[k];
   if ( lightcurvefilename[k] == '.' ) {
    star_number_string[k - 3]= '\0';
    break;
   }
  }
  strcpy( star_numbers[i], star_number_string );
  lightcurvefile= fopen( lightcurvefilename, "r" );
  if ( NULL == lightcurvefile ) {
   fprintf( stderr, "ERROR: Can't read file %s\n", lightcurvefilename );
   exit( 1 );
  }
  //while(-1<fscanf(lightcurvefile,"%lf %f %f %f %f %f %s",&djd,&dmag,&dmerr,&x,&y,&app,string)){
  while ( -1 < read_lightcurve_point( lightcurvefile, &djd, &dmag, &dmerr, &x, &y, &app, string, NULL ) ) {
   if ( djd == 0.0 )
    continue; // if this line could not be parsed, try the next one
   // Find which j is corresponding to the current JD
   for ( k= 0; k < Nobs; k++ ) {
    if ( fabs( jd[k] - djd ) <= 0.00001 ) { // 0.8 sec
                                            //if( fabs(jd[k]-djd)<=0.0001 ){ // 8 sec
     j= k;
     r[i][j]= dmag;
     mag_err[i][j]= dmerr;
     break;
    }
   }
  }
  fclose( lightcurvefile );
  i++;
 }
 fclose( datafile );

 /* Do the actual work */
 fprintf( stderr, "Computing average magnitudes... " );

 /* For each star compute median magnitude and subtract it from all measurements */
 //for(i=0;i<Nstars;i++){
 for ( i= Nstars; i--; ) {
  k= 0;
  //for(j=0;j<Nobs;j++){
  for ( j= Nobs; j--; ) {
   if ( r[i][j] != 0.0 ) {
    data[k]= r[i][j];
    k++;
   }
  }
  gsl_sort_float( data, 1, k );
  median= gsl_stats_float_median_from_sorted_data( data, 1, k );
  //for(j=0;j<Nobs;j++){
  for ( j= Nobs; j--; ) {
   if ( r[i][j] != 0.0 ) {
    r[i][j]= r[i][j] - median;
   }
  }
 }
 fprintf( stderr, "done\nStarting iterations...\n" );

 /* Iterative search for best c[i] and a[j] */
 for ( iter= 0; iter < NUMBER_OF_Ai_Ci_ITERATIONS; iter++ ) {
  fprintf( stderr, "\riteration %4d", iter + 1 );
  //for(i=0;i<Nstars;i++){
  for ( i= Nstars; i--; ) {
   sum1= sum2= 0.0;
   for ( j= 0; j < Nobs; j++ ) {
    //for(j=Nobs;j--;){
    if ( r[i][j] != 0.0 ) {
     tmpfloat= 1.0f / ( mag_err[i][j] * mag_err[i][j] );
     sum1+= r[i][j] * a[j] * tmpfloat; // /(mag_err[i][j]*mag_err[i][j]);
     sum2+= a[j] * a[j] * tmpfloat;    // /(mag_err[i][j]*mag_err[i][j]);
    }
   }
   if ( sum1 != 0.0 && sum2 != 0.0 ) {
    old_c[i]= c[i];
    c[i]= sum1 / sum2;
   }
  }

  //for(j=0;j<Nobs;j++){
  for ( j= Nobs; j--; ) {
   sum1= sum2= 0.0;
   //for(i=0;i<Nstars;i++){
   for ( i= Nstars; i--; ) {
    if ( r[i][j] != 0.0 ) {
     tmpfloat= 1.0f / ( mag_err[i][j] * mag_err[i][j] );
     sum1+= r[i][j] * c[i] * tmpfloat; // /(mag_err[i][j]*mag_err[i][j]);
     sum2+= c[i] * c[i] * tmpfloat;    // /(mag_err[i][j]*mag_err[i][j]);
    }
   }
   if ( sum1 != 0.0 && sum2 != 0.0 ) {
    old_a[j]= a[j];
    a[j]= sum1 / sum2;
   }
  }

  /* Should we stop now? */
  stop_iterations= 1;
  //for(i=0;i<Nstars;i++){
  for ( i= Nstars; i--; ) {
   if ( fabsf( c[i] - old_c[i] ) > Ai_Ci_DIFFERENCE_TO_STOP_ITERATIONS ) {
    stop_iterations= 0;
    break;
   }
  }

  if ( stop_iterations == 1 ) {
   //for(j=0;j<Nobs;j++){
   for ( j= Nobs; j--; ) {
    if ( fabsf( a[j] - old_a[j] ) > Ai_Ci_DIFFERENCE_TO_STOP_ITERATIONS ) {
     stop_iterations= 0;
     break;
    }
   }
  }
  if ( stop_iterations == 1 )
   break; // Stop iteretions if they make no difference

 } // Iterative search for best c[i] and a[j]

 fprintf( stderr, "\nRemoving outliers... " );
 /* Check for obviously bad corrections */
 //for(j=0;j<Nobs;j++){
 for ( j= Nobs; j--; ) {
  if ( 0 != isnan( a[j] ) ) {
   fprintf( stderr, "a[%d]= %f\n", j, a[j] );
   exit( 1 );
  }
  k= 0;
  //for(i=0;i<Nstars;i++){
  for ( i= Nstars; i--; ) {
   if ( r[i][j] != 0.0 ) {
    data[k]= a[j] * c[i];
    k++;
   }
  }
  mean= gsl_stats_float_mean( data, 1, k );
  sigma= gsl_stats_float_sd_m( data, 1, k, mean );
  gsl_sort_float( data, 1, k );
  median= gsl_stats_float_median_from_sorted_data( data, 1, k );

  //for(i=0;i<Nstars;i++){
  for ( i= Nstars; i--; ) {
   if ( r[i][j] != 0.0 && fabsf( a[j] * c[i] - median ) > 3 * sigma )
    bad_stars[i]= 1;
  }
 }
 fprintf( stderr, "done\n" );

 /* 2nd pass */

 for ( iter= 0; iter < NUMBER_OF_Ai_Ci_ITERATIONS; iter++ ) {
  fprintf( stderr, "\riteration %4d", iter + 1 );
  //for(i=0;i<Nstars;i++){
  for ( i= Nstars; i--; ) {
   sum1= sum2= 0.0;
   if ( bad_stars[i] == 0 ) {
    //for(j=0;j<Nobs;j++){
    for ( j= Nobs; j--; ) {
     if ( r[i][j] != 0.0 ) {
      tmpfloat= 1.0f / ( mag_err[i][j] * mag_err[i][j] );
      sum1+= r[i][j] * a[j] * tmpfloat; // /(mag_err[i][j]*mag_err[i][j]);
      sum2+= a[j] * a[j] * tmpfloat;    // /(mag_err[i][j]*mag_err[i][j]);
     }
    }
   }
   if ( sum1 != 0.0 && sum2 != 0.0 ) {
    old_c[i]= c[i];
    c[i]= sum1 / sum2;
   }
  }

  //for(j=0;j<Nobs;j++){
  for ( j= Nobs; j--; ) {
   sum1= sum2= 0.0;
   //for(i=0;i<Nstars;i++){
   for ( i= Nstars; i--; ) {
    if ( bad_stars[i] == 0 ) {
     if ( r[i][j] != 0.0 ) {
      tmpfloat= 1.0f / ( mag_err[i][j] * mag_err[i][j] );
      sum1+= r[i][j] * c[i] * tmpfloat; // /(mag_err[i][j]*mag_err[i][j]);
      sum2+= c[i] * c[i] * tmpfloat;    // /(mag_err[i][j]*mag_err[i][j]);
     }
    }
   }
   if ( sum1 != 0.0 && sum2 != 0.0 ) {
    old_a[j]= a[j];
    a[j]= sum1 / sum2;
   }
  }

  /* Should we stop now? */
  stop_iterations= 1;
  //for(i=0;i<Nstars;i++){
  for ( i= Nstars; i--; ) {
   if ( fabsf( c[i] - old_c[i] ) > Ai_Ci_DIFFERENCE_TO_STOP_ITERATIONS ) {
    stop_iterations= 0;
    break;
   }
  }

  if ( stop_iterations == 1 ) {
   //for(j=0;j<Nobs;j++){
   for ( j= Nobs; j--; ) {
    if ( fabsf( a[j] - old_a[j] ) > Ai_Ci_DIFFERENCE_TO_STOP_ITERATIONS ) {
     stop_iterations= 0;
     break;
    }
   }
  }

  if ( stop_iterations == 1 )
   break; // Stop iteretions if they make no difference

 } // Iterative search for best c[i] and a[j]

 fprintf( stderr, "\nRemoving outliers... " );
 /* 2nd check for obviously bad corrections */
 for ( j= 0; j < Nobs; j++ ) {
  if ( 0 != isnan( a[j] ) ) {
   fprintf( stderr, "a[%d]= %f\n", j, a[j] );
   exit( 1 );
  }
  k= 0;
  for ( i= 0; i < Nstars; i++ ) {
   if ( bad_stars[i] == 0 )
    if ( r[i][j] != 0.0 ) {
     data[k]= a[j] * c[i];
     k++;
    }
  }
  mean= gsl_stats_float_mean( data, 1, k );
  sigma= gsl_stats_float_sd_m( data, 1, k, mean );
  gsl_sort_float( data, 1, k );
  median= gsl_stats_float_median_from_sorted_data( data, 1, k );

  for ( i= 0; i < Nstars; i++ ) {
   //if( r[i][j]!=0.0 && fabsf(a[j]*c[i]-mean)>5*sigma )bad_stars[i]=1;
   if ( r[i][j] != 0.0 && fabsf( a[j] * c[i] - median ) > 3 * sigma )
    bad_stars[i]= 1;
   if ( r[i][j] != 0.0 && fabsf( a[j] * c[i] - median ) > 6 * sigma )
    bad_stars[i]= 2;
  }
 }
 fprintf( stderr, "done\n" );

 /* Apply corrections to lightcurves */
 fprintf( stderr, "Applying corrections... \n" );
 for ( i= 0; i < Nstars; i++ ) {
  sprintf( lightcurvefilename, "out%s.dat", star_numbers[i] );
  if ( bad_stars[i] == 0 ) {
   lightcurvefile= fopen( lightcurvefilename, "r" );
   if ( NULL == lightcurvefile ) {
    fprintf( stderr, "ERROR: Can't read file %s\n", lightcurvefilename );
    exit( 1 );
   }
   outlightcurvefile= fopen( "TMP.dat", "w" );
   if ( outlightcurvefile == NULL ) {
    fprintf( stderr, "ERROR: Can't open file TMP.dat\n" );
   };
   //while(-1<fscanf(lightcurvefile,"%lf %f %f %f %f %f %s",&djd,&dmag,&dmerr,&x,&y,&app,string)){
   while ( -1 < read_lightcurve_point( lightcurvefile, &djd, &dmag, &dmerr, &x, &y, &app, string, NULL ) ) {
    if ( djd == 0.0 )
     continue; // if this line could not be parsed, try the next one
    // Find which j is corresponding to the current JD
    for ( k= 0; k < Nobs; k++ ) {
     if ( fabs( jd[k] - djd ) <= 0.00001 ) { // 0.8 sec
                                             //if( fabs(jd[k]-djd)<=0.0001 ){ // 8 sec
      j= k;
      //fprintf(outlightcurvefile,"%.5lf %8.5f %.5f %8.3f %8.3f %4.lf %s\n",djd,dmag-c[i]*a[j],dmerr,x,y,app,string);
      write_lightcurve_point( outlightcurvefile, djd, (double)( dmag - c[i] * a[j] ), (double)dmerr, (double)x, (double)y, (double)app, string, NULL );
      break;
     }
    }
   }
   fclose( outlightcurvefile );
   fclose( lightcurvefile );
   sprintf( system_command_str, "mv -f TMP.dat %s", lightcurvefilename );
   if ( 0 != system( system_command_str ) ) {
    fprintf( stderr, "ERROR running  %s\n", system_command_str );
   }
  } else {
   fprintf( stderr, "Skip correction for %s", lightcurvefilename );
   if ( bad_stars[i] == 2 ) {
    fprintf( stderr, " removing it from sysrem_input_star_list.lst\n" );
    sprintf( system_command_str, "grep -v %s sysrem_input_star_list.lst > TMP.dat && mv -f TMP.dat sysrem_input_star_list.lst", lightcurvefilename );
    if ( 0 != system( system_command_str ) ) {
     fprintf( stderr, "ERROR running  %s\n", system_command_str );
    }
   } else {
    fprintf( stderr, "\n" );
   }
  }
 }
 fprintf( stderr, "done\n" );

 /* Print out some stats */
 k= 0;
 for ( i= 0; i < Nstars; i++ ) {
  if ( bad_stars[i] == 0 ) {
   for ( j= 0; j < Nobs; j++ ) {
    if ( r[i][j] != 0.0 ) {
     data[k]= fabsf( a[j] * c[i] );
     k++;
    }
   }
  }
 }
 mean= gsl_stats_float_mean( data, 1, k );
 sigma= gsl_stats_float_sd_m( data, 1, k, mean );
 gsl_sort_float( data, 1, k );
 median= gsl_stats_float_median_from_sorted_data( data, 1, k );
 fprintf( stderr, "Mean correction %.6f +/-%.6f  (median=%lf)\n", mean, sigma, median );

 //!!!
 free( bad_stars );
 for ( i= 0; i < Nstars; i++ ) {
  free( star_numbers[i] );
  free( mag_err[i] );
  free( r[i] );
 }
 free( star_numbers );
 free( old_c );
 free( old_a );

 /* Free memory */
 free( mag_err );
 free( r );
 free( a );
 free( c );
 free( jd );
 free( data );

 change_number_of_sysrem_iterations_in_log_file();

 // unsetenv("MALLOC_CHECK_");

 fprintf( stderr, "All done!  =)\n" );

 return 0;
}
