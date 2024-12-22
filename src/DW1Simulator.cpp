#include "FullSolver.hpp"
#include "MonochromeShop.hpp"

#include <boost/program_options.hpp>
#include <signal.h>
#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

std::atomic_bool stop = false;

std::string convertInput(Input input)
{
    switch (input)
    {
        case Input::LOWER: return "LOWER";
        case Input::LOWER_CANCEL: return "LOWER_CANCEL";
        case Input::RAISE_CANCEL: return "RAISE_CANCEL";
        case Input::RAISE: return "RAISE";
        case Input::NORMAL_CANCEL: return "NORMAL_CANCEL";
        case Input::NORMAL: return "NORMAL";
        case Input::CATCH_UP: return "CATCH_UP";
    }

    return "SOMETHING BROKE";
}

std::string convertResult(InputResult result)
{
    switch (result)
    {
        case InputResult::BUY: return "BUY";
        case InputResult::BUY_ENDED: return "BUY_ENDED";
        case InputResult::LEAVE_ENDED: return "LEAVE_ENDED";
        case InputResult::LEAVE: return "LEAVE";
        case InputResult::DENY: return "DENY";
        case InputResult::CANCEL: return "CANCEL";
        case InputResult::ADVANCE: return "ADVANCE";
    }

    return "SOMETHING BROKE";
}

std::string convertCustomerType(CustomerType type)
{
    switch (type)
    {
        case CustomerType::GOBURIMON: return "GOBURIMON";
        case CustomerType::GOTSUMON: return "GOTSUMON";
        case CustomerType::MUCHOMON: return "MUCHOMON";
        case CustomerType::WEEDMON: return "WEEDMON";
        case CustomerType::INVALID: return "INVALID";
    }

    return "SOMETHING BROKE";
}

std::string convertItem(Item type)
{
    switch (type)
    {
        case Item::MEAT: return "MEAT";
        case Item::MEDICINE: return "MEDICINE";
        case Item::PORT_POTTY: return "PORT_POTTY";
        case Item::INVALID: return "INVALID";
    }

    return "SOMETHING BROKE";
}

/*
    1. set upper limit for depth (30)
    2. do all iterations for depth N (10), abort when max depth is reached,
       the end is reached or the best possible score is higher than the best
    3. sort by score
    4. pick lowest score, go back to 2 until either max depth is reached, the




    0. pick lowest priority score from list
    1. perform NextInputs on it
    2. if result is "ended", compare with best score and replace it if it is better
    3. else put all still valid iterations into the queue for next iteration
    4. go back to 0. until that list is empty

NextInputs
    0. check is shop is ended
    1. check if known best can still be beaten
    2. perform RAISE_CANCEL and NORMAL input
    3. perform RAISE input and store result
    4. if result of RAISE is not BUY, perform LOWER input



*/

void deepSolve(FullSolveEntry root, BestResult& best_result)
{
    if (stop) return;

    std::vector<FullSolveEntry> active_entries = { root };
    std::vector<FullSolveEntry> next_iteration;

    for (uint32_t i = 0; i < SOLVE_DEPTH; i++)
    {
        for (auto& entry : active_entries)
        {
            auto next_inputs = entry.next(best_result);
            next_iteration.insert(next_iteration.end(), next_inputs.begin(), next_inputs.end());
        }
        std::swap(active_entries, next_iteration);
        next_iteration.clear();
    }

    std::sort(active_entries.begin(), active_entries.end());

    for (auto& entry : active_entries)
        deepSolve(entry, best_result);
}

void heuristicSolve(uint32_t seed, uint32_t attempts, uint32_t advances, BestResult& best_result)
{
    if (stop) return;

    for (uint32_t j = 0; j < attempts; j++)
        HeuristicSolveEntry(seed, advances).next(best_result);
}

enum class Mode
{
    DEEP,
    HEURISTIC,
    COMBINED,
};

BestResult solve(uint32_t seed,
                 uint32_t maxAdvances        = 0,
                 uint32_t heuristic_attempts = 0,
                 uint32_t minimumScore       = IMPOSSIBLE_SCORE,
                 Mode mode                   = Mode::COMBINED)
{
    BestResult result(minimumScore);

    std::vector<std::thread> threads;

    if (mode == Mode::COMBINED || mode == Mode::HEURISTIC)
    {
        for (uint32_t i = 0; i <= maxAdvances; i++)
            threads.emplace_back(heuristicSolve, seed, heuristic_attempts, i, std::ref(result));
    }

    if (mode == Mode::COMBINED || mode == Mode::DEEP)
    {
        for (uint32_t i = 0; i <= maxAdvances; i++)
            threads.emplace_back(deepSolve, FullSolveEntry(seed, i), std::ref(result));
    }

    auto signalHandler = [](BestResult& result, int signal) { result.abort(); };
    auto a             = std::bind_front(signalHandler, result);

    std::for_each(threads.begin(), threads.end(), [](auto& a) { a.join(); });

    if (result.getBest().has_value())
    {
        for (auto val : result.getBest()->getInputs()) // bestResult.sequence)
        {
            std::cout << std::format("{:12} -> {:12} | {:12} {}\n",
                                     convertInput(val.input),
                                     convertResult(val.result),
                                     convertCustomerType(val.customer),
                                     convertItem(val.item));
        }
        std::cout << "Customers: " << result.getBest()->getCustomerCount() << std::endl;
        std::cout << "   Inputs: " << result.getBest()->getInputs().size() << std::endl;
        std::cout << "   Profit: " << result.getBest()->getShop().getProfits() << std::endl;
        std::cout << "    Score: " << result.getScore() << std::endl;
        std::cout << "     Seed: " << result.getBest()->getShop().getInitialSeed() << std::endl;
    }

    return result;
}

