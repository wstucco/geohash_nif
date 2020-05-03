#include <string.h>
#include <stdint.h>
#include <math.h>

#include "erl_nif.h"
#include "geohash.h"

ERL_NIF_TERM ok_atom;
ERL_NIF_TERM err_atom;

#define MAXBUFLEN 1024
#define BOUNDARIES 4
#define NEIGHBORS 8

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

inline ERL_NIF_TERM make_binary(ErlNifEnv *env, const char *value)
{
  ErlNifBinary output_binary;
  const int binary_len = strlen(value);
  enif_alloc_binary(binary_len, &output_binary);
  strcpy((char *)output_binary.data, value);
  return enif_make_binary(env, &output_binary);
}

inline static ERL_NIF_TERM make_error(ErlNifEnv *env, const char *error)
{
  return enif_make_tuple2(env,
                          err_atom,
                          make_binary(env, error));
}

static int
load(ErlNifEnv *env, void **priv, ERL_NIF_TERM load_info)
{
  enif_make_existing_atom(env, "ok", &ok_atom, ERL_NIF_LATIN1);
  enif_make_existing_atom(env, "error", &err_atom, ERL_NIF_LATIN1);

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

  ERL_NIF_TERM ret = make_binary(env, hash);
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

  char hash[MAXBUFLEN];
  (void)memset(&hash, '\0', MAXBUFLEN);

  if (enif_get_string(env, argv[0], hash, sizeof(hash), ERL_NIF_LATIN1) < 1)
  {
    return enif_make_badarg(env);
  }

  if (!GEOHASH_verify_hash(hash))
  {
    return make_error(env, "invalid hash");
  }

  GEOHASH_area *area;
  area = GEOHASH_decode(hash);

  unsigned short lat_decimals = floor(2 - log10(area->latitude.max - area->latitude.min));
  double latitude = _round((area->latitude.min + area->latitude.max) / 2, lat_decimals);

  unsigned short lon_decimals = floor(2 - log10(area->longitude.max - area->longitude.min));
  double longitude = _round((area->longitude.min + area->longitude.max) / 2, lon_decimals);

  ERL_NIF_TERM ret = enif_make_tuple2(env,
                                      enif_make_double(env, latitude),
                                      enif_make_double(env, longitude));

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

  char hash[MAXBUFLEN];
  (void)memset(&hash, '\0', MAXBUFLEN);

  if (enif_get_string(env, argv[0], hash, sizeof(hash), ERL_NIF_LATIN1) < 1)
  {
    return enif_make_badarg(env);
  }

  if (!GEOHASH_verify_hash(hash))
  {
    return make_error(env, "invalid hash");
  }

  GEOHASH_area *area;
  area = GEOHASH_decode(hash);

  ERL_NIF_TERM ret;
  ERL_NIF_TERM keys[BOUNDARIES] = {
      enif_make_atom(env, "max_lat"),
      enif_make_atom(env, "max_lon"),
      enif_make_atom(env, "min_lat"),
      enif_make_atom(env, "min_lon"),
  };
  ERL_NIF_TERM values[BOUNDARIES] = {
      enif_make_double(env, area->latitude.max),
      enif_make_double(env, area->longitude.max),
      enif_make_double(env, area->latitude.min),
      enif_make_double(env, area->longitude.min),
  };

  enif_make_map_from_arrays(env, keys, values, BOUNDARIES, &ret);

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

  char hash[MAXBUFLEN];
  (void)memset(&hash, '\0', MAXBUFLEN);

  if (enif_get_string(env, argv[0], hash, sizeof(hash), ERL_NIF_LATIN1) < 1)
  {
    return enif_make_badarg(env);
  }

  if (!GEOHASH_verify_hash(hash))
  {
    return make_error(env, "invalid hash");
  }

  GEOHASH_neighbors *neighbors;
  neighbors = GEOHASH_get_neighbors(hash);

  ERL_NIF_TERM ret;
  ERL_NIF_TERM keys[NEIGHBORS] = {
      make_binary(env, "n"),
      make_binary(env, "s"),
      make_binary(env, "e"),
      make_binary(env, "w"),
      make_binary(env, "ne"),
      make_binary(env, "se"),
      make_binary(env, "nw"),
      make_binary(env, "sw"),
  };
  ERL_NIF_TERM values[NEIGHBORS] = {
      make_binary(env, neighbors->north),
      make_binary(env, neighbors->south),
      make_binary(env, neighbors->east),
      make_binary(env, neighbors->west),
      make_binary(env, neighbors->north_east),
      make_binary(env, neighbors->south_east),
      make_binary(env, neighbors->north_west),
      make_binary(env, neighbors->south_west),
  };

  enif_make_map_from_arrays(env, keys, values, NEIGHBORS, &ret);

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

  char hash[MAXBUFLEN];
  (void)memset(&hash, '\0', MAXBUFLEN);
  if (enif_get_string(env, argv[0], hash, MAXBUFLEN, ERL_NIF_LATIN1) < 1)
  {
    return enif_make_badarg(env);
  }
  char direction[2];
  (void)memset(&direction, '\0', 2);
  if (enif_get_string(env, argv[1], direction, 2, ERL_NIF_LATIN1) < 1)
  {
    return enif_make_badarg(env);
  }

  if (!GEOHASH_verify_hash(hash))
  {
    return make_error(env, "invalid hash");
  }

  GEOHASH_direction dir;

  switch (*direction)
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

  char *adjacent;
  adjacent = GEOHASH_get_adjacent(hash, dir);
  ERL_NIF_TERM ret = make_binary(env, adjacent);
  free(adjacent);

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
  if (argc != 2)
  {
    return enif_make_badarg(env);
  }

  char hash[MAXBUFLEN];
  (void)memset(&hash, '\0', MAXBUFLEN);

  if (enif_get_string(env, argv[0], hash, sizeof(hash), ERL_NIF_LATIN1) < 1)
  {
    return enif_make_badarg(env);
  }

  unsigned int length;
  if (!enif_get_uint(env, argv[1], &length))
  {
    return enif_make_badarg(env);
  }

  uint64_t bits;
  bits = GEOHASH_decode_to_bits(hash, length);
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
        {"decode_to_bits", 2, decode_to_bits},
        {"bounds", 1, bounds},
        {"neighbors", 1, neighbors},
        {"adjacent", 2, adjacent},
};

ERL_NIF_INIT(Elixir.Geohash.Nif, nif_funcs, &load, NULL, &upgrade, &unload);
