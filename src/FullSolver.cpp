#include "FullSolver.hpp"

#include "MonochromeShop.hpp"

#include <iostream>

constexpr int32_t ceilDiv(int32_t val1, int32_t val2) { return (val1 + val2 - 1) / val2; }

uint32_t SolveSequenceResult::getScore() const
{
    uint32_t score = 0;
    switch (input)
    {
        case Input::RAISE: score += RAISE_COST; break;
        case Input::NORMAL: score += NORMAL_COST; break;
        case Input::LOWER: score += LOWER_COST; break;
        case Input::RAISE_CANCEL: score += CANCEL_COST; break;
        case Input::CATCH_UP: score += ADVANCE_COST; break;
        default: break;
    }

    switch (result)
    {
        case InputResult::ADVANCE: break;
        case InputResult::BUY:       // fall-through
        case InputResult::BUY_ENDED: // fall-through
            if (item == Item::MEDICINE && customer == CustomerType::MUCHOMON) score -= NON_MUCHO_MEDICINE_COST;
            // conditional fall-though
        case InputResult::LEAVE: // fall-through
        case InputResult::LEAVE_ENDED:
            score += NON_MUCHO_MEDICINE_COST;
            if (item == Item::MEAT) score += MEAT_COST;
            score += STATIC_COST;
            switch (customer)
            {
                case CustomerType::MUCHOMON: score += MUCHOMON_WALK_COST; break;
                case CustomerType::GOBURIMON: score += GOBURIMON_WALK_COST; break;
                case CustomerType::GOTSUMON: score += GOTSUMON_WALK_COST; break;
                case CustomerType::WEEDMON: score += WEEDMON_WALK_COST; break;
                default: break;
            }
            break;
        case InputResult::CANCEL: break;
        case InputResult::DENY: score += DENY_COST; break;
    }

    return score;
}

ISolveEntry::ISolveEntry(uint32_t seed, uint32_t advances)
    : shop(seed, advances)
{
    for (uint32_t i = 0; i < advances; i++)
    {
        SolveSequenceResult res = {
            .customer = CustomerType::INVALID,
            .item     = Item::INVALID,
            .input    = Input::CATCH_UP,
            .result   = InputResult::ADVANCE,
        };
        currentScore += ADVANCE_COST;
        inputs.push_back(res);
    }
}

uint32_t ISolveEntry::getScore() const { return currentScore; };
uint32_t ISolveEntry::getCustomerCount() const { return customerCount; }
std::vector<SolveSequenceResult> ISolveEntry::getInputs() const { return inputs; }
MonochromeShop ISolveEntry::getShop() const { return shop; }
bool ISolveEntry::operator<(const ISolveEntry& other) const { return best_possible_score < other.best_possible_score; }

uint32_t FullSolveEntry::getBestPossibleScore() const
{
    constexpr int32_t BUY_COST    = STATIC_COST + MUCHOMON_WALK_COST + RAISE_COST;
    constexpr int32_t LEAVE_COST  = STATIC_COST + WEEDMON_WALK_COST + RAISE_COST + NON_MUCHO_MEDICINE_COST;
    constexpr auto MAXIMUM_PROFIT = 800;

    uint32_t currentScore = getScore();
    if (shop.hasEnded()) return currentScore;

    int32_t remainingProfit    = REQUIRED_PROFITS - shop.getProfits();
    int32_t remainingCustomers = shop.getRemainingCustomers() + 1;
    int32_t maxProfit          = getProfit(shop.getCustomer().item, Offer::PLUS_50);
    auto minMedicine           = ceilDiv(remainingProfit, MAXIMUM_PROFIT);

    // we can't get enough profit anymore -> dead path
    if (minMedicine > remainingCustomers) return IMPOSSIBLE_SCORE;

    remainingCustomers -= minMedicine;
    uint32_t newScore = currentScore;
    newScore += (remainingCustomers / 2) * LEAVE_COST;
    newScore += (minMedicine + remainingCustomers % 2) * BUY_COST;

    return newScore;
}

FullSolveEntry::FullSolveEntry(uint32_t seed, uint32_t advances)
    : ISolveEntry(seed, advances)
{
    best_possible_score = getBestPossibleScore();
}

FullSolveEntry::FullSolveEntry(const FullSolveEntry& previous, Input input)
    : ISolveEntry(previous)
{
    SolveSequenceResult res;
    res.input    = input;
    res.customer = shop.getCustomer().type;
    res.item     = shop.getCustomer().item;
    res.result   = shop.input(input);

    currentScore += res.getScore();

    switch (res.result)
    {
        case InputResult::BUY_ENDED:
        case InputResult::LEAVE_ENDED:
        case InputResult::BUY:
        case InputResult::LEAVE: customerCount++; break;
        default: break;
    }
    inputs.push_back(res);
    best_possible_score = getBestPossibleScore();
}

std::vector<FullSolveEntry> FullSolveEntry::next(BestResult& best_result) const
{
    if (shop.hasEnded())
    {
        if (shop.getProfits() >= REQUIRED_PROFITS && currentScore < best_result.getScore())
            best_result.updateScore(*this);

        return {};
    }

    if (best_possible_score >= best_result.getScore()) { return {}; }

    std::vector<FullSolveEntry> entries;

    entries.emplace_back(*this, Input::RAISE_CANCEL);
    entries.emplace_back(*this, Input::NORMAL);
    entries.emplace_back(*this, Input::RAISE);

    // if raise results in a buy, then a lower will also guarantee a buy
    auto result = entries.rbegin()->inputs.rbegin()->result;
    if (result != InputResult::BUY && result != InputResult::BUY_ENDED) entries.emplace_back(*this, Input::LOWER);

    return entries;
}

BestResult::BestResult(uint32_t initScore)
    : score(initScore)
{
}
BestResult::BestResult(const BestResult& other)
{
    score = other.getScore();
    node  = other.getBest();
}

void BestResult::updateScore(ISolveEntry entry)
{
    std::scoped_lock lock(nodeMutex);

    uint32_t newScore = entry.getScore();
    if (newScore >= score) return;

    score = newScore;
    node  = entry;
    std::cout << "New best: " << score << "\n";
}

uint32_t BestResult::getScore() const { return score; }
std::optional<ISolveEntry> BestResult::getBest() const { return node; }
void BestResult::abort() { score = 0; }

Input HeuristicSolveEntry::rollInput()
{
    uint32_t roll = rng() % 100;
    if (roll < 60)
        return Input::RAISE;
    else if (roll < 80)
        return Input::RAISE_CANCEL;
    else if (roll < 90)
        return Input::NORMAL;
    else
        return Input::LOWER;
}

HeuristicSolveEntry::HeuristicSolveEntry(uint32_t seed, uint32_t advances)
    : ISolveEntry(seed, advances)
    , rng(std::chrono::high_resolution_clock::now().time_since_epoch().count())
{
}
void HeuristicSolveEntry::next(BestResult& best_result)
{
    while (!shop.hasEnded())
    {
        SolveSequenceResult result;
        result.item     = shop.getCustomer().item;
        result.customer = shop.getCustomer().type;
        result.input    = rollInput();
        result.result   = shop.input(result.input);
        currentScore += result.getScore();

        switch (result.result)
        {
            case InputResult::BUY_ENDED:
            case InputResult::LEAVE_ENDED:
            case InputResult::BUY:
            case InputResult::LEAVE: customerCount++; break;
            default: break;
        }
        inputs.push_back(result);
    }

    if (shop.getProfits() >= REQUIRED_PROFITS && currentScore < best_result.getScore()) best_result.updateScore(*this);
}