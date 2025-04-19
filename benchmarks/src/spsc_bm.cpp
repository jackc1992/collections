#include <benchmark/benchmark.h>
#include <jc_collections/spsc.hpp>

#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <thread>
#include <print>

void set_thread_affinity(int core_id) {
#ifdef __linux__
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core_id, &cpu_set);

    pthread_t current_thread = pthread_self();
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpu_set);

#endif
}

struct alignas(4) bigger {
    int x_;
};

static void bm_spsc_cached_throughput(benchmark::State& state)
{
    const int core_id = (state.thread_index() == 0) ? 2 : 3;

    set_thread_affinity(core_id);
    static JC_SPSC::cached_spsc<bigger, 512> q;

    if (state.thread_index() == 0) {
        // Producer thread loop
        for (auto _ : state) {
            q.put(bigger { 0 });
        }
    } else {
        // Consumer thread loop
        for (auto _ : state) {
            bigger val = q.read();
            benchmark::DoNotOptimize(val);
        }
    }
}


static void bm_spsc_throughput(benchmark::State& state)
{
    const int core_id = (state.thread_index() == 0) ? 2 : 3;

    set_thread_affinity(core_id);

    static JC_SPSC::simple_spsc<bigger, 2048> q;

    if (state.thread_index() == 0) {
        // Producer thread loop
        for (auto a : state) {
            q.put(bigger { 1 });
        }
    } else {
        // Consumer thread loop
        for (auto _ : state) {
            auto val = q.read();
            benchmark::DoNotOptimize(val);
        }
    }
}

template<typename T>
static void bm_rtt_throughput(benchmark::State& state) {
    static auto q1 = JC_SPSC::cached_spsc<T, 512>();
    static auto q2 = JC_SPSC::cached_spsc<T, 512>();

    if (state.thread_index() == 0) {
        set_thread_affinity(3);
        for (auto _ : state) {
            q1.put(T());
            T ele = q2.read();
            benchmark::DoNotOptimize(ele);
        }
    } else {
        set_thread_affinity(2);
        for (auto _ : state) {
            T ele = q1.read();
            benchmark::DoNotOptimize(ele);
            q2.put(std::move(ele));
        }
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bm_spsc_cached_throughput)->Threads(2)->UseRealTime()->MinTime(1.0);
BENCHMARK(bm_spsc_throughput)->Threads(2)->UseRealTime()->Iterations(100000000);
BENCHMARK(bm_rtt_throughput<size_t>)->Threads(2)->UseRealTime()->Range((1 << 16), (1 << 22));


BENCHMARK_MAIN();