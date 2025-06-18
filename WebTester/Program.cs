using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace WebTester
{
    class Program
    {
        static async Task<int> Main(string[] args)
        {
            if (args.Length == 0)
            {
                Console.WriteLine("Usage: WebTester <url> [expectedStatusCode] [containsString]");
                return 1;
            }

            string url = args[0];
            int expectedStatusCode = 200;
            if (args.Length > 1 && !int.TryParse(args[1], out expectedStatusCode))
            {
                Console.WriteLine($"Invalid status code: {args[1]}");
                return 1;
            }

            string contains = args.Length > 2 ? args[2] : null;

            using var client = new HttpClient();
            HttpResponseMessage response;
            try
            {
                response = await client.GetAsync(url);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Request failed: {ex.Message}");
                return 1;
            }

            if ((int)response.StatusCode != expectedStatusCode)
            {
                Console.WriteLine($"Expected status {expectedStatusCode} but got {(int)response.StatusCode}");
                return 1;
            }

            if (contains != null)
            {
                string body = await response.Content.ReadAsStringAsync();
                if (!body.Contains(contains, StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine($"Body does not contain expected string: {contains}");
                    return 1;
                }
            }

            Console.WriteLine("Test passed!");
            return 0;
        }
    }
}
