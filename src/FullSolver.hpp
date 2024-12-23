#pragma once
#include "MonochromeShop.hpp"

#include <atomic>
#include <mutex>
#include <optional>
#include <random>
#include <vector>

constexpr uint32_t RAISE_COST              = 12;
constexpr uint32_t NORMAL_COST             = 17;
constexpr uint32_t LOWER_COST              = 17;
constexpr uint32_t CANCEL_COST             = 16;
constexpr uint32_t STATIC_COST             = 100; // spawn + request + react + wait
constexpr uint32_t DENY_COST               = 36;
constexpr uint32_t MEAT_COST               = 10;
constexpr uint32_t NON_MUCHO_MEDICINE_COST = 10;
constexpr uint32_t ADVANCE_COST            = 20;
constexpr uint32_t GOBURIMON_WALK_COST     = 82;
constexpr uint32_t GOTSUMON_WALK_COST      = 73;
constexpr uint32_t MUCHOMON_WALK_COST      = 71;
constexpr uint32_t WEEDMON_WALK_COST       = 65;


constexpr int32_t SOLVE_DEPTH      = 10;
constexpr uint32_t DEFAULT_DEPTH   = 30;
constexpr int32_t REQUIRED_PROFITS = 3072;
constexpr int32_t IMPOSSIBLE_SCORE = 99999;

struct SolveSequenceResult
{
    CustomerType customer;
    Item item;
    Input input;
    InputResult result;

    uint32_t getScore() const;
};

struct ISolveEntry
{
protected:
    uint32_t currentScore        = 0;
    uint32_t customerCount       = 0;
    uint32_t best_possible_score = 0;
    MonochromeShop shop;
    std::vector<SolveSequenceResult> inputs;

public:
    explicit ISolveEntry(uint32_t seed, uint32_t advances = 0);

    [[nodiscard]] uint32_t getScore() const;
    [[nodiscard]] uint32_t getCustomerCount() const;
    [[nodiscard]] std::vector<SolveSequenceResult> getInputs() const;
    [[nodiscard]] MonochromeShop getShop() const;
    [[nodiscard]] bool operator<(const ISolveEntry& other) const;
};

struct BestResult
{
private:
    std::atomic_uint32_t score;
    std::optional<ISolveEntry> node;
    std::mutex nodeMutex;

public:
    BestResult(uint32_t initScore = IMPOSSIBLE_SCORE);
    BestResult(const BestResult& other);

    void updateScore(ISolveEntry entry);

    uint32_t getScore() const;
    std::optional<ISolveEntry> getBest() const;

    void abort();
};

struct HeuristicSolveEntry : public ISolveEntry
{
private:
    std::mt19937_64 rng;

    Input rollInput();

public:
    HeuristicSolveEntry(uint32_t seed, uint32_t advances = 0);

    void next(BestResult& best_result);
};

struct FullSolveEntry : public ISolveEntry
{
private:
    uint32_t getBestPossibleScore() const;

public:
    FullSolveEntry(uint32_t seed, uint32_t advances = 0);

    FullSolveEntry(const FullSolveEntry& previous, Input input);

    std::vector<FullSolveEntry> next(BestResult& best_result) const;
};