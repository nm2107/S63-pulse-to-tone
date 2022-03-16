#include "DialedDigit.h"

DialedDigit::DialedDigit(): value(-1)
{}

bool DialedDigit::isNew() const
{
    return this->value > -1;
}

void DialedDigit::push(unsigned int value)
{
    this->value = (int) value;
}

unsigned int DialedDigit::flush()
{
    unsigned int val = this->value;

    this->value = -1;

    return val;
}
