#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>

struct ApiTest {
    char *method;
    char *url;
    long expected_status;
    char *contains; // optional
};

struct Memory {
    char *data;
    size_t size;
};

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "not enough memory\n");
        return 0;
    }
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

static void *run_test(void *arg) {
    struct ApiTest *test = (struct ApiTest *)arg;
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to init curl\n");
        return NULL;
    }
    struct Memory chunk = {0};
    curl_easy_setopt(curl, CURLOPT_URL, test->url);
    if (strcasecmp(test->method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    } else if (strcasecmp(test->method, "HEAD") == 0) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "%s %s: request failed: %s\n", test->method, test->url, curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(chunk.data);
        return NULL;
    }
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (status != test->expected_status) {
        fprintf(stderr, "%s: expected %ld but got %ld\n", test->url, test->expected_status, status);
    } else if (test->contains && (!chunk.data || !strstr(chunk.data, test->contains))) {
        fprintf(stderr, "%s: body does not contain '%s'\n", test->url, test->contains);
    } else {
        printf("%s %s: OK\n", test->method, test->url);
    }
    curl_easy_cleanup(curl);
    free(chunk.data);
    return NULL;
}

static int load_tests(const char *file, struct ApiTest **out_tests) {
    FILE *fp = fopen(file, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    char *line = NULL;
    size_t len = 0;
    int count = 0;
    while (getline(&line, &len, fp) != -1) {
        if (line[0] == '#' || line[0] == '\n') continue;
        count++;
    }
    rewind(fp);
    struct ApiTest *tests = calloc(count, sizeof(struct ApiTest));
    int idx = 0;
    while (getline(&line, &len, fp) != -1) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char method[8] = {0};
        char url[1024] = {0};
        long status = 0;
        char contains[256] = {0};
        int items = sscanf(line, "%7s %1023s %ld %255s", method, url, &status, contains);
        if (items < 3) {
            fprintf(stderr, "Invalid line: %s", line);
            continue;
        }
        tests[idx].method = strdup(method);
        tests[idx].url = strdup(url);
        tests[idx].expected_status = status;
        if (items == 4) tests[idx].contains = strdup(contains);
        idx++;
    }
    free(line);
    fclose(fp);
    *out_tests = tests;
    return idx;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <tests_file>\n", argv[0]);
        return 1;
    }
    const char *file = argv[1];
    struct ApiTest *tests = NULL;
    int test_count = load_tests(file, &tests);
    if (test_count <= 0) {
        return 1;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);
    pthread_t *threads = malloc(sizeof(pthread_t) * test_count);
    for (int i = 0; i < test_count; ++i) {
        pthread_create(&threads[i], NULL, run_test, &tests[i]);
    }
    for (int i = 0; i < test_count; ++i) {
        pthread_join(threads[i], NULL);
    }
    for (int i = 0; i < test_count; ++i) {
        free(tests[i].method);
        free(tests[i].url);
        free(tests[i].contains);
    }
    free(tests);
    free(threads);
    curl_global_cleanup();
    return 0;
}
