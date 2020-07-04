import Config

config :logger, level: :warn

Code.compiler_options(ignore_module_conflict: true)
Code.require_file("../deps/geohash/lib/geohash/helpers.ex", __DIR__)
Code.require_file("../deps/geohash/lib/geohash.ex", __DIR__)
