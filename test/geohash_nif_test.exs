defmodule GeohashTest do
  use ExUnit.Case
  use ExUnitProperties

  doctest Geohash

  test "Geohash.encode" do
    assert Geohash.encode(57.64911, 10.40744) == "u4pruydqqvj"
    assert Geohash.encode(50.958087, 6.9204459) == "u1hcvkxk65f"
    assert Geohash.encode(39.51, -76.24, 10) == "dr1bc0edrj"
    assert Geohash.encode(42.6, -5.6, 5) == "ezs42"
    assert Geohash.encode(0, 0) == "s0000000000"
    assert Geohash.encode(0, 0, 2) == "s0"
    assert Geohash.encode(57.648, 10.410, 6) == "u4pruy"
    assert Geohash.encode(-25.38262, -49.26561, 8) == "6gkzwgjz"
  end

  test "Geohash.bounds" do
    assert Geohash.bounds("u4pruydqqv") == %{
             min_lon: 10.407432317733765,
             min_lat: 57.649109959602356,
             max_lon: 10.407443046569824,
             max_lat: 57.649115324020386
           }
  end

  test "Geohash.encode matches elasticsearch geohash example" do
    assert Geohash.encode(51.501568, -0.141257, 1) == "g"
    assert Geohash.encode(51.501568, -0.141257, 2) == "gc"
    assert Geohash.encode(51.501568, -0.141257, 3) == "gcp"
    assert Geohash.encode(51.501568, -0.141257, 4) == "gcpu"
    assert Geohash.encode(51.501568, -0.141257, 5) == "gcpuu"
    assert Geohash.encode(51.501568, -0.141257, 6) == "gcpuuz"
    assert Geohash.encode(51.501568, -0.141257, 7) == "gcpuuz9"
    assert Geohash.encode(51.501568, -0.141257, 8) == "gcpuuz94"
    assert Geohash.encode(51.501568, -0.141257, 9) == "gcpuuz94k"
    assert Geohash.encode(51.501568, -0.141257, 10) == "gcpuuz94kk"
    assert Geohash.encode(51.501568, -0.141257, 11) == "gcpuuz94kkp"
    assert Geohash.encode(51.501568, -0.141257, 12) == "gcpuuz94kkp5"
  end

  test "Geohash.decode_to_bits" do
    assert Geohash.decode_to_bits("ezs42") == <<0b0110111111110000010000010::25>>
  end

  test "Geohash.decode" do
    assert Geohash.decode("ww8p1r4t8") == {37.832386, 112.558386}
    assert Geohash.decode("ezs42") == {42.605, -5.603}
    assert Geohash.decode("u4pruy") == {57.648, 10.410}
    assert Geohash.decode("6gkzwgjz") == {-25.38262, -49.26561}
  end

  test "Geohash.neighbors" do
    assert Geohash.neighbors("6gkzwgjz") == %{
             n: "6gkzwgmb",
             s: "6gkzwgjy",
             e: "6gkzwgnp",
             w: "6gkzwgjx",
             ne: "6gkzwgq0",
             se: "6gkzwgnn",
             nw: "6gkzwgm8",
             sw: "6gkzwgjw"
           }
  end

  test "Geohash.adjacent" do
    assert Geohash.adjacent("ww8p1r4t8", "e") == "ww8p1r4t9"
  end

  @geobase32 '0123456789bcdefghjkmnpqrstuvwxyz'

  defp geocodes_domain,
    do: StreamData.list_of(StreamData.member_of(@geobase32), min_length: 1, max_length: 12)

  property "decode is reversible" do
    check all(geohash <- geocodes_domain(), max_runs: 5_000) do
      geohash = to_string(geohash)
      precision = String.length(geohash)
      {lat, lng} = Geohash.decode(geohash)
      new_geohash = Geohash.encode(lat, lng, precision)
      geohash == new_geohash
    end
  end

  @tag iterations: 10_000
  property "neighbors is reversible" do
    check all(geohash <- geocodes_domain(), max_runs: 5_000) do
      geohash = to_string(geohash)

      for {direction, opposite} <- [{"n", "s"}, {"e", "w"}, {"s", "n"}, {"w", "e"}] do
        adj = Geohash.adjacent(geohash, direction)
        original = Geohash.adjacent(adj, opposite)

        assert(
          geohash === original,
          "Inverse operation didn't work \"#{geohash} -> #{adj} -> #{original}\""
        )
      end
      |> Enum.all?()
    end
  end

  # Error margins taken from Wikipedia's Geohash page
  @error_margin %{
    1 => {23, 23},
    2 => {2.8, 5.6},
    3 => {0.70, 0.7},
    4 => {0.087, 0.18},
    5 => {0.022, 0.022},
    6 => {0.0027, 0.0055},
    7 => {0.00068, 0.00068},
    8 => {0.000085, 0.00017}
  }

  property "errors are below margin after encode/decode" do
    check all(
            lat <- StreamData.float(min: -90.0, max: 90.0),
            lng <- StreamData.float(min: -180.0, max: 180.0),
            precision <- StreamData.integer(1..8),
            # TODO: check lng error margin for precision 2
            precision != 2,
            max_runs: 500
          ) do
      geohash = Geohash.encode(lat, lng, precision)
      {new_lat, new_lng} = Geohash.decode(geohash)
      new_geohash = Geohash.encode(new_lat, new_lng, precision)
      {lat_error, lng_error} = @error_margin[precision]

      lat_precision = lat_error |> :math.log10() |> ceil() |> abs()
      lng_precision = lng_error |> :math.log10() |> ceil() |> abs()
      new_lat_error = Float.round(abs(new_lat - lat) - lat_error, lat_precision)
      new_lng_error = Float.round(abs(new_lng - lng) - lng_error, lng_precision)
      {lat_threshold, _} = Float.parse("1e-#{lat_precision}")
      {lng_threshold, _} = Float.parse("1e-#{lng_precision}")

      ok? = new_lat_error <= lat_threshold and new_lng_error <= lng_threshold

      unless ok? do
        IO.inspect({"coords", {lat, lng}})
        IO.inspect({"precision", precision, lat_precision, lng_precision})
        IO.inspect({"new coords", {new_lat, new_lng}})
        IO.inspect({"error margin", {lat_error, lng_error}})
        IO.inspect({"thresholds", {lat_threshold, lng_threshold}})

        IO.inspect({"real error", abs(new_lat - lat), abs(new_lng - lng)})

        IO.inspect({"difference", {new_lat_error, new_lng_error}})

        IO.inspect({geohash, new_geohash})
      end

      assert ok?
    end
  end

  property "encode -> decode -> encode is the same geohash" do
    check all(
            lat <- StreamData.float(min: -90.0, max: 90.0),
            lon <- StreamData.float(min: -180.0, max: 180.0),
            precision <- StreamData.integer(1..8),
            max_runs: 500
          ) do
      geohash = Geohash.encode(lat, lon, precision)
      {new_lat, new_lon} = Geohash.decode(geohash)
      new_geohash = Geohash.encode(new_lat, new_lon, precision)
      assert geohash == new_geohash
    end
  end

  property "coordinate encoded is inside geohash boundaries" do
    check all(
            lat <- StreamData.float(min: -90.0, max: 90.0),
            lon <- StreamData.float(min: -180.0, max: 180.0),
            precision <- StreamData.integer(1..8),
            max_runs: 800
          ) do
      geohash = Geohash.encode(lat, lon, precision)
      bounds = Geohash.bounds(geohash)
      assert bounds.min_lat <= lat && lat <= bounds.max_lat
      assert bounds.min_lon <= lon && lon <= bounds.max_lon
    end
  end
end
