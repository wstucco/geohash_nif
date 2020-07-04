# GeohashNif

Drop in replacement fot the Elixir native [Geohash encode/decode library](https://hexdocs.pm/geohash/) implemented as a NIF

**Warning**

`neighbors` has a breaking change, see the [find neighbors](#find-neighbors).


## [Documentation](https://hexdocs.pm/geohash_nif/)

## Usage

- Encode coordinates with `Geohash.encode(lat, lon, precision \\ 11)`

```Elixir
iex(1)> Geohash.encode(42.6, -5.6, 5)
"ezs42"
```

- Decode coordinates with `Geohash.decode(geohash)`

```Elixir
iex(1)> Geohash.decode("ezs42")
{42.605, -5.603}
```

- <a name="find-neighbors"></a>Find neighbors

**warning**

*There is a difference in the implementation of the `neighbors` function, the original library returns a map of `string => string` this one returns a map of `atom: string`.*



```Elixir
iex(1)> Geohash.neighbors("ezs42")
%{
  e: "ezs43",
  n: "ezs48",
  ne: "ezs49",
  nw: "ezefx",
  s: "ezs40",
  se: "ezs41",
  sw: "ezefp",
  w: "ezefr"
}



```

- Find adjacent

```Elixir
iex(1)> Geohash.adjacent("ezs42","n")
"ezs48"

```

- Get bounds

```Elixir
iex(1)> Geohash.bounds("u4pruydqqv")
%{
  max_lat: 57.649115324020386,
  max_lon: 10.407443046569824,
  min_lat: 57.649109959602356,
  min_lon: 10.407432317733765
}

```

## Installation

  1. Add geohash_nif to your list of dependencies in `mix.exs`:

        def deps do
          [{:geohash_nif, "~> 1.0"}]
        end

  2. Ensure geohash is started before your application:

        def application do
          [applications: [:geohash_nif]]
        end
