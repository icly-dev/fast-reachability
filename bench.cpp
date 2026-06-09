#include "kick_srs.hpp"
#include "search.hpp"
#include "utils.hpp"
#include "board.hpp"
#include <cstdio>
#include <iostream>
#include <cassert>
#include <chrono>
#include <cstring>
#include <array>
#include <utility>
using namespace std;
using reachability::operator""_szc;

using BOARD = reachability::board_t<10, 48>;

uint64_t perft(BOARD b, const char* block, unsigned depth, unsigned height = 0, reachability::search::search_config cfg = {}) {
    return reachability::call_with_block<reachability::rules::SRS>(reachability::rules::Tetromino::from_name(*block), [&]<reachability::block B> [[gnu::always_inline]] () {
        uint64_t n = 0;
        constexpr int downmost = reachability::search::downmost_position<B>;
        b.call_with_height<reachability::tuple{6, 12, 24, 48}>(height + 3, [&] [[gnu::always_inline]] (auto nb) {
            constexpr reachability::coord spawn_pos = reachability::coord{4, 20};
            constexpr int necessary_height = spawn_pos[1_szc] + downmost;
            std::array<decltype(nb), B.shapes> reachable;
            if constexpr (nb.height < necessary_height) {
                reachable = reachability::search::binary_bfs<B, false>(nb, cfg, spawn_pos, 0);
            } else {
                bool check_consecutive = height > necessary_height;
                if (check_consecutive) [[unlikely]] {
                    reachable = reachability::search::binary_bfs<B, true>(nb, cfg, spawn_pos, 0);
                } else {
                    reachable = reachability::search::binary_bfs<B, false>(nb, cfg, spawn_pos, 0);
                }
            }
            if (depth == 1) {
                for (std::size_t rot = 0; rot < reachable.size(); ++rot)
                    n += reachable[rot].popcount();
                return;
            }
            reachability::static_for<B.shapes>([&] [[gnu::always_inline]] (auto rot) {
                constexpr auto mino = B.minos[rot];
                constexpr auto range = reachability::mino_range<mino>();
                constexpr auto max_y = range[3];
                reachable[rot].for_each_bit([&] [[gnu::always_inline]] (int x, int y) {
                    BOARD new_board = b | BOARD::put<mino>(x, y);
                    auto result = new_board.clear_full_lines();
                    unsigned new_height = std::max(height, unsigned(y + max_y + 1)) - result.count;
                    n += perft(result.board, block + 1, depth - 1, new_height, cfg);
                });
            });
        });
        return n;
    });
}

pair<uint64_t, uint64_t> perft_with_time(BOARD b, const char* block, unsigned depth) {
    constexpr auto cfg = reachability::search::search_config{false, true, true};
    const auto start = std::chrono::high_resolution_clock::now();
    uint64_t nodes = perft(b, block, depth, 0, cfg);
    const auto end = std::chrono::high_resolution_clock::now();
    const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return {nodes, dt};
}

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

// void configs() {
// 	auto set_row = [](BOARD& board, int y, unsigned int bits) {
// 		for (int x = 0; x < 10; ++x)
// 			if (bits & (1u << x))
// 				board.set(x, y);
// 	};

// 	BOARD b;
// 	set_row(b, 0, 0b1011000101);
// 	set_row(b, 1, 0b1001100100);
// 	set_row(b, 2, 0b1001000100);
// 	set_row(b, 3, 0b0011001001);
// 	set_row(b, 4, 0b0011101100);
// 	set_row(b, 5, 0b0001000100);
// 	set_row(b, 6, 0b1000000110);
// 	set_row(b, 7, 0b1101000000);
// 	set_row(b, 8, 0b0);
// 	set_row(b, 9, 0b1001001);
// 	set_row(b, 10, 0b1000);
// 	set_row(b, 11, 0b1001);
// 	set_row(b, 12, 0b1);
// 	std::cout << "Board:\n"
// 		  << to_string<16>(b);

// 	constexpr reachability::coord spawn{4, 20};

