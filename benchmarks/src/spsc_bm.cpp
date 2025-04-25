#include <benchmark/benchmark.h>
#include <jc_collections/lockfree/spsc.hpp>
#include "../include/SPSCQueue.h" // rigtorps queue
#include <boost/lockfree/spsc_queue.hpp> // boost
#include <folly/ProducerConsumerQueue.h> // todo: figure out why this is so high
#define _GNU_SOURCE


#include <pthread.h>
#include <sched.h>
#include <thread>

void set_thread_affinity(int core_id)
{
#ifdef __linux__
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core_id, &cpu_set);

    pthread_t current_thread = pthread_self();
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpu_set);

#endif
}

struct bigger
{
    int x_[2];
};

static void bm_spsc_cached_throughput(benchmark::State &state)
{
    const int core_id = (state.thread_index() == 0) ? 2 : 3;

    set_thread_affinity(core_id);
    static jc::lockfree::cached_spsc<bigger, 512> q;

    if (state.thread_index() == 0)
    {
        // Producer thread loop
        for (auto _ : state)
        {
            q.emplace(bigger{1});
        }
    }
    else
    {
        // Consumer thread loop
        for (auto _ : state)
        {
            bigger val = q.read();
            benchmark::DoNotOptimize(val);
        }
    }
}

static void bm_rigtorp_spsc_throughput(benchmark::State &state)
{
    const int core_id = (state.thread_index() == 0) ? 2 : 3;

    set_thread_affinity(core_id);
    static rigtorp::SPSCQueue<bigger> q(512);

    if (state.thread_index() == 0)
    {
        // Producer thread loop
        for (auto _ : state)
        {
            q.push(bigger{1});
        }
    }
    else
    {
        // Consumer thread loop
        for (auto _ : state)
        {
            while (q.front() == nullptr)
            {
            }
            bigger val = *q.front();
            q.pop();
            benchmark::DoNotOptimize(val);
        }
    }
}

static void bm_boost_spsc_throughput(benchmark::State &state)
{
    const int core_id = (state.thread_index() == 0) ? 2 : 3;

    set_thread_affinity(core_id);
    static boost::lockfree::spsc_queue<bigger> q(1024);

    if (state.thread_index() == 0)
    {
        // Producer thread loop
        for (auto _ : state)
        {
            while (!q.push(bigger{1}))
                ;
        }
    }
    else
    {
        // Consumer thread loop
        for (auto _ : state)
        {
            bigger val{};
            while (q.pop(&val, 1) != 1)
            {
            };
            benchmark::DoNotOptimize(val);
        }
    }
}

static void bm_folly_spsc_throughput(benchmark::State &state)
{
    const int core_id = (state.thread_index() == 0) ? 2 : 3;

    static folly::ProducerConsumerQueue<bigger> q(512);

    if (state.thread_index() == 0)
    {
        // Producer thread loop
        for (auto _ : state)
        {
            while (!q.write(bigger{0}))
                ;
        }
    }
    else
    {
        // Consumer thread loop
        for (auto _ : state)
        {
            bigger val{};
            while (!q.read(val))
            {
            };
            benchmark::DoNotOptimize(val);
        }
    }
}

static void bm_spsc_simple_throughput(benchmark::State &state)
{
    const int core_id = (state.thread_index() == 0) ? 2 : 3;

    set_thread_affinity(core_id);

    static jc::lockfree::simple_spsc<bigger, 512> q;

    if (state.thread_index() == 0)
    {
        // Producer thread loop
        for (auto a : state)
        {
            q.put(bigger{1});
        }
    }
    else
    {
        // Consumer thread loop
        for (auto _ : state)
        {
            auto val = q.read();
            benchmark::DoNotOptimize(val);
        }
    }
}

template <typename T>
static void bm_rtt_throughput(benchmark::State &state)
{
    static auto q1 = jc::lockfree::cached_spsc<T, 8192>();
    static auto q2 = jc::lockfree::cached_spsc<T, 8192>();

    if (state.thread_index() == 0)
    {
        set_thread_affinity(1);
        for (auto _ : state)
        {
            q1.put(T());
            T ele = q2.read();
        }
    }
    else
    {
        set_thread_affinity(2);
        for (auto _ : state)
        {
            q2.put(q1.read());
        }
    }
    state.SetItemsProcessed(state.iterations());
}

template <typename T>
static void bm_rigtorp_rtt(benchmark::State &state)
{
    static auto q1 = rigtorp::SPSCQueue<T>(1 << 10);
    static auto q2 = rigtorp::SPSCQueue<T>(1 << 10);

    if (state.thread_index() == 0)
    {
        set_thread_affinity(1);
        for (auto _ : state)
        {
            q1.emplace(T());
            while (!q2.front())
            {
            }
            q2.pop();
        }
    }
    else
    {
        set_thread_affinity(2);
        for (auto _ : state)
        {
            while (!q1.front())
            {
            }
            q2.emplace(*(q1.front()));
            q1.pop();
        }
    }
    state.SetItemsProcessed(state.iterations());
}

template <typename T>
static void bm_boost_rtt(benchmark::State &state)
{
    static auto q1 = boost::lockfree::spsc_queue<T>(1 << 10);
    static auto q2 = boost::lockfree::spsc_queue<T>(1 << 10);

    if (state.thread_index() == 0)
    {
        set_thread_affinity(1);
        for (auto _ : state)
        {
            T ele;
            q1.push(T());
            while (q2.pop(&ele, 1) != 1)
            {
            }
        }
    }
    else
    {
        set_thread_affinity(2);
        for (auto _ : state)
        {
            T ele;
            while (q1.pop(&ele, 1) != 1)
            {
            }
            q2.push(ele);
        }
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bm_rigtorp_spsc_throughput)->Threads(2)->UseRealTime()->MinTime(1.0);
BENCHMARK(bm_boost_spsc_throughput)->Threads(2)->UseRealTime()->MinTime(1.0);
BENCHMARK(bm_folly_spsc_throughput)->Threads(2)->UseRealTime()->MinTime(1.0);
BENCHMARK(bm_spsc_simple_throughput)->Threads(2)->UseRealTime()->MinTime(1.0);
BENCHMARK(bm_spsc_cached_throughput)->Threads(2)->UseRealTime()->MinTime(1.0);
BENCHMARK(bm_rtt_throughput<int>)->Threads(2)->UseRealTime()->Range((1 << 16), (1 << 22));
BENCHMARK(bm_rigtorp_rtt<int>)->Threads(2)->UseRealTime()->Range((1 << 16), (1 << 22));
BENCHMARK(bm_boost_rtt<int>)->Threads(2)->UseRealTime()->Range((1 << 16), (1 << 22));

BENCHMARK_MAIN();
