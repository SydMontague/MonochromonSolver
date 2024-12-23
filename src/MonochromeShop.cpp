
#include "MonochromeShop.hpp"

#include <array>
#include <cassert>
#include <tuple>

/*
 * Static data access
 */

struct CustomerData
{
    uint32_t chanceMeat;
    uint32_t chancePotty;
    uint32_t chanceMedicine;

    std::array<uint32_t, 9> lookupMeat;
    std::array<uint32_t, 9> lookupPotty;
    std::array<uint32_t, 9> lookupMedicine;
};

struct ItemChances
{
    uint32_t chanceMeat;
    uint32_t chancePotty;
    uint32_t chanceMedicine;
};

constexpr std::array<ItemChances, 4> itemChances = {
    {
        { 35, 50, 15 },
        { 30, 55, 15 },
        { 50, 35, 15 },
        { 32, 35, 33 },
    },
};

constexpr std::array<std::array<std::array<uint32_t, 9>, 3>, 4> takeChances = {
    {
        // Goburimon
        {
            {
                { { 45, 55, 65, 75, 85, 95, 97, 99, 100 } },
                { { 45, 55, 65, 75, 85, 95, 97, 99, 100 } },
                { { 45, 55, 65, 75, 85, 95, 97, 99, 100 } },
            },
        },
        // Gotsumon
        {
            {
                { { 1, 10, 30, 50, 70, 90, 92, 94, 96 } },
                { { 1, 10, 30, 50, 70, 90, 92, 94, 96 } },
                { { 1, 10, 30, 50, 70, 90, 92, 94, 96 } },
            },
        },
        // Weedmon
        {
            {
                { { 1, 1, 1, 20, 50, 80, 85, 90, 95 } },
                { { 1, 1, 1, 20, 50, 80, 85, 90, 95 } },
                { { 2, 2, 2, 20, 50, 80, 85, 90, 95 } },
            },
        },
        // Muchomon
        {
            {
                { { 45, 50, 55, 60, 65, 70, 75, 80, 85 } },
                { { 45, 50, 55, 60, 65, 70, 75, 80, 85 } },
                { { 45, 50, 55, 60, 65, 70, 75, 80, 85 } },
            },
        },
    },
};

constexpr std::array<std::array<uint32_t, 3>, 4> leaveChances = {
    {
        { { 5, 15, 50 } },
        { { 10, 30, 70 } },
        { { 20, 60, 90 } },
        { { 50, 50, 100 } },
    },
};

constexpr std::array<std::array<uint32_t, 9>, 3> profits = {
    {
        { { 40, 35, 30, 25, 20, 15, 10, 5, 0 } },
        { { 240, 210, 180, 150, 120, 90, 60, 30, 0 } },
        { { 800, 700, 600, 500, 400, 300, 200, 100, 0 } },
    },
};

constexpr uint32_t getLeaveChance(CustomerType type, uint32_t fails)
{
    return leaveChances[static_cast<int>(type)][std::min(fails, 2U)];
}

constexpr uint32_t getBuyChance(CustomerType customer, Item item, Offer offer)
{
    return takeChances[static_cast<int>(customer)][static_cast<int>(item)][static_cast<int>(offer)];
}

constexpr CustomerType getCustomerType(uint32_t roll)
{
    if (roll <= 2)
        return CustomerType::GOBURIMON;
    else if (roll <= 5)
        return CustomerType::GOTSUMON;
    else if (roll <= 7)
        return CustomerType::WEEDMON;
    else
        return CustomerType::MUCHOMON;
}

constexpr Item getCustomerItem(CustomerType customer, uint32_t roll)
{
    auto& chances = itemChances[static_cast<int>(customer)];

    if (roll < chances.chanceMeat)
        return Item::MEAT;
    else if (roll < chances.chanceMeat + chances.chancePotty)
        return Item::PORT_POTTY;
    else
        return Item::MEDICINE;
}

uint32_t getProfit(Item item, Offer offer) { return profits[static_cast<int>(item)][static_cast<int>(offer)]; }

/*
 * MonochromeShop Public Methods
 */

MonochromeShop::MonochromeShop(DW1Random rng)
    : rng(rng)
    , initial_seed(rng.getState())
{
    nextCustomer();
}

MonochromeShop::MonochromeShop(uint32_t seed, uint32_t advances)
    : rng(seed)
{
    rng.advance(advances);
    initial_seed = rng.getState();
    nextCustomer();
}

InputResult MonochromeShop::input(Input input)
{
    Offer offer;

    switch (input)
    {
        case Input::CATCH_UP:
        case Input::RAISE_CANCEL:
        case Input::LOWER_CANCEL: std::ignore = rng.next(); // ignore result and bounds, only the advance counts
        case Input::NORMAL_CANCEL:                          // fall-through
            return InputResult::CANCEL;

        case Input::RAISE: offer = static_cast<Offer>(rng.next(5)); break;
        case Input::NORMAL: offer = Offer::NORMAL; break;
        case Input::LOWER: offer = static_cast<Offer>(static_cast<int>(Offer::MINUS_10) + rng.next(3)); break;
    }

    Result result = makeOffer(offer);

    switch (result)
    {
        case Result::DENY: return InputResult::DENY;
        case Result::BUY:
            profit += getProfit(currentCustomer.item, offer);

            if (remainingCustomers <= 0)
            {
                ended = true;
                return InputResult::BUY_ENDED;
            }
            nextCustomer();
            return InputResult::BUY;
        case Result::LEAVE:
            remainingCustomers--;

            if (remainingCustomers <= 0)
            {
                ended = true;
                return InputResult::LEAVE_ENDED;
            }
            nextCustomer();
            return InputResult::LEAVE;
    }

    assert(true);
    return InputResult::CANCEL;
    // if you got here, something went wrong
}

const Customer& MonochromeShop::getCustomer() const { return currentCustomer; }

uint32_t MonochromeShop::getProfits() const { return profit; }

uint32_t MonochromeShop::getRemainingCustomers() const { return remainingCustomers; }

bool MonochromeShop::hasEnded() const { return ended; }

uint32_t MonochromeShop::getInitialSeed() const { return initial_seed; }

/*
 * MonochromeShop Private Methods
 */

void MonochromeShop::nextCustomer()
{
    CustomerType type = getCustomerType(rng.next(9));
    Item item         = getCustomerItem(type, rng.next(100));
    remainingCustomers--;
    currentCustomer = { type, item, 0 };
}

Result MonochromeShop::makeOffer(Offer offer)
{
    uint32_t buyChance = getBuyChance(currentCustomer.type, currentCustomer.item, offer);
    uint32_t buyRoll   = rng.next(100);

    if (buyRoll < buyChance) return Result::BUY;

    uint32_t leaveChance = getLeaveChance(currentCustomer.type, currentCustomer.fails++);
    uint32_t leaveRoll   = rng.next(100);

    if (leaveRoll < leaveChance) return Result::LEAVE;

    return Result::DENY;
}