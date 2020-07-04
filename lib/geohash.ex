defmodule Geohash do
  @moduledoc ~S"""
  Geohash encode/decode library
  ## Usage
  - Encode coordinates with `Geohash.encode(lat, lon, precision \\ 11)`

  ```
  Geohash.encode(42.6, -5.6, 5)
  "ezs42"
  ```

  - Decode coordinates with `Geohash.decode(geohash)`

  ```
  Geohash.decode("ezs42")
  {42.605, -5.603}
  ```

  - Find neighbors

  ```
  Geohash.neighbors("ezs42")
  %{
    e: "ezs43",
    n: "ezs48",
    ne: "ezs49",
    nw: "ezefx",
    s: "ezs40",
    se: "ezs41",
    sw: "ezefp",
    w: "ezefr"
  }
  ```

  - Find adjacent

  ```
  Geohash.adjacent("ezs42","n")
  "ezs48"

  ```

  - Get bounds

  ```
  Geohash.bounds("u4pruydqqv")
  %{
    max_lat: 57.649115324020386,
    max_lon: 10.407443046569824,
    min_lat: 57.649109959602356,
    min_lon: 10.407432317733765
  }
  ```

  """

  alias Geohash.Nif

  @doc ~S"""
  Encodes given coordinates to a geohash of length `precision`
  ## Examples
  ```
  iex> Geohash.encode(42.6, -5.6, 5)
  "ezs42"
  ```
  """
  defdelegate encode(latitude, longitude, precision \\ 11), to: Nif

  @doc ~S"""
  Decodes given geohash to a coordinate pair
  ## Examples
  ```
  iex> {_lat, _lng} = Geohash.decode("ezs42")
  {42.605, -5.603}
  ```
  """
  defdelegate decode(hash), to: Nif

  @doc ~S"""
  Decodes given geohash to a bitstring
  ## Examples
  ```
  iex> Geohash.decode_to_bits("ezs42")
  <<0b0110111111110000010000010::25>>
  ```
  """
  def decode_to_bits(hash) do
    bits = Nif.decode_to_bits(hash)

    bit_size = round(:math.log2(bits) + 1)
    <<bits::size(bit_size)>>
  end

  @doc ~S"""
  Calculates bounds for a given geohash
  ## Examples
  ```
  iex> Geohash.bounds("u4pruydqqv")
  %{
    min_lon: 10.407432317733765,
    min_lat: 57.649109959602356,
    max_lon: 10.407443046569824,
    max_lat: 57.649115324020386
  }
  ```
  """
  defdelegate bounds(hash), to: Nif

  @doc ~S"""
  Calculate adjacent hashes for the 8 touching `neighbors/1`
  ## Examples
  ```
  iex>   Geohash.neighbors("ezs42")
  %{
    e: "ezs43",
    n: "ezs48",
    ne: "ezs49",
    nw: "ezefx",
    s: "ezs40",
    se: "ezs41",
    sw: "ezefp",
    w: "ezefr"
  }
  ```
  """
  defdelegate neighbors(hash), to: Nif

  @doc ~S"""
  Calculate `adjacent/2` geohash in ordinal direction `["n","s","e","w"]`.
  Deals with boundary cases when adjacent is not of the same prefix.
  ## Examples
  ```
  iex> Geohash.adjacent("ezs42","n")
  "ezs48"
  ```
  """
  defdelegate adjacent(hash, direction), to: Nif
end
