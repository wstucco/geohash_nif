defmodule Geohash.Nif do
  @moduledoc false
  @on_load :init

  def init do
    :erlang.load_nif(:code.priv_dir(:geohash_nif) ++ '/geohash', 0)
  end

  @doc false
  def encode(_latitude, _longitude, _length \\ 11)
  def encode(_latitude, _longitude, _length), do: :erlang.nif_error(:nif_not_loaded)

  def decode(_geohash), do: :erlang.nif_error(:nif_not_loaded)

  def bounds(_hash), do: :erlang.nif_error(:nif_not_loaded)
end
