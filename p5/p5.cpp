#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

static constexpr int BUF_CAP = 10;
static constexpr int SENTINEL = -1;

struct FactorResult {
    int original;
    std::vector<int> factors;
};

struct IntBuffer {
    int items[BUF_CAP];
    int head = 0, tail = 0, count = 0;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
    pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

    void put(int v) {
        pthread_mutex_lock(&mtx);
        while (count == BUF_CAP) {
            pthread_cond_wait(&not_full, &mtx);
        }
        items[tail] = v;
        tail = (tail + 1) % BUF_CAP;
        count++;
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mtx);
    }

    int get() {
        pthread_mutex_lock(&mtx);
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mtx);
        }
        int v = items[head];
        head = (head + 1) % BUF_CAP;
        count--;
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mtx);
        return v;
    }
};

struct ResBuffer {
    FactorResult* items[BUF_CAP];
    int head = 0, tail = 0, count = 0;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
    pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

    void put(FactorResult* p) {
        pthread_mutex_lock(&mtx);
        while (count == BUF_CAP) {
            pthread_cond_wait(&not_full, &mtx);
        }
        items[tail] = p;
        tail = (tail + 1) % BUF_CAP;
        count++;
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mtx);
    }

    FactorResult* get() {
        pthread_mutex_lock(&mtx);
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mtx);
        }
        FactorResult* p = items[head];
        head = (head + 1) % BUF_CAP;
        count--;
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mtx);
        return p;
    }
};

struct Shared {
    IntBuffer* in;
    ResBuffer* out;
};

static FactorResult* factor_number(int n) {
    FactorResult* res = new (std::nothrow) FactorResult();
    if (!res) return nullptr;

    res->original = n;

    int x = n;

    while (x % 2 == 0) {
        res->factors.push_back(2);
        x /= 2;
    }

    for (int d = 3; (long long)d * d <= x; d += 2) {
        while (x % d == 0) {
            res->factors.push_back(d);
            x /= d;
        }
    }

    if (x > 1) {
        res->factors.push_back(x);
    }

    return res;
}

static void* producer_main(void* arg) {
    Shared* sh = static_cast<Shared*>(arg);

    while (true) {
        int n = sh->in->get();
        if (n == SENTINEL) {
            sh->out->put(nullptr); 
            break;
        }

        FactorResult* res = factor_number(n);
        if (!res) {
            sh->out->put(nullptr); 
            break;
        }

        sh->out->put(res);
    }

    return nullptr;
}

static void* consumer_main(void* arg) {
    Shared* sh = static_cast<Shared*>(arg);

    while (true) {
        FactorResult* res = sh->out->get();
        if (res == nullptr) break;

        std::printf("%d:", res->original);
        for (int f : res->factors) {
            std::printf(" %d", f);
        }
        std::printf("\n");

        delete res;
    }

    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::printf("Usage:%s <number to factor>...\n", argv[0]);
        return 0;
    }

    int nnums = argc - 1;


    IntBuffer inBuf;
    ResBuffer outBuf;
    Shared sh{&inBuf, &outBuf};

    pthread_t producer, consumer;

    if (pthread_create(&producer, nullptr, producer_main, &sh) != 0) {
        std::fprintf(stderr, "Error: pthread_create producer failed\n");
        return 1;
    }
    if (pthread_create(&consumer, nullptr, consumer_main, &sh) != 0) {
        std::fprintf(stderr, "Error: pthread_create consumer failed\n");
        inBuf.put(SENTINEL);
        pthread_join(producer, nullptr);
        return 1;
    }

    for (int i = 0; i < nnums; i++) {
        int val = std::atoi(argv[i + 1]);
        if (val < 2) {
            std::fprintf(stderr, "Error: invalid input '%s' (must be >= 2)\n", argv[i + 1]);
            inBuf.put(SENTINEL);
            pthread_join(producer, nullptr);
            pthread_join(consumer, nullptr);
            return 1;
        }
        inBuf.put(val);
    }

    inBuf.put(SENTINEL);

    pthread_join(producer, nullptr);
    pthread_join(consumer, nullptr);
    return 0;
}