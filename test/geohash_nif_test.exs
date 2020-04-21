defmodule GeohashNifTest do
  use ExUnit.Case
  doctest GeohashNif

  test "greets the world" do
    assert GeohashNif.hello() == :world
  end
end
