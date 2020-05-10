defmodule Geohash do
  @moduledoc """
  Geohash
  """

  alias Geohash.Nif

  defdelegate encode(latitude, longitude, precision \\ 11), to: Nif

  defdelegate decode(hash), to: Nif

  def decode_to_bits(hash) do
    bits = Nif.decode_to_bits(hash)

    bit_size = round(:math.log2(bits) + 1)
    <<bits::size(bit_size)>>
  end

  defdelegate bounds(hash), to: Nif

  defdelegate neighbors(hash), to: Nif

  defdelegate adjacent(hash, direction), to: Nif
end
