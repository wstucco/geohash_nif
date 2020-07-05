defmodule GeohashBench do
  def run([]) do
    IO.puts("No valid benchmark name specified\nSee `mix help bench` for usage")
  end

  def run(what) do
    what
    |> Enum.each(fn bench ->
      module = Module.concat(GeohashBench, Macro.camelize(bench))

      case apply(module, :bench_spec, []) do
        [] ->
          IO.puts("** Skipping bench `#{bench}`: spec is empty")
          []

        spec ->
          do_bench(spec)
      end
    end)
  end

  defp do_bench(spec) do
    Benchee.run(
      spec[:benchmarks],
      inputs: spec[:inputs],
      time: 1,
      warmup: 0,
      memory_time: 1,
      print: [configuration: true]
    )
  end
end
