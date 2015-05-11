#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
  
#define MAP_SIZE           (4096)
#define MAP_MASK           (MAP_SIZE-1)


int main( int argc, char *argv[] ) 
{
  int fd;

  if( argc < 2 ) {
    printf( "Usage:\n" );
    printf( "%s byte_addr [write_data]\n", argv[ 0 ] );
    exit( -1 );
  }

  // /dev/mem is a character device file that is an image of the memory.
  fd = open( "/dev/mem", O_RDWR | O_SYNC );
  if( fd < 0 ) {
    perror( "open" );
    exit( -1 ); 
  }

  void *map_page_addr, *map_byte_addr; 
  off_t byte_addr;
  
  byte_addr = strtoul( argv[ 1 ], NULL, 0 );

  // We mmap /dev/mem file in our address space and get page address.
  map_page_addr = mmap( 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, byte_addr & ~MAP_MASK );
  if( map_page_addr == MAP_FAILED ) {
    perror( "mmap" );
    exit( -1 ); 
  }

  // Calculate word address (it is byte address)
  map_byte_addr = map_page_addr + (byte_addr & MAP_MASK);

  uint32_t data;

  // If argc == 3, then we perform write operation. Instead read operation.
  if( argc > 2 ) {
    data = strtoul( argv[ 2 ], NULL, 0 );
    *( ( uint32_t *) map_byte_addr ) = data;
  } else {
    data = *( ( uint32_t *) map_byte_addr );
    printf( "data = 0x%08x\n", data );
  }

  // Delete maping.
  if( munmap( map_page_addr, MAP_SIZE ) ) {
    perror( "munmap" );
    exit( -1 ); 
  }

  close( fd );
  return 0;
}
