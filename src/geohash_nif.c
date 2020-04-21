#include <string.h>
#include <math.h>

#include "erl_nif.h"
#include "geohash.h"

ERL_NIF_TERM ok_atom;
ERL_NIF_TERM err_atom;

#define MAXBUFLEN 1024

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

  ERL_NIF_TERM ret;
  unsigned char *ret_bin = enif_make_new_binary(env, length, &ret);
  strncpy(ret_bin, hash, length);
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

static ErlNifFunc nif_funcs[] =
    {
        {"encode", 3, encode},
        {"decode", 1, decode},
        // {"bounds", 1, bounds, ERL_NIF_DIRTY_JOB_CPU_BOUND},
        // {"neighbors", 1, neighbors, ERL_NIF_DIRTY_JOB_CPU_BOUND},
        // {"adjacent", 2, adjacent, ERL_NIF_DIRTY_JOB_CPU_BOUND},
};

ERL_NIF_INIT(Elixir.Geohash.Nif, nif_funcs, &load, NULL, &upgrade, &unload);
