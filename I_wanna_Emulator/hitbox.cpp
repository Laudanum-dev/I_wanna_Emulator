#include "hitbox.h"

#include <math.h>

namespace iwemu
{
	inline int left(const Hitbox& hbox) { return hbox.x; }
	inline int top(const Hitbox& hbox) { return hbox.y; }
	inline int right(const Hitbox& hbox) { return hbox.x + hbox.width; }
	inline int bottom(const Hitbox& hbox) { return hbox.y + hbox.height; }

	inline double left(const BBox& bbox) { return bbox.x; }
	inline double top(const BBox& bbox) { return bbox.y; }
	inline double right(const BBox& bbox) { return bbox.x + bbox.width; }
	inline double bottom(const BBox& bbox) { return bbox.y + bbox.height; }

	inline bool intersect(int s1, unsigned int l1, int s2, unsigned int l2)
	{
		return (s2 < s1 + (int)l1) && (s1 < s2 + (int)l2);
	}

	bool intersect(const Hitbox& hbox, const Segment& seg)
	{
		if (seg.vertical)
		{
			return
				intersect(hbox.x, hbox.width, seg.x, 0) &&
				intersect(hbox.y, hbox.height, seg.y, seg.length);
		}
		else
		{
			return 
				intersect(hbox.y, hbox.height, seg.y, 0) &&
				intersect(hbox.x, hbox.width, seg.x, seg.length);
		}
	}

	bool intersect(const Hitbox& hbox1, const Hitbox& hbox2)
	{
		return
			intersect(hbox1.x, hbox1.width, hbox2.x, hbox2.width) &&
			intersect(hbox1.y, hbox1.height, hbox2.y, hbox2.height);
	}

	inline void to_hitbox(const BBox& bbox, Hitbox& hbox_dest)
	{
		hbox_dest = { lround(bbox.x), lround(bbox.y), bbox.width, bbox.height, lround(bbox.dx), lround(bbox.dy) };
	}

	Hitbox get_hitbox(const BBox& bbox)
	{
		return { lround(bbox.x), lround(bbox.y), bbox.width, bbox.height, lround(bbox.dx), lround(bbox.dy) };
	}

	inline void to_bbox(const Hitbox& hbox, BBox& bbox_dest)
	{
		bbox_dest = { (double)hbox.x, (double)hbox.y, hbox.width, hbox.height, (double)hbox.dx, (double)hbox.dy };
	}

	BBox get_bbox(const Hitbox& hbox)
	{
		return { (double)hbox.x, (double)hbox.y, hbox.width, hbox.height, (double)hbox.dx, (double)hbox.dy };
	}

	BBox rel(const BBox& bbox, double dx, double dy)
	{
		return { bbox.x + dx, bbox.y + dy, bbox.width, bbox.height, bbox.dx, bbox.dy };
	}

	Hitbox rel(const Hitbox& hbox, int dx, int dy)
	{
		return { hbox.x + dx, hbox.y + dy, hbox.width, hbox.height, hbox.dx, hbox.dy };
	}

	Segment rel(const Segment& seg, int dx, int dy)
	{
		return { seg.x + dx, seg.y + dy, seg.length, seg.vertical, seg.block_lt, seg.block_rb, seg.dx, seg.dy };
	}

	double project_left(const BBox& bbox, const Hitbox& hbox)
	{
		if (intersect(lround(bbox.y), bbox.height, hbox.y, hbox.height))
		{	// projections on Y axis intersect
			if (intersect(lround(bbox.x), bbox.width, hbox.x, hbox.width))
				return 0.0;	// boxes intersect

			if (hbox.x < bbox.x)	// if to the left
				return left(bbox) - right(hbox);
			else
				return INFINITY;
		}
		return INFINITY;
	}

	double project_left(const BBox& bbox, const Segment& seg)
	{
		if (seg.vertical &&			// vertical
			seg.block_rb &&		// and blocks from right
			seg.x <= lround(left(bbox)) &&	// and to the left
			intersect(lround(bbox.y), bbox.height, seg.y, seg.length))
		{	// and projections on Y axis intersect
			return left(bbox) - seg.x;
		}
		return INFINITY;
	}

	double project_up(const BBox& bbox, const Hitbox& hbox)
	{
		if (intersect(lround(bbox.x), bbox.width, hbox.x, hbox.width))
		{	// projections on X axis intersect
			if (intersect(lround(bbox.y), bbox.height, hbox.y, hbox.height))
				return 0.0; // boxes intersect

			if (hbox.y < bbox.y)	// if to the up
				return top(bbox) - bottom(hbox);
			else
				return INFINITY;
		}
		return INFINITY;
	}

	double project_up(const BBox& bbox, const Segment& seg)
	{
		if (!seg.vertical &&		// horizontal
			seg.block_rb &&		// and blocks from bottom
			seg.y <= lround(top(bbox)) &&	// and to the up
			intersect(lround(bbox.x), bbox.width, seg.x, seg.length))
		{	// and projections on X axis intersect
			return top(bbox) - seg.y;
		}
		return INFINITY;
	}

	double project_right(const BBox& bbox, const Hitbox& hbox)
	{
		if (intersect(lround(bbox.y), bbox.height, hbox.y, hbox.height))
		{	// projections on Y axis intersect
			if (intersect(lround(bbox.x), bbox.width, hbox.x, hbox.width))
				return 0.0;	// boxes intersect

			if (hbox.x > bbox.x)	// if to the right
				return left(hbox) - right(bbox);
			else
				return INFINITY;
		}
		return INFINITY;
	}

	double project_right(const BBox& bbox, const Segment& seg)
	{
		if (seg.vertical &&			// vertical
			seg.block_lt &&		// and blocks from left
			seg.x >= lround(right(bbox)) &&	// and to the right
			intersect(lround(bbox.y), bbox.height, seg.y, seg.length))
		{	// and projections on Y axis intersect
			return seg.x - right(bbox);
		}
		return INFINITY;
	}

	double project_down(const BBox& bbox, const Hitbox& hbox)
	{
		if (intersect(lround(bbox.x), bbox.width, hbox.x, hbox.width))
		{	// projections on X axis intersect
			if (intersect(lround(bbox.y), bbox.height, hbox.y, hbox.height))
				return 0.0; // boxes intersect

			if (hbox.y > bbox.y)	// if to the down
				return top(hbox) - bottom(bbox);
			else
				return INFINITY;
		}
		return INFINITY;
	}

	double project_down(const BBox& bbox, const Segment& seg)
	{
		if (!seg.vertical &&			// horizontal
			seg.block_lt &&				// and blocks from top
			seg.y >= lround(bottom(bbox)) &&	// and to the down
			intersect(lround(bbox.x), bbox.width, seg.x, seg.length))
		{	// and projections on X axis intersect
			return seg.y - bottom(bbox);
		}
		return INFINITY;
	}
}