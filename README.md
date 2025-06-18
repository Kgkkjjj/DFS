# DFS

This repository provides tools for simple web testing. Initially a C# console app existed, but a more advanced C implementation is now available. The C version supports testing multiple URLs concurrently and checking for content in the response body.

## Building the C tool

Make sure you have GCC and libcurl installed. Then run:

```bash
make -C CWebTester
```

This will produce the `cwebtester` binary in `CWebTester/`.

## Using the C tool

```bash
./CWebTester/cwebtester [-s status] [-c string] [-f urls.txt] <url> [url ...]
```

- `-s status` sets the expected HTTP status code (default 200).
- `-c string` specifies text that must appear in the body.
- `-f urls.txt` reads additional URLs from a file (one per line). All URLs are tested concurrently.

Each URL will print `OK` if the response matches the conditions, otherwise an error message is shown.

## Legacy C# tester

The original C# application remains in `WebTester/` for reference. Build it with the .NET 6 SDK:

```bash
dotnet build WebTester/WebTester.csproj
```

Run it with:

```bash
dotnet run --project WebTester/WebTester.csproj -- https://example.com 200 Example
```

