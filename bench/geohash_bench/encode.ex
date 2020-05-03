defmodule GeohashBench.Encode do
  def run do
    Benchee.run(
      %{
        "encode NIF" => fn length ->
          Geohash.Nif.encode(51.501568, -0.141257, length)
        end,
        "encode Elixir" => fn length ->
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
      },
      formatters: [
        {Benchee.Formatters.Console, extended_statistics: true}
        # {Benchee.Formatters.HTML, file: "results/encode.html", auto_open: false}
      ],
      time: 3,
      warmup: 1,
      memory_time: 2
    )
  end
end
