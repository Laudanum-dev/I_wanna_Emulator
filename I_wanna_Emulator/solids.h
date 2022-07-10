#pragma once

#include "hitbox.h"

namespace iwemu
{
	class SolidScene 
	{
	public:
		// the rule is that if collidable ends up inside a solid, it dies 
		// (solidScene will no longer process this object)
		bool* alive = 0;
		int grav_dir = 1;

		SolidScene(
			int grav_dir,
			Hitbox* solids, size_t solidsC,
			Segment* segments, size_t segmentsC,
			BBox* collidables, size_t collidablesC
		);
		~SolidScene();

		// tells if a specified place has any solid in it
		bool place_solid(const Hitbox& hbox);
		// tells if a wall or a solid intersect a box
		bool place_free(const Hitbox& hbox);

		// tells how much box can be moved on a specified direction until it meets a solid
		// optionally can tell what object is the closest
		double project_free_left(const BBox& bbox, Hitbox** hbox_p_dest=0, Segment** seg_p_dest=0);
		double project_free_up(const BBox& bbox, Hitbox** hbox_p_dest=0, Segment** seg_p_dest=0);
		double project_free_right(const BBox& bbox, Hitbox** hbox_p_dest=0, Segment** seg_p_dest=0);
		double project_free_down(const BBox& bbox, Hitbox** hbox_p_dest=0, Segment** seg_p_dest=0);

		enum class CollisionSide {
			NONE, LEFT, TOP, RIGHT, BOTTOM
		};
		CollisionSide collision_side(const BBox& bbox, const Hitbox& hbox, int dx, int dy);

		// moves every solid by desired amount, and pushes the collidables
		void update();
	private:
		Hitbox* _solids = 0;
		size_t _solidsC = 0;
		Segment* _segments = 0;
		size_t _segmentsC = 0;
		BBox* _collidable = 0;
		size_t _collidableC = 0;
		BBox* _collidableOld = 0;

		double project_free_direction(const BBox& bbox, Hitbox** hbox_p_dest, Segment** seg_p_dest,
			double (*project_function_hbox)(const BBox&, const Hitbox&),
			double (*project_function_seg)(const BBox&, const Segment&));

	};
}
