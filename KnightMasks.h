#include "Bitboard.h"


#pragma once


namespace KnightMasks {
    static consteval uint8_t absSubtract(uint8_t left, uint8_t right) {
        if (left >= right) {
            return left - right;
        }
        return right - left;
    }
    static consteval std::array<Bitboard, 64> calcMasks() {
        std::array<Bitboard, 64> masks{};

        uint8_t dx;
        uint8_t dy;

        for (uint8_t x0 = 0; x0 < 8; x0 = x0 + 1) {
            for (uint8_t y0 = 0; y0 < 8; y0 = y0 + 1) {

                for (uint8_t x1 = 0; x1 < 8; x1 = x1 + 1) {
                    for (uint8_t y1 = 0; y1 < 8; y1 = y1 + 1) {

                        dx = absSubtract(x0, x1);
                        dy = absSubtract(y0, y1);

                        if ((dx == 2 and dy == 1) or (dx == 1 and dy == 2)) {
                            masks[y0 * 8 + x0] = BOp::set1(masks[y0 * 8 + x0], y1 * 8 + x1);
                        }
                    }
                }
            }
        }

        return masks;
    }
    static constexpr std::array<Bitboard, 64> MASKS = calcMasks();
}