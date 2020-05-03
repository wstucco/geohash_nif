defmodule Mix.Tasks.Bench do
  use Mix.Task

  @preferred_cli_env :test
  @shortdoc "Bench Geohash NIF against Gehohash native Elixir implementation"
  @moduledoc "Bench Geohash NIF against Gehohash native Elixir implementation"

  @impl true
  def run(_args) do
    Code.compiler_options(ignore_module_conflict: true)
    Code.require_file("../../../deps/geohash/lib/geohash/helpers.ex", __DIR__)
    Code.require_file("../../../deps/geohash/lib/geohash.ex", __DIR__)

    unless to_string(Geohash.module_info()[:compile][:source]) =~ "deps" do
      Mix.raise("""
      Geohash native Elixir library was not corrrectly compiled
      """)
    end

    GeohashBench.run()
  end
end
