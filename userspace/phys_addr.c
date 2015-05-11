#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// File with mapping virtual address space to physical pages.
#define PAGE_FILE "/proc/self/pagemap"

// Information of page take 8 bytes
#define PAGE_INFO_SIZE 8

// Page Frame Number (PFN) located in low 55 bits
// See: 
//   man proc
//   https://www.kernel.org/doc/Documentation/vm/pagemap.txt
#define GET_PFN(val) (val & 0x7FFFFFFFFFFFFFLU)



// Allocate memory and lock it into RAM
void *alloc_buffer( int size ) 
{
  int err;
  int *buf;

  // Allocate memory with alignment to page size
  if( ( err = posix_memalign( (void **) &buf, getpagesize( ), size ) ) ) {
    fprintf( stderr, "posix_memalign: %s (%u)\n", strerror( err ), err );
    exit( -1 );
  }

  memset( buf, 0, size );

  // Lock memory in RAM
  if( mlock( buf, size ) ) {
    perror( "mlock" ) ;
    exit( -1 );
  }

  return buf;
}


void free_buffer( void *buf, int size )
{
    if( munlock( buf, size ) ) {
      perror( "munlock" ) ;
    }

    free( buf );
}



uint64_t get_phys_addr( void *virt_addr )
{
  int fd;
  off_t offset;
  uint64_t page_info;
  uint64_t phys_addr;

  fd = open( PAGE_FILE, O_RDWR ); 
  if( fd < 0 ) {
    perror( "open" );
    exit( -1 );
  }
 
  // Calc offset for interesting page
  offset = ( unsigned long ) virt_addr / getpagesize( ) * PAGE_INFO_SIZE;
   
  if( lseek( fd, offset, SEEK_SET ) < 0 ) {
    perror( "lseek" );
    exit( -1 );
  }

  // Get page information 
  if( read( fd, &page_info, sizeof( page_info ) ) != sizeof( page_info ) ) {
    perror( "read" );
  }


  // Get physical address
  phys_addr = GET_PFN( page_info ) * getpagesize( );


  return phys_addr;
}




int main( int argc, char *argv[] )
{
  int i;
  uint64_t *buf;
  unsigned int offset;
  uint64_t phys_addr;

  // We must allocate only one page, because contiguous 
  // pages in virtual space are not consecutive in physical memory
  int size = getpagesize( );

  buf = alloc_buffer( size );

  // Paranoid check that offset is zero :)
  offset = ( unsigned long ) buf % getpagesize( );
  printf( "offset = %u\n", offset );

  // Get buffer physical address
  phys_addr = get_phys_addr( buf );
  
  printf( "phys_addr = 0x%" PRIx64 "\n", phys_addr );

  // Pause
  getchar( );


  // Print data from buffer.
  // It must consist data, which we write from FPGA via fpga2sdram interface
  for( i = 0; i < ( size / sizeof( uint64_t ) ); i++ ) {
    printf( "%u: 0x%" PRIx64 "\n", i, buf[ i ] );
  }

  free_buffer( buf, size );

  exit( 0 );
}

