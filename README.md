# DFS

This repository provides tools for simple web testing. Initially a C# console app existed, but C implementations are now available. The basic C tool checks URLs concurrently while the API tester can exercise multiple endpoints defined in a file.

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

## Building the API tester

Run the following to build the more flexible API tester written in C:

```bash
make -C ApiTester
```

This will produce the `apitester` binary in `ApiTester/`.

## Using the API tester

Create a text file listing your API checks. Each non-empty line should have the form:

```
METHOD URL STATUS [CONTAINS]
```

Lines beginning with `#` are ignored. For example:

```
GET https://example.com/api/health 200 ok
POST https://example.com/api/login 404
```

Run the tests with:

```bash
./ApiTester/apitester tests.txt
```

## Legacy C# tester

The original C# application remains in `WebTester/` for reference. Build it with the .NET 6 SDK:

```bash
dotnet build WebTester/WebTester.csproj
```

Run it with:

```bash
dotnet run --project WebTester/WebTester.csproj -- https://example.com 200 Example
```

