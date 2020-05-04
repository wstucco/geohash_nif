defmodule GeohashBench.Encode do
  def bench_spec do
    [
      benchmarks: %{
        "encode with NIF" => fn length ->
          Geohash.Nif.encode(51.501568, -0.141257, length)
        end,
        "encode with Elixir" => fn length ->
          Geohash.encode(51.501568, -0.141257, length)
        end
      },
      inputs: %{
        "length: 01" => 1,
        "length: 02" => 2,
        "length: 03" => 3,
        "length: 04" => 4,
        "length: 05" => 5,
        "length: 06" => 6,
        "length: 07" => 7,
        "length: 08" => 8,
        "length: 09" => 9,
        "length: 10" => 10,
        "length: 11" => 11,
        "length: 12" => 12
      }
    ]
  end
end