Mode convertMode(std::string input)
{
    if (input == "combined") return Mode::COMBINED;
    if (input == "deep") return Mode::DEEP;
    if (input == "heuristic") return Mode::HEURISTIC;

    return Mode::COMBINED;
}

int main(int count, char* args[])
{
    std::signal(SIGINT,
                [](int signal)
                {
                    stop = true;
                    std::cout << "Aborted\n";
                });

    constexpr uint32_t DEFAULT_ADVANCES = 4;
    constexpr uint32_t DEFAULT_ATTEMPTS = 5000000;
    constexpr auto DEFAULT_MODE         = "combined";

    // DW1Random random(845126); // absolute dogshit
    // DW1Random random(3580817068U); // best known
    // DW1Random random(1508351762);
    // DW1Random random(4063756419);
    // DW1Random random(2393675181);
    // DW1Random random(1462898466);

    namespace po = boost::program_options;
    po::variables_map vm;
    po::positional_options_description pos;
    po::options_description desc("Usage: DW1Simulator <seed> [options]\n"
                                 "Use ctrl+C to abort.",
                                 120);

    auto options = desc.add_options();
    options("help,h", "This text.");
    options("seed", po::value<uint32_t>(), "The initial seed for the shop, taken when talking to Monochromon.");
    options("mode,m",
            po::value<std::string>()->default_value(DEFAULT_MODE),
            "The solver mode used. Valid: combined|deep|heuristic\n"
            "combined -> use heuristic and deep solver in parallel, finds lowest score\n"
            "            heuristic is to find a quick base value, to speed up the deep solve\n"
            "            might take several minutes, depending on the seed!\n"
            "deep -> use deep solver exclusively, finds lowest score\n"
            "        not recommended over combined, unless you use the --score option\n"
            "        might take several minutes, depending on the seed!\n"
            "heuristic -> use heuristic solver, doesn't find lowest score\n"
            "             not recommended, unless you want a result quickly\n"
            "             fast, unless you turn heuristic_attempts very high");
    options("score,s",
            po::value<uint32_t>()->default_value(IMPOSSIBLE_SCORE),
            "Initial \"best\" score, ignores any result worse than that.\n"
            "Setting this can allow deep search to faster rule out slow paths,\n"
            "but might yield no result at all when there is no better path.\n"
            "Useful when comparing multiple seeds.");
    options("advances,a",
            po::value<uint32_t>()->default_value(DEFAULT_ADVANCES),
            "The maximum number of advances from the base seed to check.\n"
            "Time loss from advancing is taken into account.\n"
            "Spawns up to 2 threads per advance and thus increases CPU load.\n"
            "Recommended to use, it can reduce execution time significantly.");
    options("attempts",
            po::value<uint32_t>()->default_value(DEFAULT_ATTEMPTS),
            "Number of attempts when using heuristic or combined solver.\n"
            "Rarely finds anything better after 10000000.");

    pos.add("seed", 1);

    po::store(po::command_line_parser(count, args).options(desc).positional(pos).allow_unregistered().run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc;
        return 1;
    }
    /*if (!vm.count("seed"))
    {
        std::cout << "You must specify a seed!\n";
        std::cout << desc;
        return 1;
    }*/

    uint32_t advances           = 0;           // vm["advances"].as<uint32_t>();
    uint32_t heuristic_attempts = 0;           // vm["attempts"].as<uint32_t>();
    uint32_t seed               = 3580817068U; // vm["seed"].as<uint32_t>();
    uint32_t score              = vm["score"].as<uint32_t>();
    Mode mode                   = convertMode(vm["mode"].as<std::string>());

    auto start      = std::chrono::high_resolution_clock::now();
    BestResult best = solve(seed, advances, heuristic_attempts, score, mode);

    std::cout << "finished" << std::endl;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() -
                                                                       start)
              << std::endl;

    FullSolveEntry root(270672212, 0);
    std::vector<Input> inputs;
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::LOWER);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE_CANCEL);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::NORMAL);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE);
    // inputs.push_back(Input::RAISE);

    // 1508351762
    inputs.push_back(Input::RAISE_CANCEL);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE_CANCEL);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::NORMAL);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE_CANCEL);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE_CANCEL);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE);
    inputs.push_back(Input::RAISE_CANCEL);
    inputs.push_back(Input::RAISE);

    // 3580817068U
    //inputs.push_back(Input::RAISE_CANCEL);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE_CANCEL);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE_CANCEL);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE_CANCEL);
    //inputs.push_back(Input::RAISE_CANCEL);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);
    //inputs.push_back(Input::RAISE);

    FullSolveEntry r(1508351762, 0);
    for (Input i : inputs)
        r = FullSolveEntry(r, i);

    std::cout << r.getScore() << " \n";
    // std::cout << step23.getScore() << " \n";
}