#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include "erl_nif.h"
#include "geohash.h"

#define MAXBUFLEN 1024
#define BOUNDARIES 4
#define NEIGHBORS 8

struct atoms
{
  ERL_NIF_TERM atom_true;
  ERL_NIF_TERM atom_false;
  ERL_NIF_TERM atom_error;

  ERL_NIF_TERM boundaries_atoms[BOUNDARIES];
  ERL_NIF_TERM neighbors_atoms[NEIGHBORS];
} ATOMS;

inline ERL_NIF_TERM make_atom(ErlNifEnv *env, const char *name)
{
  ERL_NIF_TERM ret;

  if (enif_make_existing_atom(env, name, &ret, ERL_NIF_LATIN1))
    return ret;

  return enif_make_atom(env, name);
}

inline double _round(double n, unsigned short l)
{
  double f = pow(10.0, l);
  double val = n * f;

  if (val < 0)
  {
    val = ceil(val - 0.5);
  }
  else
  {
    val = floor(val + 0.5);
  }

  return val / f;
}

inline ERL_NIF_TERM make_binary(ErlNifEnv *env, const char *value, size_t size)
{
  ERL_NIF_TERM term;
  unsigned char *bin = enif_make_new_binary(env, size, &term);
  memcpy(bin, value, size);
  return term;
}

inline static ERL_NIF_TERM make_error(ErlNifEnv *env, const char *error)
{
  return enif_make_tuple2(env,
                          ATOMS.atom_error,
                          make_binary(env, error, strlen(error)));
}

static int
load(ErlNifEnv *env, void **priv, ERL_NIF_TERM load_info)
{
  ATOMS.atom_true = make_atom(env, "true");
  ATOMS.atom_false = make_atom(env, "false");
  ATOMS.atom_error = make_atom(env, "error");

  ATOMS.boundaries_atoms[0] = make_atom(env, "max_lat");
  ATOMS.boundaries_atoms[1] = make_atom(env, "max_lon");
  ATOMS.boundaries_atoms[2] = make_atom(env, "min_lat");
  ATOMS.boundaries_atoms[3] = make_atom(env, "min_lon");

  ATOMS.neighbors_atoms[0] = make_atom(env, "n");
  ATOMS.neighbors_atoms[1] = make_atom(env, "s");
  ATOMS.neighbors_atoms[2] = make_atom(env, "e");
  ATOMS.neighbors_atoms[3] = make_atom(env, "w");
  ATOMS.neighbors_atoms[4] = make_atom(env, "ne");
  ATOMS.neighbors_atoms[5] = make_atom(env, "se");
  ATOMS.neighbors_atoms[6] = make_atom(env, "nw");
  ATOMS.neighbors_atoms[7] = make_atom(env, "sw");

  return 0;
}

static int
upgrade(ErlNifEnv *env, void **priv, void **old_priv, ERL_NIF_TERM load_info)
{
  return 0;
}

void unload(ErlNifEnv *env, void *priv)
{
  enif_free(priv);
  return;
}

/************************************************************************
 *
 *  Encode latitude and longitude as a geohash of length length
 *
 ***********************************************************************/

/*
Geohash.Nif.encode(1, 2, 3)
"s01"
*/
static ERL_NIF_TERM
encode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  double latitude, longitude;
  unsigned int length;

  if (argc != 3)
  {
    return enif_make_badarg(env);
  }

  int int_latitude;
  if (enif_get_int(env, argv[0], &int_latitude))
  {
    latitude = (double)int_latitude;
  }
  else if (!enif_get_double(env, argv[0], &latitude))
  {
    return enif_make_badarg(env);
  }

  int int_longitude;
  if (enif_get_int(env, argv[1], &int_longitude))
  {
    longitude = (double)int_longitude;
  }
  else if (!enif_get_double(env, argv[1], &longitude))
  {
    return enif_make_badarg(env);
  }

  if (!enif_get_uint(env, argv[2], &length))
  {
    return enif_make_badarg(env);
  }

  char *hash;
  hash = GEOHASH_encode(latitude, longitude, length);

  ERL_NIF_TERM ret = make_binary(env, hash, length);
  free(hash);

  return ret;
}

/************************************************************************
 *
 *  Decodes a geohash and returns the tuple {latitude, longitude}
 *
 ***********************************************************************/

