#include "perft.hpp"

#include <cstdio>
#include <iostream>
#include <cassert>
#include <cstring>
#include <array>
#include <string_view>
using namespace std;

void test() {
    constexpr std::array test_data = {
        std::pair{"IIIIII"sv, 33325433u},
        std::pair{"IOLJSZT"sv, 2647076135u},
        std::pair{"TIOLJSZ"sv, 2785677550u},
        std::pair{"ZTIOLJS"sv, 2741273038u},
        std::pair{"SZTIOLJ"sv, 2740055656u},
        std::pair{"JSZTIOL"sv, 2801460686u},
        std::pair{"LJSZTIO"sv, 2852978763u},
        std::pair{"OLJSZTI"sv, 2689379684u},
    };
    for (const auto& [blocks, expected] : test_data) {
        BOARD state;
        const auto cfg = reachability::search::search_config{false, true, true};
        const uint64_t result = perft(state, blocks.data(), blocks.size(), 0, cfg);
        std::cout << "Testing blocks: " << blocks << ", expected: " << expected << ", got: " << result << std::endl;
        assert(result == expected);
    }
}

void bench() {
    constexpr std::array data = {
        "IOLJSZT"sv,
        "TIOLJSZ"sv,
        "ZTIOLJS"sv,
        "SZTIOLJ"sv,
        "JSZTIOL"sv,
        "LJSZTIO"sv,
        "OLJSZTI"sv,
    };
    uint64_t nodes_sum = 0, dt_sum = 0;
    for (const auto& blocks : data) {
        const auto [nodes, dt] = perft_with_time(BOARD{}, blocks.data(), blocks.size());
        std::cout << "Blocks: " << blocks << " Nodes: " << nodes << " Time: " << dt << "ms" << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
        nodes_sum += nodes;
        dt_sum += dt;
    }
    std::cout << "Total Nodes: " << nodes_sum << " Total Time: " << dt_sum << "ms" << " Average NPS: " << (nodes_sum * 1000) / (dt_sum + 1) << std::endl;
}

reachability::search::search_config parse_config(int argc, char* argv[]) {
    reachability::search::search_config cfg{
        .allow_180 = false,
        .allow_softdrop = false,
        .allow_sonicdrop = false,
        .allow_20g = false};
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--config=", 9) == 0) {
            const char* s = argv[i] + 9;
            while (*s) {
                switch (*s) {
                    case 'x':
                        cfg.allow_180 = true;
                        break;
                    case 'd':
                        cfg.allow_softdrop = true;
                        break;
                    case 'D':
                        cfg.allow_sonicdrop = true;
                        break;
                    case 'g':
                        cfg.allow_20g = true;
                        break;
                }
                s++;
            }
        }
    }
    return cfg;
}

const char* find_piece(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--config=", 9) != 0 && strncmp(argv[i], "--", 2) != 0 && strcmp(argv[i], "test") != 0 && strcmp(argv[i], "bench") != 0) {
            return argv[i];
        }
    }
    return "";
}

bool has_flag(int argc, char* argv[], const char* flag) {
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], flag) == 0)
            return true;
    return false;
}

void diagnose(BOARD board, const char* pieces, reachability::search::search_config cfg, const char* label) {
    const auto depth = strlen(pieces);
    const auto [nodes, dt] = perft_with_time(board, pieces, depth, cfg);
    std::cout << label << " Depth: " << depth
              << " Nodes: " << nodes
              << " Time: " << dt << "ms"
              << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;

    constexpr auto spawn = reachability::coord{4, 20};
    int move_count = 0;

    auto show = [&](BOARD b, const char* p, unsigned remaining, unsigned height, auto& self) -> void {
        if (remaining == 0)
            return;
        reachability::call_with_block<reachability::rules::SRS>(
            reachability::rules::Tetromino::from_name(*p),
            [&]<reachability::block B> -> int {
                constexpr int downmost = reachability::search::downmost_position<B>;
                b.call_with_height<reachability::tuple{6, 12, 24, 48}>(height + 3, [&](auto nb) {
                    constexpr int necessary_height = spawn[1_szc] + downmost;
                    std::array<decltype(nb), B.shapes> reachable;
                    if constexpr (nb.height < necessary_height) {
                        reachable = reachability::search::binary_bfs<B, false>(nb, cfg, spawn, 0);
                    } else {
                        bool check_consecutive = height > (unsigned)necessary_height;
                        if (check_consecutive) [[unlikely]] {
                            reachable = reachability::search::binary_bfs<B, true>(nb, cfg, spawn, 0);
                        } else {
                            reachable = reachability::search::binary_bfs<B, false>(nb, cfg, spawn, 0);
                        }
                    }
                    reachability::static_for<B.shapes>([&](auto shape_idx) {
                        constexpr auto mino = B.minos[shape_idx];
                        constexpr auto range = reachability::mino_range<mino>();
                        constexpr auto max_y = range[3];
                        reachable[shape_idx].for_each_bit([&](int x, int y) {
                            BOARD new_board = b | BOARD::put<mino>(x, y);
                            auto [cleared, cleared_count, _] = new_board.clear_full_lines();
                            unsigned new_height = std::max(height, unsigned(y + max_y + 1)) - cleared_count;
                            std::cout << "move " << ++move_count << ": "
                                      << *p << " r" << shape_idx << " (" << x << "," << y << ")"
                                      << (cleared_count ? " clear " + std::to_string(cleared_count) : "")
                                      << "\n"
                                      << to_string<16>(cleared) << std::flush;
                            std::cin.get();
                            if (remaining > 1)
                                self(cleared, p + 1, remaining - 1, new_height, self);
                        });
                    });
                });
                return 0;
            });
    };
    show(board, pieces, depth, 0, show);
}

int main(int argc, char* argv[]) {
    assert(argc >= 2);
    if (strcmp(argv[1], "test") == 0) {
        test();
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else if (strcmp(argv[1], "bench") == 0) {
        bench();
        return 0;
    }

    auto cfg = parse_config(argc, argv);
    const char* pieces = find_piece(argc, argv);

    if (has_flag(argc, argv, "--diagnose")) {
        diagnose(BOARD{}, pieces, cfg, pieces);
        std::cin.get();
        return 0;
    }

    const auto [nodes, dt] = perft_with_time(BOARD{}, pieces, strlen(pieces), cfg);

    std::cout << "Depth: " << strlen(pieces)
              << " Nodes: " << nodes
              << " Time: " << dt << "ms"
              << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
}
