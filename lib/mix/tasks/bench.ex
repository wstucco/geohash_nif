defmodule Mix.Tasks.Bench do
  use Mix.Task

  @bench_funcs ["encode", "decode", "bounds", "adjacent", "neighbors", "decode_to_bits"]
  @preferred_cli_env :test
  @shortdoc "Bench Geohash NIF against Gehohash native Elixir implementation"
  @moduledoc """
  Bench Geohash NIF against Gehohash native Elixir implementation

  ## Usage

  `mix bench <bench_name> [<bench_name> ...]`

  If `<bench_name>` is omitted or one of them is **all** runs all the benchmarks.

  `<bench_name>` can be a single value or a list of values separated by spaces.
  Invalid values are simply ignored.

  Valid `<bench_name>` values are
  * **#{Enum.join(@bench_funcs, "**\n* **")}**

  ## Examples

  * `$ mix bench` - runs all the benchmarks
  * `$ mix bench all` - runs all the benchmarks
  * `$ mix bench encode` - runs the `encode` benchmark
  * `$ mix bench encode decode` - runs the `encode` and `decode` benchmarks
  * `$ mix bench xxx all yyy` - runs all benchmarks
  """

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
