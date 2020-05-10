defmodule Geohash.Nif do
  @moduledoc false
  @on_load :init

  def init do
    :erlang.load_nif(:code.priv_dir(:geohash_nif) ++ '/geohash', 0)
  end

  @doc false
  def encode(_latitude, _longitude, _length \\ 11)
  def encode(_latitude, _longitude, _length), do: :erlang.nif_error(:nif_not_loaded)
  def decode(hash) when is_binary(hash), do: :erlang.nif_error(:nif_not_loaded)
  def decode_to_bits(hash) when is_binary(hash), do: :erlang.nif_error(:nif_not_loaded)
  def bounds(hash) when is_binary(hash), do: :erlang.nif_error(:nif_not_loaded)
  def neighbors(hash) when is_binary(hash), do: :erlang.nif_error(:nif_not_loaded)

  def adjacent(hash, direction) when is_binary(hash) and is_binary(direction),
    do: :erlang.nif_error(:nif_not_loaded)
end
