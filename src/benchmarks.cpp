#include "matrix.h"
#include <benchmark/benchmark.h>

static void BM_TransposeNaive(benchmark::State& state) {
    std::size_t size { static_cast<std::size_t>(state.range(0)) };

    Matrix<double> A { size, size };
    A.fillMatrix();

    for (auto _ : state) {
        benchmark::DoNotOptimize(A.transpose_naive());
    }
}
BENCHMARK(BM_TransposeNaive)->RangeMultiplier(2)->Range(32, 4096);

static void BM_TransposeTiled(benchmark::State& state) {
    std::size_t size { static_cast<std::size_t>(state.range(0)) };

    Matrix<double> A { size, size };
    A.fillMatrix();

    for (auto _ : state) {
        benchmark::DoNotOptimize(A.transpose_tiled());
    }
}
BENCHMARK(BM_TransposeTiled)->RangeMultiplier(2)->Range(32, 4096);

static void BM_MultiplyNaive(benchmark::State& state) {
    std::size_t size { static_cast<std::size_t>(state.range(0)) };

    Matrix<double> A { size, size };
    Matrix<double> B { size, size };
    A.fillMatrix();
    B.fillMatrix();

    for (auto _ : state) {
        benchmark::DoNotOptimize(A.multiply_naive(B));
    }
}
BENCHMARK(BM_MultiplyNaive)->RangeMultiplier(2)->Range(32, 1024);

static void BM_MultiplyReordered(benchmark::State& state) {
    std::size_t size { static_cast<std::size_t>(state.range(0)) };

    Matrix<double> A { size, size };
    Matrix<double> B { size, size };
    A.fillMatrix();
    B.fillMatrix();

    for (auto _ : state) {
        benchmark::DoNotOptimize(A.multiply_reordered(B));
    }
}
BENCHMARK(BM_MultiplyReordered)->RangeMultiplier(2)->Range(32, 2048);

static void BM_MultiplyTransposed(benchmark::State& state) {
    std::size_t size { static_cast<std::size_t>(state.range(0)) };

    Matrix<double> A { size, size };
    Matrix<double> B { size, size };
    A.fillMatrix();
    B.fillMatrix();
    B = B.transpose_tiled();

    for (auto _ : state) {
        benchmark::DoNotOptimize(A.multiply_transposed(B));
    }
}
BENCHMARK(BM_MultiplyTransposed)->RangeMultiplier(2)->Range(32, 1024);

static void BM_MultiplyTiledReordered(benchmark::State& state) {
    std::size_t size { static_cast<std::size_t>(state.range(0)) };

    Matrix<double> A { size, size };
    Matrix<double> B { size, size };
    A.fillMatrix();
    B.fillMatrix();

    for (auto _ : state) {
        benchmark::DoNotOptimize(A.multiply_tiled_reordered(B));
    }
}
BENCHMARK(BM_MultiplyTiledReordered)->RangeMultiplier(2)->Range(32, 2048);

BENCHMARK_MAIN();
