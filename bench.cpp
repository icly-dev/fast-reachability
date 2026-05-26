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

template <reachability::search::search_config cfg = reachability::search::search_config{}>
uint64_t perft(BOARD b, const char* block, unsigned depth, unsigned height = 0) {
	return reachability::call_with_block<reachability::rules::SRS>(reachability::block_from_name(*block), [&]<reachability::block B> [[gnu::always_inline]] () {
		uint64_t n = 0;
		constexpr int relative_height = reachability::search::lowest_position<B>;
		b.call_with_height<reachability::tuple{6, 12, 24, 48}>(height + 3, [&] [[gnu::always_inline]] (auto nb) {
			constexpr reachability::coord spawn_pos = reachability::coord{4, 20};
			constexpr int necessary_height = spawn_pos[1_szc] + relative_height;
			std::array<decltype(nb), B.shapes> reachable;
			if constexpr (nb.height < necessary_height) {
				reachable = reachability::search::binary_bfs<B, spawn_pos, 0, false, cfg>(nb);
			} else {
				bool check_consecutive = height > necessary_height;
				if (check_consecutive) [[unlikely]] {
					reachable = reachability::search::binary_bfs<B, spawn_pos, 0, true, cfg>(nb);
				} else {
					reachable = reachability::search::binary_bfs<B, spawn_pos, 0, false, cfg>(nb);
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
					auto [cleared, cleared_lines] = new_board.clear_full_lines();
					unsigned new_height = std::max(height, unsigned(y + max_y + 1)) - cleared_lines;
					n += perft<cfg>(cleared, block + 1, depth - 1, new_height);
				});
			});
		});
		return n;
	});
}

pair<uint64_t, uint64_t> perft_with_time(BOARD b, const char* block, unsigned depth) {
	const auto start = std::chrono::high_resolution_clock::now();
	uint64_t nodes = perft(b, block, depth);
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
		const uint64_t result = perft(state, blocks.data(), blocks.size());
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

void configs() {
	auto set_col = [](BOARD& board, int x, unsigned long long bits) {
		for (int y = 0; y < 12; ++y)
			if (bits & (1ull << y))
				board.set(x, y);
	};

	BOARD b;
	set_col(b, 0, 0b111111111100);
	set_col(b, 1, 0b110000001100);
	set_col(b, 2, 0b110000001100);
	set_col(b, 3, 0b110011001100);
	set_col(b, 4, 0b110011001100);
	set_col(b, 5, 0b110011001100);
	set_col(b, 6, 0b110011001100);
	set_col(b, 7, 0b110011001100);
	set_col(b, 8, 0b000011000000);
	set_col(b, 9, 0b000011111111);
	std::cout << "Board:\n" << to_string<12>(b);

	constexpr reachability::coord spawn{4, 20};

	auto show_placements = [&]<reachability::search::search_config cfg, reachability::block B>(const char* piece_name, const char* cfg_label, BOARD board) {
		auto result = reachability::search::binary_bfs<B, spawn, 0, false, cfg>(board);
		std::cout << "\n--- " << cfg_label << ", " << piece_name << " ---\n";
		int count = 0;
		reachability::static_for<B.shapes>([&](auto rot) {
			constexpr auto mino = B.minos[rot];
			result[rot].for_each_bit([&](int x, int y) {
				BOARD placed = board | BOARD::put<mino>(x, y);
				auto [cleared, cl] = placed.clear_full_lines();
				std::cout << "placement " << ++count
					  << " (rot=" << rot << " x=" << x << " y=" << y
					  << " cleared=" << cl << "):\n" << to_string<16>(cleared);
			});
		});
	};

	auto run_config = [&]<reachability::search::search_config cfg>(const char* label) {
		show_placements.template operator()<cfg, std::get<5>(reachability::rules::SRS::block_list)>("O", label, b);
		show_placements.template operator()<cfg, std::get<0>(reachability::rules::SRS::block_list)>("T", label, b);
	};

	run_config.template operator()<reachability::search::search_config{}>("softdrop");
	run_config.template operator()<reachability::search::search_config{true, false, true}>("sonicdrop");
	run_config.template operator()<reachability::search::search_config{true, false, false}>("harddrop");
}

void perft_configs(const char* pieces) {
	const auto len = std::strlen(pieces);
	auto run = [&]<reachability::search::search_config cfg>(const char* label) {
		const auto start = std::chrono::high_resolution_clock::now();
		uint64_t nodes = perft<cfg>(BOARD{}, pieces, len);
		const auto end = std::chrono::high_resolution_clock::now();
		auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << label << ": Nodes=" << nodes << " Time=" << dt << "ms" << std::endl;
	};
	run.template operator()<reachability::search::search_config{}>("softdrop");
	run.template operator()<reachability::search::search_config{true, false, true}>("sonicdrop");
	run.template operator()<reachability::search::search_config{true, false, false}>("harddrop");
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
	} else if (strcmp(argv[1], "configs") == 0) {
		assert(argc >= 2);
		if (argc == 2) {
			configs();
		} else {
			perft_configs(argv[2]);
		}
		return 0;
	}

	const auto [nodes, dt] = perft_with_time(BOARD{}, argv[1], strlen(argv[1]));

	std::cout << "Depth: " << strlen(argv[1])
		  << " Nodes: " << nodes
		  << " Time: " << dt << "ms"
		  << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
}
