/*
   BLAKE2 reference source code package - b2sum tool
  
   Copyright 2012, Samuel Neves <sneves@dei.uc.pt>.  You may use this under the
   terms of the CC0, the OpenSSL Licence, or the Apache Public License 2.0, at
   your option.  The terms of these licenses can be found at:
  
   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
   - OpenSSL license   : https://www.openssl.org/source/license.html
   - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0
  
   More information about the BLAKE2 hash function can be found at
   https://blake2.net.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#include "blake2.h"


int blake2b_stream( FILE *stream1, FILE *stream2, FILE *stream3, FILE *stream4, void *resstream, void *resstream2, void *resstream3, void *resstream4, size_t outbytes )
{
  int ret = -1;
  size_t sum, n1, n2, n3, n4, length;
  blake2b_state S[1];
  static const size_t buffer_length = 32768;
  uint8_t *buffer = ( uint8_t * )malloc( 4 * buffer_length );

  if( !buffer ) return -1;
	
  blake2b_init( S, outbytes );

  while( 1 )
  {
    sum = length = 0;

    while( 1 )
    {
      //Reading first file and storing in the buffer
      n1 = fread( buffer + sum, 1, buffer_length - sum, stream1 );
      length += n1;
      
      //Reading second file and storing in the buffer
      n2 = fread( buffer + sum + length, 1, buffer_length - sum, stream2 );
      length += n2;
      
      //Reading third file and storing in the buffer
      n3 = fread( buffer + sum + length, 1, buffer_length - sum, stream3 );
      length += n3;
      
      //Reading fourth file and storing in the buffer
      n4 = fread( buffer + sum + length, 1, buffer_length - sum, stream4 );
      length += n4;
      
      
      sum += n1;
      
      length = length/4; //get files length means
      
      //checking if all inputs have the same length
      if(length != n1)
      	return -1;
      

      if( buffer_length == sum )
        break;
      if( 0 == n1 || 0 == n2 || 0 == n3 || 0 == n4 )
      {
        if( ferror( stream1 ) )
          goto cleanup_buffer;

        goto final_process;
      }

      if( feof( stream1 ) || feof( stream2 ) || feof( stream3 ) || feof( stream4 ) )
        goto final_process;
    }

    blake2b_update( S, buffer, sum );
  }

final_process:;

  if( sum > 0 ) 
  	blake2b_update( S, buffer, sum );

  blake2b_final( S, resstream, resstream2, resstream3, resstream4, outbytes );
  ret = 0;

cleanup_buffer:
  free( buffer );
  return ret;
}

int main( int argc, char **argv )
{
  unsigned long maxbytes = BLAKE2B_OUTBYTES;
  unsigned long outbytes = 0;
  unsigned char hash1[BLAKE2B_OUTBYTES] = {0};
  unsigned char hash2[BLAKE2B_OUTBYTES] = {0};
  unsigned char hash3[BLAKE2B_OUTBYTES] = {0};
  unsigned char hash4[BLAKE2B_OUTBYTES] = {0};
  int c;
  opterr = 1;

  if( outbytes == 0 )
    outbytes = maxbytes;

  if( optind == argc )
    argv[argc++] = (char *) "-";

  for( int i = 1; i < argc; i+=4 )
  {
    FILE *f1 = NULL;
    FILE *f2 = NULL;
    FILE *f3 = NULL;
    FILE *f4 = NULL;
    
    if( argv[i][0] == '-' && argv[i][1] == '\0' )
      f1 = stdin;
    else
      f1 = fopen( argv[i], "rb" );
      
    if( argv[i+1][0] == '-' && argv[i+1][1] == '\0' )
      f2 = stdin;
    else
      f2 = fopen( argv[i+1], "rb" );
    
    if( argv[i+2][0] == '-' && argv[i+2][1] == '\0' )
      f3 = stdin;
    else
      f3 = fopen( argv[i+2], "rb" );
      
    if( argv[i+3][0] == '-' && argv[i+3][1] == '\0' )
      f4 = stdin;
    else
      f4 = fopen( argv[i+3], "rb" );

    if( !f1 )
    {
      fprintf( stderr, "Could not open `%s': %s\n", argv[i], strerror( errno ) );
      continue;
    }
    if( !f2 )
    {
      fprintf( stderr, "Could not open `%s': %s\n", argv[i+1], strerror( errno ) );
      continue;
    }
    if( !f3 )
    {
      fprintf( stderr, "Could not open `%s': %s\n", argv[i+2], strerror( errno ) );
      continue;
    }
    if( !f4 )
    {
      fprintf( stderr, "Could not open `%s': %s\n", argv[i+3], strerror( errno ) );
      continue;
    }

    if( blake2b_stream( f1, f2, f3, f4, hash1, hash2, hash3, hash4, outbytes ) < 0 )
    {
      fprintf( stderr, "Failed to hash!'\n" );
    }
    else
    {
      for( size_t j = 0; j < outbytes; ++j )
        printf( "%02x", hash1[j] );
     
      printf( "  %s\n", argv[i] );
      
      for( size_t j = 0; j < outbytes; ++j )
        printf( "%02x", hash2[j] );

      printf( "  %s\n", argv[i+1] );
        
      for( size_t j = 0; j < outbytes; ++j )
        printf( "%02x", hash3[j] );

      printf( "  %s\n", argv[i+2] );
        
      for( size_t j = 0; j < outbytes; ++j )
        printf( "%02x", hash4[j] );

      printf( "  %s\n", argv[i+3] );
    }

    if( f1 != stdin ) fclose( f1 );
    if( f2 != stdin ) fclose( f2 );
    if( f3 != stdin ) fclose( f3 );
    if( f4 != stdin ) fclose( f4 );
  }

  return 0;
}

