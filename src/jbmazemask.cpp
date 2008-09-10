/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBMazeMask
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 * ---------------------------------------------------------------------- */

#include <fstream>
#include <stdio.h>
#include <string.h>

#include "jbmazemask.h"

JBMazeMask::JBMazeMask( int width, int height ) {
  int i;

  m_width  = width;
  m_height = height;

  m_mask = new char*[ m_width ];
  for( i = 0; i < m_width; i++ ) {
    m_mask[ i ] = new char[ m_height ];
    memset( m_mask[ i ], 1, m_height );
  }
}


JBMazeMask::JBMazeMask( char* filename ) {
  std::ifstream in( filename );
  char     line[ 1024 ];
  int      i;
  int      j;

  if( !in ) {
    m_width = 0;
    m_height = 0;
    m_mask = 0;
  }

  /* read the first line to get the width and height */

  in >> line;
  sscanf( line, "%d,%d", &m_width, &m_height );
  
  /* allocate and initialize the mask array */

  m_mask = new char*[ m_width ];
  for( i = 0; i < m_width; i++ ) {
    m_mask[ i ] = new char[ m_height ];
    memset( m_mask[ i ], 0, m_height );
  }

  /* loop until we hit the end of the file, or until we have read as many
   * lines as the mask is high. */

  j = 0;
  while( !in.eof() ) {
    if( j >= m_height ) {
      break;
    }

    /* read the next line and parse it */
    in >> line;

    for( i = 0; i < m_width; i++ ) {
      if( line[ i ] == 0 ) {
        break;
      }

      /* if the current char is a '1' then the mask is valid at that point */
      m_mask[ i ][ j ] = ( line[i] == '1' ? 1 : 0 );
    }

    j++;
  }
}


JBMazeMask::JBMazeMask( JBMazeMask& master ) {
  int i;

  m_width = master.m_width;
  m_height = master.m_height;

  m_mask = new char*[ m_width ];
  for( i = 0; i < m_width; i++ ) {
    m_mask[ i ] = new char[ m_height ];
    memcpy( m_mask[ i ], master.m_mask[ i ], m_height );
  }
}


JBMazeMask::~JBMazeMask() {
  int i;

  if( m_mask == 0 ) {
    return;
  }

  for( i = 0; i < m_width; i++ ) {
    delete[] m_mask[i];
  }
  delete m_mask;

  m_mask = 0;
  m_width = m_height = 0;
}
