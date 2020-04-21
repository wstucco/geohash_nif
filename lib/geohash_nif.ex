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
end
