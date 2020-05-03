#ifndef _LIB_GEOHASH_H_
#define _LIB_GEOHASH_H_

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef enum
  {
    GEOHASH_NORTH = 0,
    GEOHASH_EAST,
    GEOHASH_WEST,
    GEOHASH_SOUTH
  } GEOHASH_direction;

  typedef struct
  {
    double max;
    double min;
  } GEOHASH_range;

  typedef struct
  {
    GEOHASH_range latitude;
    GEOHASH_range longitude;
  } GEOHASH_area;

  typedef struct
  {
    char *north;
    char *east;
    char *west;
    char *south;
    char *north_east;
    char *south_east;
    char *north_west;
    char *south_west;
  } GEOHASH_neighbors;

  bool GEOHASH_verify_hash(const char *hash);
  uint64_t GEOHASH_decode_to_bits(const char *hash);
  char *GEOHASH_encode(double latitude, double longitude, unsigned int hash_length);
  GEOHASH_area *GEOHASH_decode(const char *hash);
  GEOHASH_neighbors *GEOHASH_get_neighbors(const char *hash);
  void GEOHASH_free_neighbors(GEOHASH_neighbors *neighbors);
  char *GEOHASH_get_adjacent(const char *hash, GEOHASH_direction dir);
  void GEOHASH_free_area(GEOHASH_area *area);

#if defined(__cplusplus)
}
#endif

#endif