/*
Geohash.Nif.decode("s01")
{0,7, 2.1}

*/
static ERL_NIF_TERM
decode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  if (argc != 1)
  {
    return enif_make_badarg(env);
  }

  ErlNifBinary hash;
  if (enif_inspect_binary(env, argv[0], &hash) < 1)
  {
    return enif_make_badarg(env);
  }

  GEOHASH_area *area;
  area = GEOHASH_decode((const char *)hash.data, hash.size);

  if (area == NULL)
  {
    return make_error(env, "invalid hash");
  }

  unsigned short lat_decimals = floor(2 - log10(area->latitude.max - area->latitude.min));
  double latitude = _round((area->latitude.min + area->latitude.max) / 2, lat_decimals);

  unsigned short lon_decimals = floor(2 - log10(area->longitude.max - area->longitude.min));
  double longitude = _round((area->longitude.min + area->longitude.max) / 2, lon_decimals);

  ERL_NIF_TERM ret = enif_make_tuple2(env,
                                      enif_make_double(env, latitude),
                                      enif_make_double(env, longitude));

  enif_release_binary(&hash);
  GEOHASH_free_area(area);

  return ret;
}

/************************************************************************
 *
 *  Returns the bounds of a geohash as map
 *  %{max_lat: ..., max_lon: ..., min_lat: ..., min_lon: ...}
 *
 ***********************************************************************/

/*
Geohash.Nif.bounds("s01")
%{max_lat: 1.40625, max_lon: 2.8125, min_lat: 0.0, min_lon: 1.40625}

*/
static ERL_NIF_TERM
bounds(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  if (argc != 1)
  {
    return enif_make_badarg(env);
  }

  ErlNifBinary hash;
  if (enif_inspect_binary(env, argv[0], &hash) < 1)
  {
    return enif_make_badarg(env);
  }

  GEOHASH_area *area;
  area = GEOHASH_decode((const char *)hash.data, hash.size);
  if (area == NULL)
  {
    enif_release_binary(&hash);
    return make_error(env, "invalid hash");
  }

  ERL_NIF_TERM ret;
  ERL_NIF_TERM values[BOUNDARIES] = {
      enif_make_double(env, area->latitude.max),
      enif_make_double(env, area->longitude.max),
      enif_make_double(env, area->latitude.min),
      enif_make_double(env, area->longitude.min),
  };

  enif_make_map_from_arrays(env, ATOMS.boundaries_atoms, values, BOUNDARIES, &ret);

  enif_release_binary(&hash);
  GEOHASH_free_area(area);

  return ret;
}

/************************************************************************
 *
 *  Returns the neighbors of a geohash as map
 *
 * %{
 *    "n" => ...,
 *    "s" => ...,
 *    "e" => ...,
 *    "w" => ...,
 *    "ne" => ...,
 *    "se" => ...,
 *    "nw" => ...,
 *    "sw" => ...
 * }
 *
 ***********************************************************************/

/*
Geohash.Nif.neighbors("6gkzwgjz")
%{
    "n" => "6gkzwgmb",
    "s" => "6gkzwgjy",
    "e" => "6gkzwgnp",
    "w" => "6gkzwgjx",
    "ne" => "6gkzwgq0",
    "se" => "6gkzwgnn",
    "nw" => "6gkzwgm8",
    "sw" => "6gkzwgjw"
}
*/
static ERL_NIF_TERM
neighbors(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  if (argc != 1)
  {
    return enif_make_badarg(env);
  }

  ErlNifBinary hash;
  if (enif_inspect_binary(env, argv[0], &hash) < 1)
  {
    return enif_make_badarg(env);
  }

  if (!GEOHASH_verify_hash((const char *)hash.data, hash.size))
  {
    return make_error(env, "invalid hash");
  }

  GEOHASH_neighbors *neighbors;
  neighbors = GEOHASH_get_neighbors((const char *)(const char *)hash.data, hash.size);
  assert(neighbors != NULL);

  ERL_NIF_TERM ret;

  ERL_NIF_TERM keys[NEIGHBORS] = {
      make_binary(env, "n", 1),
      make_binary(env, "s", 1),
      make_binary(env, "e", 1),
      make_binary(env, "w", 1),
      make_binary(env, "ne", 2),
      make_binary(env, "se", 2),
      make_binary(env, "nw", 2),
      make_binary(env, "sw", 2),
  };

  ERL_NIF_TERM values[NEIGHBORS] = {
      make_binary(env, neighbors->north, hash.size),
      make_binary(env, neighbors->south, hash.size),
      make_binary(env, neighbors->east, hash.size),
      make_binary(env, neighbors->west, hash.size),
      make_binary(env, neighbors->north_east, hash.size),
      make_binary(env, neighbors->south_east, hash.size),
      make_binary(env, neighbors->north_west, hash.size),
      make_binary(env, neighbors->south_west, hash.size),
  };

  enif_make_map_from_arrays(env, keys, values, NEIGHBORS, &ret);
  enif_release_binary(&hash);

  GEOHASH_free_neighbors(neighbors);

  return ret;
}

/************************************************************************
 *
 *  Returns the neighbors of a geohash as map
 *
 * %{
 *    n: ...,
 *    s: ...,
 *    e: ...,
 *    w: ...,
 *    ne: ...,
 *    se: ...,
 *    nw: ...,
 *    sw: ...
 * }
 *
 ***********************************************************************/

