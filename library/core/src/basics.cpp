#include <core/basics.h>

std::ostream& operator<<(std::ostream& os, const Polarity& p) {
    os << (p == Positive ? "+" : "-");
    return os;
}

std::ostream& operator<<(std::ostream& os, const Tier& t) {
    switch (t) {
        case A: {
            os << "A";
            break;
        }
        case B: {
            os << "B";
            break;
        }
        case C: {
            os << "C";
            break;
        }
        case D: {
            os << "D";
            break;
        }
        default: os << "X";
    }
    return os;
}