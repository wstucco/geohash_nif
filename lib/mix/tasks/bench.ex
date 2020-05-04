defmodule Mix.Tasks.Bench do
  use Mix.Task

  @preferred_cli_env :test
  @shortdoc "Bench Geohash NIF against Gehohash native Elixir implementation"
  @moduledoc """
  Bench Geohash NIF against Gehohash native Elixir implementation

  If run without parameters runs all the benchmarks.

  ## Command line options
    * `all` - if present, enables **all** the benchmarks
    * `encode` - if present, enables the `encode` benchmark
    * `decode` - if present, enables the `decode` benchmark
    * `adjacent` - if present, enables the `adjacent` benchmark
    * `neighbours` - if present, enables the `neighbours` benchmark
    * `bounds` - if present, enables the `bounds` benchmark

  Options can be combined to run multiple benchmarks.

  ## Examples

  * `$ mix bench` - runs all the benchmarks
  * `$ mix bench all` - runs all the benchmarks
  * `$ mix bench encode` - runs the `encode` benchmark
  * `$ mix bench encode decode` - runs the `encode` and `decode` benchmarks
  * `$ mix bench xxx all yyy` - runs all benchmarks
  """

  @bench_funcs ["encode", "decode", "bounds", "adjacent", "neighbours"]

  @impl true
  def run(args) do
    Code.compiler_options(ignore_module_conflict: true)
    Code.require_file("../../../deps/geohash/lib/geohash/helpers.ex", __DIR__)
    Code.require_file("../../../deps/geohash/lib/geohash.ex", __DIR__)

    unless to_string(Geohash.module_info()[:compile][:source]) =~ "deps" do
      Mix.raise("""
      Geohash native Elixir library was not corrrectly compiled
      """)
    end

    args
    |> get_benchmarks
    |> GeohashBench.run()
  end

  defp get_benchmarks([]), do: @bench_funcs
  defp get_benchmarks(["all"]), do: @bench_funcs

  defp get_benchmarks(args) do
    if "all" in args do
      @bench_funcs
    else
      Enum.reduce(args, [], fn
        arg, acc when arg in @bench_funcs ->
          [arg | acc]

        _arg, acc ->
          acc
      end)
    end
  end
end