/*
Geohash.Nif.neighbors2("6gkzwgjz")
%{
    n: "6gkzwgmb",
    s: "6gkzwgjy",
    e: "6gkzwgnp",
    w: "6gkzwgjx",
    ne: "6gkzwgq0",
    se: "6gkzwgnn",
    nw: "6gkzwgm8",
    sw: "6gkzwgjw"
}
*/
static ERL_NIF_TERM
neighbors2(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  if (argc != 1)
  {
    return enif_make_badarg(env);
  }

  ErlNifBinary hash;
  if (enif_inspect_binary(env, argv[0], &hash) < 1)
  {
    return enif_make_badarg(env);
  }

  if (!GEOHASH_verify_hash((const char *)hash.data, hash.size))
  {
    return make_error(env, "invalid hash");
  }

  GEOHASH_neighbors *neighbors;
  neighbors = GEOHASH_get_neighbors((const char *)(const char *)hash.data, hash.size);
  assert(neighbors != NULL);

  ERL_NIF_TERM ret;

  ERL_NIF_TERM values[NEIGHBORS] = {
      make_binary(env, neighbors->north, hash.size),
      make_binary(env, neighbors->south, hash.size),
      make_binary(env, neighbors->east, hash.size),
      make_binary(env, neighbors->west, hash.size),
      make_binary(env, neighbors->north_east, hash.size),
      make_binary(env, neighbors->south_east, hash.size),
      make_binary(env, neighbors->north_west, hash.size),
      make_binary(env, neighbors->south_west, hash.size),
  };

  enif_make_map_from_arrays(env, ATOMS.neighbors_atoms, values, NEIGHBORS, &ret);
  enif_release_binary(&hash);

  GEOHASH_free_neighbors(neighbors);

  return ret;
}

/************************************************************************
 *
 *  Returns the adjacent geohash in ordinal direction ["n","s","e","w"]
 *
 ***********************************************************************/

/*
Geohash.Nif.adjacent("abx1","n")
"abx4"
*/
static ERL_NIF_TERM
adjacent(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  if (argc != 2)
  {
    return enif_make_badarg(env);
  }

  ErlNifBinary hash;
  if (enif_inspect_binary(env, argv[0], &hash) < 1)
  {
    return enif_make_badarg(env);
  }

  if (!GEOHASH_verify_hash((const char *)hash.data, hash.size))
  {
    return make_error(env, "invalid hash");
  }

  ErlNifBinary direction;
  if (enif_inspect_binary(env, argv[1], &direction) < 1)
  {
    return enif_make_badarg(env);
  }

  GEOHASH_direction dir;

  switch (*(const char *)direction.data)
  {
  case 'n':
  case 'N':
    dir = GEOHASH_NORTH;
    break;
  case 's':
  case 'S':
    dir = GEOHASH_SOUTH;
    break;
  case 'e':
  case 'E':
    dir = GEOHASH_EAST;
    break;
  case 'w':
  case 'W':
    dir = GEOHASH_WEST;
    break;
  default:
    return make_error(env, "invalid direction");
  }

  const char *adjacent;
  adjacent = (const char *)GEOHASH_get_adjacent((const char *)hash.data, hash.size, dir);

  assert(adjacent != NULL);

  ERL_NIF_TERM ret = make_binary(env, adjacent, hash.size);

  enif_release_binary(&hash);
  enif_release_binary(&direction);
  free((void *)adjacent);

  return ret;
}

/************************************************************************
 *
 *  Decodes a geohash to a bitstring of length `length`
 *
 ***********************************************************************/
/*
Geohash.Nif.decode_to_bits('ezs42', 25)
<<0b0110111111110000010000010::25>>
*/
static ERL_NIF_TERM
decode_to_bits(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  if (argc != 1)
  {
    return enif_make_badarg(env);
  }

  ErlNifBinary hash;
  if (enif_inspect_binary(env, argv[0], &hash) < 1)
  {
    return enif_make_badarg(env);
  }

  uint64_t bits;
  bits = GEOHASH_decode_to_bits((const char *)hash.data, hash.size);

  if (bits == 0)
  {
    return make_error(env, "invalid hash");
  }
  else
  {
    return enif_make_uint64(env, bits);
  }
}

/************************************************************************
 *
 * ErlNifFunc struct declaration
 *
 ***********************************************************************/

static ErlNifFunc nif_funcs[] =
    {
        {"encode", 3, encode},
        {"decode", 1, decode},
        {"decode_to_bits", 1, decode_to_bits},
        {"bounds", 1, bounds},
        {"neighbors", 1, neighbors},
        {"neighbors2", 1, neighbors2},
        {"adjacent", 2, adjacent}};

ERL_NIF_INIT(Elixir.Geohash.Nif, nif_funcs, &load, NULL, &upgrade, &unload);
