#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>

struct Test {
    char *url;
    long expected_status;
    const char *contains;
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
        fprintf(stderr, "not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

static void *run_test(void *arg) {
    struct Test *test = (struct Test *)arg;
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to init curl\n");
        return NULL;
    }

    struct Memory chunk = {0};
    curl_easy_setopt(curl, CURLOPT_URL, test->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "%s: request failed: %s\n", test->url, curl_easy_strerror(res));
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
        printf("%s: OK\n", test->url);
    }
    curl_easy_cleanup(curl);
    free(chunk.data);
    return NULL;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-s status] [-c string] [-f file] [url ...]\n", prog);
}

int main(int argc, char **argv) {
    const char *contains = NULL;
    long expected_status = 200;
    const char *file = NULL;

    int opt;
    for (opt = 1; opt < argc; ++opt) {
        if (!strcmp(argv[opt], "-s") && opt + 1 < argc) {
            expected_status = strtol(argv[++opt], NULL, 10);
        } else if (!strcmp(argv[opt], "-c") && opt + 1 < argc) {
            contains = argv[++opt];
        } else if (!strcmp(argv[opt], "-f") && opt + 1 < argc) {
            file = argv[++opt];
        } else if (argv[opt][0] == '-') {
            usage(argv[0]);
            return 1;
        } else {
            break;
        }
    }

    int url_count = argc - opt;
    char **urls = argv + opt;

    FILE *fp = NULL;
    if (file) {
        fp = fopen(file, "r");
        if (!fp) {
            perror("fopen");
            return 1;
        }
        // count lines to know how many urls
        char *line = NULL;
        size_t len = 0;
        url_count = 0;
        while (getline(&line, &len, fp) != -1) {
            if (line[0] != '\n' && line[0] != '#') url_count++;
        }
        rewind(fp);
        urls = malloc(sizeof(char*) * url_count);
        int idx = 0;
        while (getline(&line, &len, fp) != -1) {
            char *trim = strtok(line, "\n");
            if (!trim || trim[0] == '#') continue;
            urls[idx++] = strdup(trim);
        }
        free(line);
        fclose(fp);
    }

    if (url_count == 0) {
        usage(argv[0]);
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    pthread_t *threads = malloc(sizeof(pthread_t) * url_count);
    struct Test *tests = malloc(sizeof(struct Test) * url_count);

    for (int i = 0; i < url_count; ++i) {
        tests[i].url = urls[i];
        tests[i].expected_status = expected_status;
        tests[i].contains = contains;
        pthread_create(&threads[i], NULL, run_test, &tests[i]);
    }

    for (int i = 0; i < url_count; ++i) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    if (file) {
        for (int i = 0; i < url_count; ++i) {
            free(urls[i]);
        }
        free(urls);
    }
    free(tests);
    curl_global_cleanup();
    return 0;
}

