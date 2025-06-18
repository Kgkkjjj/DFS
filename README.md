# DFS

This repository includes a simple C# console application for basic web testing.

## Building

Ensure you have [.NET 6 SDK](https://dotnet.microsoft.com/download/dotnet/6.0) installed and run:

```bash
dotnet build WebTester/WebTester.csproj
```

## Running

To test a web page, pass the URL, optional expected status code (default is `200`), and an optional string that should appear in the response body.

```bash
dotnet run --project WebTester/WebTester.csproj -- https://example.com 200 Example
```

The application will output `Test passed!` when the conditions are satisfied.
