#pragma once

#include "DW1Random.hpp"

enum class Input : uint8_t
{
    RAISE,
    RAISE_CANCEL,
    NORMAL,
    NORMAL_CANCEL,
    LOWER,
    LOWER_CANCEL,
    CATCH_UP,
};

enum class CustomerType : uint8_t
{
    GOBURIMON,
    GOTSUMON,
    WEEDMON,
    MUCHOMON,

    INVALID,
};

enum class Item : uint8_t
{
    MEAT,
    PORT_POTTY,
    MEDICINE,

    INVALID,
};

enum class Offer : uint8_t
{
    PLUS_50,
    PLUS_40,
    PLUS_30,
    PLUS_20,
    PLUS_10,
    NORMAL,
    MINUS_10,
    MINUS_20,
    MINUS_30,
};

enum class Result : uint8_t
{
    BUY,
    DENY,
    LEAVE,
};

enum class InputResult : uint8_t
{
    ADVANCE,
    CANCEL,
    DENY,
    BUY,
    BUY_ENDED,
    LEAVE,
    LEAVE_ENDED,
};

struct Customer
{
    CustomerType type;
    Item item;
    uint32_t fails;
};

class MonochromeShop
{
private:
    uint32_t initial_seed;
    DW1Random rng;

    int32_t remainingCustomers = 20;
    uint32_t profit            = 0;
    Customer currentCustomer;
    bool ended = false;

public:
    explicit MonochromeShop(DW1Random rng);
    explicit MonochromeShop(uint32_t seed, uint32_t advances = 0);

    InputResult input(Input input);

    const Customer& getCustomer() const;
    bool hasEnded() const;
    uint32_t getProfits() const;
    uint32_t getRemainingCustomers() const;
    uint32_t getInitialSeed() const;

private:
    void nextCustomer();
    Result makeOffer(Offer offer);
};

uint32_t getProfit(Item item, Offer offer);