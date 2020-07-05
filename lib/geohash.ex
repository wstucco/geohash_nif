defmodule Geohash do
  @moduledoc ~S"""
  Drop in replacement fot the Elixir native [Geohash encode/decode library](https://hexdocs.pm/geohash/) implemented as a NIF

  ## Basic Usage


  ```elixir
  iex(1)> Geohash.encode(42.6, -5.6, 5)
  "ezs42"

  iex(1)> Geohash.encode(42.6, -5.6, 11)
  "ezs42e44yx9"

  iex(1)> Geohash.decode("ezs42")
  {42.605, -5.603}

  iex(1)> Geohash.neighbors("ezs42")
  %{
    "e" => "ezs43",
    "n" => "ezs48",
    "ne" => "ezs49",
    "nw" => "ezefx",
    "s" => "ezs40",
    "se" => "ezs41",
    "sw" => "ezefp",
    "w" => "ezefr"
  }

  iex(1)> Geohash.adjacent("ezs42","n")
  "ezs48"

  iex(1)> Geohash.bounds("u4pruydqqv")
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
  Calculate adjacent hashes for the 8 touching `neighbors/2`

  ## Options
  These options are specific to this function
  * `:keys` -- controls how keys in objects are decoded. Possible values are:
    * `:strings` (default) - decodes keys as binary strings (compatible mode)
    * `:atoms` - decodes keys as atoms (fast mode)

  ## Examples
  ```
  iex> Geohash.neighbors("6gkzwgjz")
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

  iex> Geohash.neighbors("6gkzwgjz", keys: :atoms)
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
  ```

  """
  def neighbors(hash, opts \\ [keys: :strings])

  def neighbors(hash, keys: :strings) do
    Nif.neighbors(hash)
  end

  def neighbors(hash, keys: :atoms) do
    Nif.neighbors2(hash)
  end

  @doc ~S"""
  Calculate `adjacent/2` geohash in ordinal direction `["n","s","e","w"]`.

  ## Examples
  ```
  iex> Geohash.adjacent("ezs42","n")
  "ezs48"
  ```
  """
  defdelegate adjacent(hash, direction), to: Nif
end
