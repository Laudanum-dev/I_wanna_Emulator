#pragma once


namespace iwemu
{
	struct Hitbox
	{
		int x, y;
		unsigned int width, height;
		int dx, dy;
	};

	// one(or two)-way wall, of a sort. x, y - coordinates of left/top point
	// block_lt - blocks left/top
	// block_rb - blocks right/bottom
	struct Segment
	{
		int x, y;
		unsigned int length;
		bool vertical;
		bool block_lt, block_rb;
		int dx, dy;
	};

	struct BBox
	{
		double x, y;
		unsigned int width, height;
		double dx, dy;
	};

	inline int left(const Hitbox& hbox);
	inline int top(const Hitbox& hbox);
	inline int right(const Hitbox& hbox);
	inline int bottom(const Hitbox& hbox);

	inline double left(const BBox& bbox);
	inline double top(const BBox& bbox);
	inline double right(const BBox& bbox);
	inline double bottom(const BBox& bbox);

	inline bool intersect(int s1, unsigned int l1, int s2, unsigned int l2);

	bool intersect(const Hitbox& hbox, const Segment& seg);

	bool intersect(const Hitbox& hbox1, const Hitbox& hbox2);

	inline void to_hitbox(const BBox& bbox, Hitbox& hbox_dest);
	Hitbox get_hitbox(const BBox& bbox);

	inline void to_bbox(const Hitbox& hbox, BBox& bbox_dest);
	BBox get_bbox(const Hitbox& hbox);

	BBox rel(const BBox& bbox, double dx, double dy);
	Hitbox rel(const Hitbox& hbox, int dx, int dy);
	Segment rel(const Segment& seg, int dx, int dy);

	// how much bbox needs to be moved horizontaly until it hits the hbox
	double project_left(const BBox& bbox, const Hitbox& hbox);
	double project_left(const BBox& bbox, const Segment& seg);
	double project_up(const BBox& bbox, const Hitbox& hbox);
	double project_up(const BBox& bbox, const Segment& seg);
	double project_right(const BBox& bbox, const Hitbox& hbox);
	double project_right(const BBox& bbox, const Segment& seg);
	double project_down(const BBox& bbox, const Hitbox& hbox);
	double project_down(const BBox& bbox, const Segment& seg);
}
