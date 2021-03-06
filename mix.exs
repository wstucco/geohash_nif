defmodule GeohashNif.MixProject do
  use Mix.Project

  @version "1.0.0"

  def project do
    [
      app: :geohash_nif,
      version: @version,
      elixir: "~> 1.9",
      elixirc_paths: elixirc_paths(Mix.env()),
      compilers: [:nif, :elixir, :app],
      preferred_cli_env: [bench: :test],
      deps: deps(),
      package: package(),
      description: description(),
      start_permanent: Mix.env() == :prod,
      deps: deps(),
      docs: docs()
    ]
  end

  def application, do: []

  defp elixirc_paths(:test), do: ["lib", "bench"]
  defp elixirc_paths(_), do: ["lib"]

  defp description do
    """
    Elixir NIF for encoding/decoding and manipulating geohashes.
    """
  end

  def package do
    [
      maintainers: ["Massimo Ronca"],
      licenses: ["MIT"],
      links: %{
        "GitHub" => "https://gitlab.com/wstucco/geohash_nif",
        "Issues" => "https://gitlab.com/wstucco/geohash_nif/issues",
        "Docs" => "http://hexdocs.pm/geohash_nif/#{@version}/"
      },
      files: [
        "c_src/*.h",
        "c_src/*.c",
        "c_src/COPYING",
        "lib",
        "LICENSE",
        "LICENSE.geohash",
        "Makefile",
        "mix.exs",
        "README.md",
        "src"
      ]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:benchee, "~> 1.0", only: :test},
      {:benchee_html, "~> 1.0", only: :test},
      {:geohash, "~> 1.2", only: :test},
      {:stream_data, "~> 0.1", only: :test},
      {:ex_doc, ">= 0.0.0", only: :dev, runtime: false}
    ]
  end

  defp docs() do
    [
      main: "readme",
      name: "GeohashNif",
      canonical: "http://hexdocs.pm/geohash_nif/#{@version}/",
      source_url: "https://gitlab.com/wstucco/geohash_nif",
      extras: [
        "README.md"
      ]
    ]
  end
end

defmodule Mix.Tasks.Compile.Nif do
  use Mix.Task
  @shortdoc "Compiles geohash nif library"
  def run(_) do
    if Mix.env() != :test, do: File.rm_rf("priv")
    File.mkdir("priv")

    make_cmd =
      System.get_env("MAKE") ||
        case :os.type() do
          {:unix, :freebsd} -> "gmake"
          {:unix, :openbsd} -> "gmake"
          {:unix, :netbsd} -> "gmake"
          {:unix, :dragonfly} -> "gmake"
          _ -> "make"
        end

    {result, error_code} = System.cmd(make_cmd, [], stderr_to_stdout: true)
    # IO.binwrite(result)

    if error_code != 0 do
      raise Mix.Error,
        message: """
          Could not run `#{make_cmd}`.
          Please check if `make` and either `clang` or `gcc` are installed
          Error: #{result}
        """
    end

    Mix.Project.build_structure()
    :ok
  end
end
