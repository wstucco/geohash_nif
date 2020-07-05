# GeohashNif

Drop in replacement fot the Elixir native [Geohash encode/decode library](https://hexdocs.pm/geohash/) implemented as a NIF

## Installation

The package can be installed by adding geohash_nif to your list of dependencies in mix.exs:

```elixir
def deps do
  [{:geohash_nif, "~> 1.0"}]
end
```

## Basic Usage


```elixir
iex(1)> Geohash.encode(42.6, -5.6, 5)
"ezs42"

iex(1)> Geohash.decode("ezs42")
{42.605, -5.603}

iex(1)> Geohash.neighbors("ezs42")
%{
  "e" => "ezs43",
  "n" => "ezs48",
  "ne" => "ezs49",
  "nw" => "ezefx",
  "s" => "ezs40",
  "se" => "ezs41",
  "sw" => "ezefp",
  "w" => "ezefr"
}

iex(1)> Geohash.adjacent("ezs42","n")
"ezs48"

iex(1)> Geohash.bounds("u4pruydqqv")
%{
  max_lat: 57.649115324020386,
  max_lon: 10.407443046569824,
  min_lat: 57.649109959602356,
  min_lon: 10.407432317733765
}

```

Full documentation can be found at [https://hexdocs.pm/geohash_nif](https://hexdocs.pm/geohash_nif).


##  Differences to Geohash

For compatibility reasons `Geohash.neighbors/2` returns a map with string as keys,
but passing the option `keys: :atoms` a different implementation is called
which returns a map with atom as keys.

The atoms implementation is ~30% faster and uses ~40% less memory.


## Benchmarks

Included witht the library there is a complete suite of benchmarks available
as the `bench` mix task.

### Usage:

```bash
$ mix help bench

Bench Geohash NIF against Gehohash native Elixir implementation

## Usage

mix bench <bench_name> [<bench_name> ...]

If <bench_name> is omitted or one of them is `all` runs all the benchmarks.

<bench_name> can be a single value or a list of values separated by spaces.
Invalid values are simply ignored.

Valid <bench_name> values are

  • encode
  • decode
  • bounds
  • adjacent
  • neighbors
  • decode_to_bits

## Examples

  • $ mix bench - runs all the benchmarks
  • $ mix bench all - runs all the benchmarks
  • $ mix bench encode - runs the encode benchmark
  • $ mix bench encode decode - runs the encode and decode benchmarks
  • $ mix bench xxx all yyy - runs all benchmarks

```

Detailed benchmarks (including memory measurements): [benchmarks.txt](https://gitlab.com/wstucco/geohash_nif/-/raw/master/benchmarks.txt)


## License

GohashNif is released under the Apache License 2.0 - see the [LICENSE](https://gitlab.com/wstucco/geohash_nif/-/raw/master/LICENSE) file.

The code for the C geohash library is released under the MIT license - see the [LICENSE.geohash](https://gitlab.com/wstucco/geohash_nif/-/raw/master/LICENSE.geohash)