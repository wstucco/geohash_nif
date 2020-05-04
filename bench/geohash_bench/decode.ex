defmodule GeohashBench.Decode do
  def bench_spec do
    [
      benchmarks: %{
        "decode with NIF" => fn input ->
          Geohash.Nif.decode(to_charlist(input))
        end,
        "decode with Elixir" => fn input ->
          Geohash.decode(input)
        end
      },
      inputs: %{
        "length: 01" => "g",
        "length: 02" => "gc",
        "length: 03" => "gcp",
        "length: 04" => "gcpu",
        "length: 05" => "gcpuu",
        "length: 06" => "gcpuuz",
        "length: 07" => "gcpuuz9",
        "length: 08" => "gcpuuz94",
        "length: 09" => "gcpuuz94k",
        "length: 10" => "gcpuuz94kk",
        "length: 11" => "gcpuuz94kkp",
        "length: 12" => "gcpuuz94kkp5"
      }
    ]
  end
end