// 	auto show_placements = [&]<reachability::search::search_config cfg, reachability::block B>(const char* piece_name, const char* cfg_label, BOARD board) {
// 		std::array<BOARD, B.orientations> cache;
// 		auto result = reachability::search::binary_bfs<B, spawn, 0, false, cfg>(board, &cache);
// 		reachability::search::move_checker<B, BOARD> state{cache, board};
// 		std::cout << "\n--- " << cfg_label << ", " << piece_name << " ---\n";
// 		int count = 0;
// 		reachability::static_for<B.shapes>([&](auto rot) {
// 			constexpr auto mino = B.minos[rot];
// 			constexpr int N = B.orientations;
// 			result[rot].for_each_bit([&](int x, int y) {
// 				bool L = state.can_move_left(rot, x, y);
// 				bool R = state.can_move_right(rot, x, y);
// 				bool U = state.can_move_up(rot, x, y);
// 				bool D = state.can_move_down(rot, x, y);

// 				auto try_show = [&](int to_rot, const char* label) {
// 					auto [nr, nx, ny] = state.try_rotate(rot, to_rot, x, y);
// 					std::cout << "    " << label << ": ";
// 					if (nr != rot)
// 						std::cout << rot << "\u2192" << nr << " (" << nx << "," << ny << ")\n";
// 					else
// 						std::cout << rot << "\u2192" << to_rot << " (no)\n";
// 				};
// 				try_show((rot + 1) % N, "R");
// 				try_show((rot + N - 1) % N, "L");
// 				try_show((rot + 2) % N, "180");

// 				BOARD placed = board | BOARD::put<mino>(x, y);
// 				auto [cleared, cl] = placed.clear_full_lines();
// 				std::cout << "placement " << ++count
// 					  << " (rot=" << rot << " x=" << x << " y=" << y
// 					  << " L=" << L << " R=" << R << " U=" << U << " D=" << D
// 					  << " cleared=" << cl << "):\n"
// 					  << to_string<16>(cleared);
// 			});
// 		});
// 	};

// 	auto run_config = [&]<reachability::search::search_config cfg>(const char* label) {
// 		show_placements.template operator()<cfg, std::get<4>(reachability::rules::SRS::block_list)>("L", label, b);
// 	};

// 	run_config.template operator()<reachability::search::search_config{true, true, true}>("softdrop");
// 	run_config.template operator()<reachability::search::search_config{true, false, true}>("sonicdrop");
// 	run_config.template operator()<reachability::search::search_config{true, false, false}>("harddrop");
// }

// void perft_configs(const char* pieces) {
// 	const auto len = std::strlen(pieces);
// 	auto run = [&]<reachability::search::search_config cfg>(const char* label) {
// 		const auto start = std::chrono::high_resolution_clock::now();
// 		uint64_t nodes = perft<cfg>(BOARD{}, pieces, len);
// 		const auto end = std::chrono::high_resolution_clock::now();
// 		auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
// 		std::cout << label << ": Nodes=" << nodes << " Time=" << dt << "ms" << std::endl;
// 	};
// 	run.template operator()<reachability::search::search_config{true, true, true}>("softdrop");
// 	run.template operator()<reachability::search::search_config{true, false, true}>("sonicdrop");
// 	run.template operator()<reachability::search::search_config{true, false, false}>("harddrop");
// }

int main(int argc, char* argv[]) {
    assert(argc >= 2);
    if (strcmp(argv[1], "test") == 0) {
        test();
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else if (strcmp(argv[1], "bench") == 0) {
        bench();
        return 0;
    } //  else if (strcmp(argv[1], "configs") == 0) {
    // 	assert(argc >= 2);
    // 	if (argc == 2) {
    // 		configs();
    // 	} else {
    // 		perft_configs(argv[2]);
    // 	}
    // 	return 0;
    // }

    const auto [nodes, dt] = perft_with_time(BOARD{}, argv[1], strlen(argv[1]));

    std::cout << "Depth: " << strlen(argv[1])
              << " Nodes: " << nodes
              << " Time: " << dt << "ms"
              << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
}