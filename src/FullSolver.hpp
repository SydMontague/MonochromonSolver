#pragma once
#include "MonochromeShop.hpp"

#include <atomic>
#include <mutex>
#include <optional>
#include <random>
#include <vector>

// CANCEL -> 16 frames
// MEAT -> 10 frames on first entry
// DENY -> 36 frames
// NON_RAISE -> 5 frames

// - WALK_IN
// - WALK_OUT
// - LOADING
// - REQUEST
// - PICK_OPTION
// - OFFER_CANCEL
// -

// 110 static per customer (loading, menus, delays)
// + walk in/out per customer type
// + meat penalty per customer
// + action cost (per action)
// + deny cost (if applicable)
// - muchomon bonus (if applicable)

// GOTS MEDICINE CANCEL BUY
// Spawn: 25:11:12
// Last Frame: 25:21:81
// Total: 214 frames
//      50 frames in
//      21 frames for request
//      15 frames raise cancel
//      12 frames to raise offer
//      50 frames to accept and start walking
//      23 frames out
//      40 frames wait+loading

// MUCH MEDICINE BUY
// Spawn: 23:59:86
// Last Frame: 24:09:26
// 188 frames total
//      48 frames walk-in
//      23 frames for request
//      2 frames to pick option
//      9 frames to offer/cancel
//      1 frame to pick offer/cancel
//      40 frames to accept and start walking
//      22 frames to walk and disappear
//      18 frames wait after disappear
//      25 frames loading

// GOTS MEAT DENY LEAVE
// Spawn: 23:36:63
// Last Frame: 23:49:44
// 256 total frames
//      73 walking in+out
//      36 DENY
//      20 MEAT
//  127 frames unaccounted
//
// 50 frames walk-in
// 31 frames for request (+10 because of meat)
// 3 frames to pick option
// 7 frames to offer/cancel
// 2 frames to pick offer/cancel
// 36 frames to deny and re-offer
// 2 frames to pick option
// 7 frames to offer/cancel
// 1 frame to pick offer/cancel
// 21 frames to refuse and start walking
// 24 frames to walk out
// 49 frames wait after walk out
// 21 frames loading

// GOBU PORT POTTY BUY
// Spawn: 24:50:24
// Last Frame: 25:00:65
// 208 total frames
//   82 walking in+out
//
// 126 frames unaccounted
//
// 54 frames walking in
// 22 frames for request
// 3 frames to confirm option
// 9 frames to offer/cancel
// 2 frames to pick offer/cancel
// 50 frames to accept and start walking
// 28 frames to walk out
// 18 frames wait after disappear
// 22 frames loading

// GOTS POTTY LOWER_BUY
// Spawn: 25:00:70
// Last Frame: 25:11:09
//   208 total frames
//      73 frames walking in+out
//      5 frames LOWER
//
// 130 frames unaccounted
//
// 50 frames walking in
// 20 frames for request
// 5 frames to pick option
// 2 frames for confirm option
// 9 frames to offer/cancel
// 8 frames to pick offer/cancel
// 50 frames to accept and start walking
// 23 frames walking
// 20 frames wait after disappear
// 20 frames loading

// 53.32 -> 53.68 0.36
// 12.75 -> 13.11 0.36

// 28.73 29.53 -> 0.8 -> 16 frames
// 40.90 43.24 -> 2.3 -> 46 frames - 10 frame meat?

// Walking times
// GOBR: 4.10s | 2.70s in | 1.40s out
// GOTS: 3.65s | 2.50s in | 1.15s out
// MUCH: 3.55s | 2.40s in | 1.15s out
// WEED: 3.25s | 2.25s in | 1.00s out

// GOTS Spawn to Textbox open -> 2.5s
// GOTS Leave to Despawn -> 1.15s
// GOBR Spawn to Textbox open -> 2.7s
// GOBR Leave to Despawn -> 1.4s
// GOTS Spawn to Textbox open -> 2.5s
// GOTS Leave to Despawn -> 1.15s
// GOBR Spawn to Textbox open -> 2.7s
// GOBR Leave to Despawn -> 1.35s
// MUCH Spawn to Textbox open -> 2.4s
// MUCH Leave to Despawn -> 1.15s
// WEED Spawn to Textbox open -> 2.25s
// WEED Leave to Despawn -> 1s
// WEED Spawn to Textbox open -> 2.20s
// WEED Leave to Despawn -> 0.95s
// GOBR Spawn to Textbox open -> 2.65s
// GOBR Leave to Despawn -> 1.4s
// MUCH Spawn to Textbox open -> 2.4s
// MUCH Leave to Despawn -> 1.15s

// First frame with "I want to buy X" visible to first movement frame
// 3,5  02,50 -> 05,98 MUCH buying Medicine (RAISE)
// 3,4  43,81 -> 47,16 MUCH buying Medicine (RAISE)
// 4    53,15 -> 57,19 GOBR buying Port. Potty (RAISE)
// 4,3  36,95 -> 41,28 MUCH buying Medicine (CANCEL RAISE)
// 5,8  47,13 -> 52,95 GOTS buying Port. Potty (DENY RAISE)
// 5,4  39,38 -> 44,74 GOTS buying Meat (DENY LEAVE)

// 21,81

// ~7 frames unload, ~13 frames load

// 15 frames load delay? Start -> GOBR
// 22 frames GOBR -> WEED
// 20 frames WEED -> WEED
// 20 frames WEED -> WEED
// 19 frames WEED -> GOTS
// 20 frames GOTS -> GOTS
// 22 frames GOTS -> MUCH
//

#define USE_NEW
#define DEBUG

#ifdef USE_NEW
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

#else
constexpr uint32_t BUY_COST                         = 130;
constexpr uint32_t LEAVE_COST                       = 140;
constexpr uint32_t CANCEL_COST                      = 16;
constexpr uint32_t DENY_COST                        = 36;
constexpr uint32_t MEAT_COST                        = 10;
constexpr uint32_t MUCHOMON_MEDICINE_COST_REDUCTION = 10;
constexpr uint32_t NON_RAISE_COST                   = 5;
constexpr uint32_t ADVANCE_COST                     = 20;
#endif

constexpr uint32_t SOLVE_DEPTH      = 10;
constexpr uint32_t MAX_DEPTH        = 30;
constexpr uint32_t REQUIRED_PROFITS = 3072;
constexpr uint32_t IMPOSSIBLE_SCORE = 99999;

struct SolveSequenceResult
{
    CustomerType customer;
    Item item;
    Input input;
    InputResult result;
    bool isFreshCustomer;

    uint32_t getScore() const;
};

struct ISolveEntry
{
protected:
    uint32_t currentScore  = 0;
    uint32_t customerCount = 0;
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
    int32_t last_score = 0;
private:
    uint32_t getBestPossibleScore() const;

public:
    FullSolveEntry(uint32_t seed, uint32_t advances = 0);

    FullSolveEntry(const FullSolveEntry& previous, Input input);

    std::vector<FullSolveEntry> next(BestResult& best_result) const;
};