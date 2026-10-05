#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <cmath>
#include <string>

using json = nlohmann::json;

struct Result {
    std::string implementation {};
    std::size_t matrix_size {};
    int threads {};
    double cpu_time_ms {};
    std::string baseline {};
    double speedup_vs_baseline {};
    double parallel_speedup;
};

int main(int argc, char* argv[]) {
    if (argc < 3 || argc % 2 == 0) {
        std::cerr << "Usage: matrix_analysis <json> <threads> [<json> <threads> ...]\n";
        return 1;
    }

    std::vector<Result> results;

    for (int i = 1; i < argc; i += 2) {
        std::string json_path = argv[i];
        int threads = std::stoi(argv[i + 1]);

        // open this JSON file
        std::ifstream f(json_path);

        if (!f) {
            std::cerr << "Could not open JSON file\n";
            return 1;
        }

        // parse this JSON file
        json benchmarks = json::parse(f);
        auto& entries = benchmarks["benchmarks"];

        // loop through its benchmark entries
        // push Result objects into results
        for (auto& entry : entries) {
            if (entry["run_type"] == "aggregate" 
                && entry["aggregate_name"] == "median") {
                std::string name = entry["run_name"].get<std::string>();
                std::size_t delimiter = name.find('/');

                std::string implementation = name.substr(0, delimiter);
                std::string size = name.substr(delimiter + 1);
                std::size_t matrix_size = static_cast<std::size_t>(std::stoull(size));      

                if (threads > 1
                    && implementation != "BM_MultiplyTiledReordered") {
                    continue;
                }
                
                double cpu_time = entry["cpu_time"].get<double>();
                double cpu_time_ms = cpu_time / 1'000'000.0;

                Result result { implementation, matrix_size, threads, cpu_time_ms};
                results.push_back(result);
            }
        }
    }

    // AFTER this loop:
    // calculate baselines/speedups

    // Computing speedups from JSON
    for (auto& result : results) {
        double baseline_time {};
        bool found_baseline = false;

        // First try Naive as the baseline
        for (const auto& candidate : results) {
            if (candidate.matrix_size == result.matrix_size
                && candidate.threads == 1
                && candidate.implementation == "BM_MultiplyNaive") {

                result.baseline = candidate.implementation;
                baseline_time = candidate.cpu_time_ms;
                found_baseline = true;
                break;
            }
        }

        // If Naive does not exist for this size, fall back to Reordered
        if (!found_baseline) {
            for (const auto& candidate : results) {
                if (candidate.matrix_size == result.matrix_size
                    && candidate.threads == 1
                    && candidate.implementation == "BM_MultiplyReordered") {

                    result.baseline = candidate.implementation;
                    baseline_time = candidate.cpu_time_ms;
                    found_baseline = true;
                    break;
                }
            }
        }

        // Only calculate speedup if a baseline was actually found
        if (found_baseline) {
            result.speedup_vs_baseline =
                std::round((baseline_time / result.cpu_time_ms) * 100.0) / 100.0;
        }
    }


    for (auto& result : results) {
        if (result.implementation != "BM_MultiplyTiledReordered") {
            continue;
        }

        for (const auto& candidate : results) {
            if (candidate.implementation == result.implementation
                && candidate.matrix_size == result.matrix_size
                && candidate.threads == 1) {

                result.parallel_speedup =
                    std::round(
                        (candidate.cpu_time_ms / result.cpu_time_ms) * 100.0
                    ) / 100.0;

                break;
            }
        }
    }

    // AFTER that:
    // write results.csv
    std::ofstream CSV("results/results.csv");

    if (!CSV) {
        std::cerr << "Could not open CSV file\n";
        return 1;
    }
    
    CSV << "implementation,matrix_size,threads,cpu_time_ms,baseline,"
     << "speedup_vs_baseline,parallel_speedup\n";
    for (auto& result : results) {
        CSV << result.implementation << ","
            << result.matrix_size << ","
            << result.threads << ","
            << result.cpu_time_ms << ","
            << result.baseline << ","
            << result.speedup_vs_baseline << ",";

        if (result.implementation == "BM_MultiplyTiledReordered") {
            CSV << result.parallel_speedup;
        }

        CSV << "\n";
    }

    return 0;
}


