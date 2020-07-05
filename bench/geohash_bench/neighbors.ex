defmodule GeohashBench.Neighbors do
  def bench_spec do
    [
      benchmarks: %{
        "neighbors with NIF (binary keys)" => fn input ->
          Geohash.Nif.neighbors(input)
        end,
        "neighbors with NIF (atom keys)" => fn input ->
          Geohash.Nif.neighbors2(input)
        end,
        "neighbors with Elixir" => fn input ->
          Geohash.neighbors(input)
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
