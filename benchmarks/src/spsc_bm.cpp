// #include <benchmark/benchmark.h>
// #include <jc_collections/spsc.hpp>

// #define _GNU_SOURCE
// #include <pthread.h>
// #include <sched.h>
// #include <unistd.h>

// void set_thread_affinity(int core_id) {
//     cpu_set_t cpu_set;
//     CPU_ZERO(&cpu_set);
//     CPU_SET(core_id, &cpu_set);

//     pthread_t current_thread = pthread_self();
//     pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpu_set);
// }

// struct bigger {
//     size_t a[32];
// };

// static void bm_spsc_cached_throughput(benchmark::State& state)
// {
//     int core_id = (state.thread_index() == 0) ? 2 : 9;

//     set_thread_affinity(core_id);

//     static JC_SPSC::cached_spsc<bigger, 2048> q;

//     if (state.thread_index() == 0) {
//         // Producer thread loop
//         for (auto _ : state) {
//             q.put(bigger { 0 });
//         }
//     } else {
//         // Consumer thread loop
//         for (auto _ : state) {
//             bigger val = q.read();
//             benchmark::DoNotOptimize(val);
//         }
//     }
// }


// static void bm_spsc_throughput(benchmark::State& state)
// {
//     int core_id = (state.thread_index() == 0) ? 2 : 3;

//     set_thread_affinity(core_id);

//     static JC_SPSC::simple_spsc<size_t, 2048> q;

//     if (state.thread_index() == 0) {
//         // Producer thread loop
//         for (auto _ : state) {
//             q.put(1);
//         }
//     } else {
//         // Consumer thread loop
//         for (auto _ : state) {
//             int val = q.read();
//             benchmark::DoNotOptimize(val);
//         }
//     }
// }

// BENCHMARK(bm_spsc_throughput)->Threads(2)->UseRealTime()->Iterations(100000000);
// BENCHMARK(bm_spsc_cached_throughput)->Threads(2)->UseRealTime()->MinTime(1.0);

// BENCHMARK_MAIN();