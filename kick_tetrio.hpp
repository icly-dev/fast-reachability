#pragma once
#include "piece_tetromino.hpp"

namespace reachability::rules {
	struct TETRIO_SRS_Plus_Kicks {
		static constexpr auto ZS_kick_data = tuple{
		    tuple{tuple{0, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},
		    tuple{tuple{0, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},
		    tuple{tuple{0, 2}, tuple{coord{0, 0}, coord{0, 1}}},
		    tuple{tuple{1, 2}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},
		    tuple{tuple{1, 0}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},
		    tuple{tuple{1, 3}, tuple{coord{0, 0}, coord{1, 0}}},
		    tuple{tuple{2, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},
		    tuple{tuple{2, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},
		    tuple{tuple{2, 0}, tuple{coord{0, 0}, coord{0, -1}}},
		    tuple{tuple{3, 0}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},
		    tuple{tuple{3, 2}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},
		    tuple{tuple{3, 1}, tuple{coord{0, 0}, coord{-1, 0}}},
		};

		static constexpr auto TJL_kick_data = tuple{
		    tuple{tuple{0, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},
		    tuple{tuple{0, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},
		    tuple{tuple{0, 2}, tuple{coord{0, 0}, coord{0, 1}, coord{1, 1}, coord{-1, 1}, coord{1, 0}, coord{-1, 0}}},
		    tuple{tuple{1, 2}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},
		    tuple{tuple{1, 0}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},
		    tuple{tuple{1, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 2}, coord{1, 1}, coord{0, 2}, coord{0, 1}}},
		    tuple{tuple{2, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},
		    tuple{tuple{2, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},
		    tuple{tuple{2, 0}, tuple{coord{0, 0}, coord{0, -1}, coord{-1, -1}, coord{1, -1}, coord{-1, 0}, coord{1, 0}}},
		    tuple{tuple{3, 0}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},
		    tuple{tuple{3, 2}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},
		    tuple{tuple{3, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 2}, coord{-1, 1}, coord{0, 2}, coord{0, 1}}},
		};

		static constexpr auto I_kick_data = tuple{
		    tuple{tuple{0, 1}, tuple{coord{0, 0}, coord{1, 0}, coord{-2, 0}, coord{-2, -1}, coord{1, 2}}},
		    tuple{tuple{0, 3}, tuple{coord{0, 0}, coord{-1, 0}, coord{2, 0}, coord{2, -1}, coord{-1, 2}}},
		    tuple{tuple{0, 2}, tuple{coord{0, 0}, coord{0, 1}}},
		    tuple{tuple{1, 2}, tuple{coord{0, 0}, coord{-1, 0}, coord{2, 0}, coord{-1, 2}, coord{2, -1}}},
		    tuple{tuple{1, 0}, tuple{coord{0, 0}, coord{-1, 0}, coord{2, 0}, coord{-1, -2}, coord{2, 1}}},
		    tuple{tuple{1, 3}, tuple{coord{0, 0}, coord{1, 0}}},
		    tuple{tuple{2, 3}, tuple{coord{0, 0}, coord{2, 0}, coord{-1, 0}, coord{2, 1}, coord{-1, -2}}},
		    tuple{tuple{2, 1}, tuple{coord{0, 0}, coord{-2, 0}, coord{1, 0}, coord{-2, 1}, coord{1, -2}}},
		    tuple{tuple{2, 0}, tuple{coord{0, 0}, coord{0, -1}}},
		    tuple{tuple{3, 0}, tuple{coord{0, 0}, coord{1, 0}, coord{-2, 0}, coord{1, -2}, coord{-2, 1}}},
		    tuple{tuple{3, 2}, tuple{coord{0, 0}, coord{1, 0}, coord{-2, 0}, coord{1, 2}, coord{-2, -1}}},
		    tuple{tuple{3, 1}, tuple{coord{0, 0}, coord{-1, 0}}},
		};

		static constexpr auto all = tuple{
		    kick_table{TJL_kick_data}, // T
		    kick_table{ZS_kick_data},  // Z
		    kick_table{ZS_kick_data},  // S
		    kick_table{TJL_kick_data}, // J
		    kick_table{TJL_kick_data}, // L
		    no_kicks,		       // O
		    kick_table{I_kick_data},   // I
		};
	};

	using TETRIO_SRS_Plus = rule_set<Tetromino, TETRIO_SRS_Plus_Kicks>;
} // namespace reachability::rules
