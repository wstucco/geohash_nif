defmodule Geohash do
  @moduledoc """
  Geohash
  """

  alias Geohash.Nif

  defdelegate encode(latitude, longitude, precision \\ 11), to: Nif

  def decode(hash) when is_binary(hash) do
    hash
    |> to_charlist()
    |> Nif.decode()
  end

  defdelegate decode(hash), to: Nif

  def decode_to_bits(hash) when is_binary(hash) do
    hash
    |> to_charlist()
    |> __MODULE__.decode_to_bits()
  end

  def decode_to_bits(hash) do
    bits = Nif.decode_to_bits(hash)

    bit_size = round(:math.log2(bits) + 1)
    <<bits::size(bit_size)>>
  end

  def bounds(hash) when is_binary(hash) do
    hash
    |> to_charlist()
    |> Nif.bounds()
  end

  defdelegate bounds(hash), to: Nif

  def neighbors(hash) when is_binary(hash) do
    hash
    |> to_charlist()
    |> Nif.neighbors()
  end

  defdelegate neighbors(hash), to: Nif

  def adjacent(hash, direction) when is_binary(hash) or is_binary(direction) do
    hash = to_charlist(hash)
    direction = to_charlist(direction)
    Nif.adjacent(hash, direction)
  end

  defdelegate adjacent(hash, direction), to: Nif
end
