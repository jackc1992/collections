// #include <jc_collections/spsc.hpp>
//
// #define _GNU_SOURCE
// #include <pthread.h>
// #include <sched.h>
// #include <unistd.h>
// #include <vector>
// #include <print>
// #include <thread>
// #include <numeric>
//
// using std::size_t;
//
// void set_thread_affinity(int core_id) {
//     cpu_set_t cpu_set;
//     CPU_ZERO(&cpu_set);
//     CPU_SET(core_id, &cpu_set);
//
//     pthread_t current_thread = pthread_self();
//     pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpu_set);
// }
//
// constexpr size_t get_size(size_t floor)
// {
//     size_t res = 1 ;
//     while (res < floor)
//     {
//         res *= 2;
//     }
//     return res;
// }
//
// struct alignas(8) TestSize
// {
//     int x_;
// };
//
//
// int main()
// {
//     const size_t iters = get_size(5'000'000);
//     const size_t trials = 5;
//
//     std::println("testing jc_spsc, iters: {}", iters);
//
//     std::vector<size_t> ops(trials);
//     std::vector<size_t> rtt(trials);
//
//
//     for (size_t i = 0; i < trials; i++)
//     {
//         auto q = JC_SPSC::cached_spsc<TestSize, 2048>();
//         auto thread = std::thread([&]() {
//             set_thread_affinity(2);
//             for (size_t i = 0; i < iters; i++)
//             {
//                 TestSize val = q.read();
//             }
//         });
//
//         set_thread_affinity(8);
//         auto start = std::chrono::high_resolution_clock::now();
//         for (int i = 0; i < iters; i++)
//         {
//             q.put(TestSize{i});
//         }
//         thread.join();
//
//         auto stop = std::chrono::high_resolution_clock::now();
//
//         ops[i] =
//         iters * 1'000'000 /
//         std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start)
//             .count();
//     }
//
//     for (size_t i = 0; i < trials; i++)
//     {
//         auto q1 = JC_SPSC::cached_spsc<TestSize, 2048>();
//         auto q2 = JC_SPSC::cached_spsc<TestSize, 2048>();
//
//         auto thread = std::thread([&]() {
//             set_thread_affinity(2);
//             for (size_t i = 0; i < iters; i++)
//             {
//                 TestSize val = q1.read();
//                 q2.put(std::move(val));
//             }
//         });
//
//         set_thread_affinity(8);
//         auto start = std::chrono::high_resolution_clock::now();
//         for (int i = 0; i < iters; i++)
//         {
//             q1.put(TestSize{i});
//             TestSize val = q2.read();
//         }
//
//         thread.join();
//         auto stop = std::chrono::high_resolution_clock::now();
//
//         rtt[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / iters;
//     }
//
//     std::sort(ops.begin(), ops.end());
//     std::sort(rtt.begin(), rtt.end());
//
//     std::println("Mean: {} ops/ms", std::accumulate(ops.begin(), ops.end(), 0) / trials);
//     std::println("Median: {} ops/ms", ops[trials / 2]);
//     std::println("Mean: {} RTT in ns", std::accumulate(rtt.begin(), rtt.end(), 0) / trials);
//     std::println("Mean: {} RTT in ns", rtt[trials / 2]);
//
//     return 0;
// }