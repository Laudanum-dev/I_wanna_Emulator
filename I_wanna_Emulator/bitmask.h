#pragma once

// vector<bool> suits most for this
#include <vector>
#include "hitbox.h"

namespace iwemu
{
	struct Bitmask : Hitbox
	{
		std::vector<std::vector<bool>> mask;
		double rotation;
	};

	bool intersect(Bitmask& mask, Hitbox& hbox);
}