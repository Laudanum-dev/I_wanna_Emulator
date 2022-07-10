#include "solids.h"

#include <math.h>

namespace iwemu
{
	inline bool intersectF(double s1, unsigned int l1, double s2, unsigned int l2)
	{
		return (s2 < s1 + (double)l1) && (s1 < s2 + (double)l2);
	}

	bool dyn_seg_h(
		double x1, double y1, unsigned int l1,
		int x2, int y2, unsigned int l2,
		int dx2, int dy2
	) {
		if (dy2)
		{
			double k = (y1 - y2) / dy2;
			if (k >= 0 && k <= 1 && intersectF(x1, l1, x2 + k * dx2, l2))
			{
				return true;
			}
		}
		return false;
	}

	bool dyn_seg_v(
		double x1, double y1, unsigned int l1,
		int x2, int y2, unsigned int l2,
		int dx2, int dy2
	) {
		if (dx2)
		{
			double k = (x1 - x2) / dx2;
			if (k >= 0 && k <= 1 && intersectF(y1, l1, y2 + k * dy2, l2))
			{
				return true;
			}
		}
		return false;
	}

	SolidScene::SolidScene(
		int grav_dir,
		Hitbox* solids, size_t solidsC,
		Segment* segments, size_t segmentsC,
		BBox* collidables, size_t collidablesC
	) : grav_dir(grav_dir), _solids(solids), _solidsC(solidsC),
		_segments(segments), _segmentsC(segmentsC),
		_collidable(collidables), _collidableC(collidablesC)
	{
		this->alive = new bool[collidablesC];
		for (size_t i = 0; i < collidablesC; i++)
			this->alive[i] = true;
		this->_collidableOld = new BBox[collidablesC];
	}

	SolidScene::~SolidScene()
	{
		delete this->alive;
		delete this->_collidableOld;
	}

	bool SolidScene::place_solid(const Hitbox& hbox)
	{
		for (size_t i = 0; i < this->_solidsC; i++)
		{
			if (intersect(hbox, this->_solids[i]))
				return true;
		}
		return false;
	}

	bool SolidScene::place_free(const Hitbox& hbox)
	{
		if (place_solid(hbox)) return false;
		for (size_t i = 0; i < this->_segmentsC; i++)
		{
			if (intersect(hbox, this->_segments[i]))
				return false;
		}
		return true;
	}


	double SolidScene::project_free_direction(const BBox& bbox, Hitbox** hbox_p_dest, Segment** seg_p_dest, 
		double (*project_function_hbox)(const BBox&, const Hitbox&), 
		double (*project_function_seg)(const BBox&, const Segment&))
	{
		double dist = INFINITY, cdist;
		Hitbox* closest_hitbox = 0;
		Segment* closest_segment = 0;
		for (size_t i = 0; i < this->_solidsC; i++)
		{
			cdist = project_function_hbox(bbox, this->_solids[i]);
			if (cdist < dist)
			{
				dist = cdist;
				closest_hitbox = this->_solids + i;
			}
		}
		for (size_t i = 0; i < this->_segmentsC; i++)
		{
			cdist = project_function_seg(bbox, this->_segments[i]);
			if (cdist < dist)
			{
				dist = cdist;
				closest_hitbox = 0;
				closest_segment = this->_segments + i;
			}
		}
		if (hbox_p_dest) *hbox_p_dest = closest_hitbox;
		if (seg_p_dest) *seg_p_dest = closest_segment;
		return dist;
	}

	double SolidScene::project_free_left(const BBox& bbox, Hitbox** hbox_p_dest, Segment** seg_p_dest)
	{
		return this->project_free_direction(bbox, hbox_p_dest, seg_p_dest, project_left, project_left);
	}

	double SolidScene::project_free_up(const BBox& bbox, Hitbox** hbox_p_dest, Segment** seg_p_dest)
	{
		return this->project_free_direction(bbox, hbox_p_dest, seg_p_dest, project_up, project_up);
	}

	double SolidScene::project_free_right(const BBox& bbox, Hitbox** hbox_p_dest, Segment** seg_p_dest)
	{
		return this->project_free_direction(bbox, hbox_p_dest, seg_p_dest, project_right, project_right);
	}

	double SolidScene::project_free_down(const BBox& bbox, Hitbox** hbox_p_dest, Segment** seg_p_dest)
	{
		return this->project_free_direction(bbox, hbox_p_dest, seg_p_dest, project_down, project_down);
	}

	SolidScene::CollisionSide SolidScene::collision_side(const BBox& bbox, const Hitbox& hbox, int dx, int dy)
	{	// black magic down here.
		if (dx < 0)
		{	// hbox moves left
			if (dy < 0)
			{	// hbox moves top-left
				bool h = dyn_seg_v(
					right(bbox), top(bbox), bbox.height,
					left(hbox), top(hbox), hbox.height,
					dx, dy
				);
				bool v = dyn_seg_h(
					left(bbox), bottom(bbox), bbox.width,
					left(hbox), top(hbox), hbox.width,
					dx, dy
				);
				if (h)
					return CollisionSide::RIGHT;
				if (v)
					return CollisionSide::BOTTOM;
				return CollisionSide::NONE;
			}
			else if (dy == 0)
			{	// hbox moves left
				if (project_right(bbox, hbox) <= abs(dx))
				{
					return CollisionSide::RIGHT;
				}
				else
				{
					return CollisionSide::NONE;
				}
			}
			else
			{	// hbox moves bottom-left
				bool h = dyn_seg_v(
					right(bbox), top(bbox), bbox.height,
					left(hbox), top(hbox), hbox.height,
					dx, dy
				);
				bool v = dyn_seg_h(
					left(bbox), top(bbox), bbox.width,
					left(hbox), bottom(hbox), hbox.width,
					dx, dy
				);
				if (h)
					return CollisionSide::RIGHT;
				if (v)
					return CollisionSide::TOP;
				return CollisionSide::NONE;
			}
		}
		else if (dx == 0)
		{	// hbox moves only vertically.
			if (dy < 0)
			{	// hbox moves up
				if (project_down(bbox, hbox) <= abs(dy))
				{
					return CollisionSide::BOTTOM;
				}
				else
				{
					return CollisionSide::NONE;
				}
			}
			else if (dy == 0)
			{	// hbox doesn't move ...
				return CollisionSide::NONE;
			}
			else
			{	// hbox moves down
				if (project_up(bbox, hbox) <= abs(dy))
				{
					return CollisionSide::TOP;
				}
				else
				{
					return CollisionSide::NONE;
				}
			}
		}
		else
		{	// hbox moves right
			if (dy < 0)
			{	// hbox moves top-right
				bool h = dyn_seg_v(
					left(bbox), top(bbox), bbox.height,
					right(hbox), top(hbox), hbox.height,
					dx, dy
				);
				bool v = dyn_seg_h(
					left(bbox), bottom(bbox), bbox.width,
					left(hbox), top(hbox), hbox.width,
					dx, dy
				);
				if (h)
					return CollisionSide::LEFT;
				if (v)
					return CollisionSide::BOTTOM;
				return CollisionSide::NONE;
			}
			else if (dy == 0)
			{	// hbox moves right
				if (project_left(bbox, hbox) <= abs(dx))
				{
					return CollisionSide::LEFT;
				}
				else
				{
					return CollisionSide::NONE;
				}
			}
			else
			{	// hbox bottom-right
				bool h = dyn_seg_v(
					left(bbox), top(bbox), bbox.height,
					right(hbox), top(hbox), hbox.height,
					dx, dy
				);
				bool v = dyn_seg_h(
					left(bbox), top(bbox), bbox.width,
					left(hbox), bottom(hbox), hbox.width,
					dx, dy
				);
				if (h)
					return CollisionSide::LEFT;
				if (v)
					return CollisionSide::TOP;
				return CollisionSide::NONE;
			}
		}
	}

	void SolidScene::update()
	{
		// update dynamic solids. for each solid 
		// if it moves into the player, push the player
		// else if player was standing on it, try to carry the player 
		// (move it until it hits a solid)
		// then update the player

		// map the solids based on how they affect the player
		bool* done_solids = new bool[this->_solidsC] {false};
		bool* done_segments = new bool[this->_segmentsC] {false};
		bool* standing = new bool[this->_collidableC] {false};
		// do all horizontal and downwards carrying first
		for (size_t i = 0; i < this->_solidsC; i++)
		{
			Hitbox& cs = this->_solids[i];
			for (size_t k = 0; k < this->_collidableC; k++)
			{
				if (!this->alive[k]) continue;
				BBox& cc = this->_collidable[k];
				if (intersect(get_hitbox(rel(cc, 0.0, this->grav_dir)), cs) && 
					!intersect(get_hitbox(cc), cs))
				{	// if standing on a solid, it can carry us.
					// horizontal and downwards are carries that can be done 
					// by "softly" pushing collidable. upwards carrying 
					// might slam us into ceiling, so it will be done in pushing part
					standing[k] = true;
					if (cs.dx != 0)
					{	// it moves horizontally
						// try to move horizontally as well
						int carryX = cs.dx;
						done_solids[i] = true;
						if (place_free(get_hitbox(rel(cc, carryX, 0.0))))
						{	// nothing stands in our way
							cc.x += carryX;
						}
						else
						{	// something is standing in our way
							double dist = 0.0;
							if (carryX > 0)
							{	// wanna go right
								dist = project_free_right(cc);
								if (dist < carryX)
									dist = round(dist);
								else
									dist = carryX;
							}
							else
							{	// wanna go left
								dist = project_free_left(cc);
								if (dist < -carryX)
									dist = -round(dist);
								else
									dist = carryX;
							}
							cc.x += dist;
						}
					}
					if (cs.dy * this->grav_dir > 0)
					{	// it (also) moves down
						int carryY = cs.dy;
						// move the solid down, so it doesn't register as collision
						cs.y += cs.dy;
						done_solids[i] = true;
						if (place_free(get_hitbox(rel(cc, 0.0, carryY))))
						{	// nothing stands in our way
							cc.y += carryY;
						}
						else
						{	// something is in our way
							double dist = 0.0;
							if (grav_dir > 0)
							{
								dist = project_free_down(cc);
								if (dist < carryY)
									dist = round(dist);
								else
									dist = carryY;
							}
							else
							{
								dist = project_free_up(cc);
								if (dist < -carryY)
									dist = -round(dist);
								else
									dist = carryY;
							}
							cc.y += dist;
							
						}
						// have to move back up to move it properly later
						cs.y -= cs.dy;
					}
				}
			}
			// after all collidables were carried, move the solid that did the carry, 
			// since bc of downwards carrying collidable will be inside a solid, 
			// and we want to ignore that
			if (done_solids[i])
			{
				cs.x += cs.dx;
				cs.y += cs.dy;
			}
		}
		
		// do the same with segments
		for (size_t i = 0; i < this->_segmentsC; i++)
		{
			Segment& cs = this->_segments[i];
			if (cs.vertical || !cs.block_lt) continue;	// vertical segments can't carry.
														// as well as those who not block top.
			for (size_t k = 0; k < this->_collidableC; k++)
			{
				if (!this->alive[k]) continue;
				BBox& cc = this->_collidable[k];
				if (intersect(get_hitbox(rel(cc, 0.0, this->grav_dir)), cs) &&
					!intersect(get_hitbox(cc), cs))
				{
					if (cs.dx != 0)
					{	// it moves horizontally
						// try to move horizontally as well
						int carryX = cs.dx;
						done_segments[i] = true;
						if (place_free(get_hitbox(rel(cc, carryX, 0.0))))
						{	// nothing stands in our way
							cc.x += carryX;
						}
						else
						{	// something is standing in our way
							double dist = 0.0;
							if (carryX > 0)
							{	// wanna go right
								dist = project_free_right(cc);
								if (dist < carryX)
									dist = round(dist);
								else
									dist = carryX;
							}
							else
							{	// wanna go left
								dist = project_free_left(cc);
								if (dist < -carryX)
									dist = -round(dist);
								else
									dist = carryX;
							}
							cc.x += dist;
						}
					}
					if (cs.dy * this->grav_dir > 0)
					{	// it (also) moves down
						int carryY = cs.dy;
						// move the solid down, so it doesn't register as collision
						cs.y += cs.dy;
						done_segments[i] = true;
						if (place_free(get_hitbox(rel(cc, 0.0, carryY))))
						{	// nothing stands in our way
							cc.y += carryY;
						}
						else
						{	// something is in our way
							double dist = 0.0;
							if (grav_dir > 0)
							{
								dist = project_free_down(cc);
								if (dist < carryY)
									dist = round(dist);
								else
									dist = carryY;
							}
							else
							{
								dist = project_free_up(cc);
								if (dist < -carryY)
									dist = -round(dist);
								else
									dist = carryY;
							}
							cc.y += dist;

						}
						// have to move back up to move it properly later
						cs.y -= cs.dy;
					}
				}
			}
			// after all collidables were carried, move the solid that did the carry, 
			// since bc of downwards carrying collidable will be inside a solid, 
			// and we want to ignore that
			if (done_segments[i])
			{
				cs.x += cs.dx;
				cs.y += cs.dy;
			}
		}

		// carry finished. 
		// all objects marked true in done_ have been moved, and no longer
		// collide with collidables
		// all objects not marked true in done_
		// should resolve collision by pushing

		for (size_t i = 0; i < this->_solidsC; i++)
		{
			if (done_solids[i]) continue;
			Hitbox& cs = this->_solids[i];
			for (size_t k = 0; k < this->_collidableC; k++)
			{
				if (!this->alive[k]) continue;
				BBox& cc = this->_collidable[k];
				if (intersect(rel(cs, cs.dx, cs.dy), get_hitbox(cc)))
				{	// the collision will happen. push the collidable
					// we use this function to see what side collidable
					// will meet the solid
					CollisionSide side = collision_side(cc, cs, cs.dx, cs.dy);
					switch (side)
					{
					case CollisionSide::NONE:
						// error?
					break;
					case CollisionSide::LEFT:
						// will be pushed from left
						cc.x = right(cs) + cs.dx;
					break;
					case CollisionSide::TOP:
						// will be pushed from up
						// can do some check here to see if fell through platform (death)
						if (grav_dir > 0 && standing[k])
						{
							this->alive[k] = false;
						}
						cc.y = bottom(cs) + cs.dy;
					break;
					case CollisionSide::RIGHT:
						// will be pushed from right
						cc.x = left(cs) + cs.dx - cc.width;
					break;
					case CollisionSide::BOTTOM:
						// will be pushed from down
						if (grav_dir < 0 && standing[k])
						{
							this->alive[k] = false;
						}
						cc.y = top(cs) + cs.dy - cc.height;
					break;

					}
				}
			}
			// after everything is pushed, we can move the solid
			cs.x += cs.dx;
			cs.y += cs.dy;
		}

		// do platforms pushing
		for (size_t i = 0; i < this->_segmentsC; i++)
		{
			if (done_segments[i]) continue;
			Segment& cs = this->_segments[i];
			for (size_t k = 0; k < this->_collidableC; k++)
			{
				if (!this->alive[k]) continue;
				BBox& cc = this->_collidable[k];
				if (intersect(get_hitbox(cc), rel(cs, cs.dx, cs.dy)) &&
					!intersect(get_hitbox(cc), cs))
				{	// we will intersect, but can't tell whether we need to stop 
					if (cs.vertical)
					{
						if (cs.dx < 0)
						{	// seg was going left
							double dist = project_right(cc, cs);	// should the seg push us
							if (dist < -cs.dx)
							{	
								cc.x = cs.x + cs.dx - cc.width;
							}
						}
						else
						{	// seg was going right
							double dist = project_left(cc, cs);		// shoud the seg push us
							if (dist < cs.dx)
							{
								cc.x = cs.x + cs.dx;
							}
						}
					}
					else
					{	
						if (cs.dy < 0)
						{	// seg was going up
							double dist = project_down(cc, cs);
							if (dist < -cs.dy)
							{
								cc.y = cs.y + cs.dy - cc.height;
							}
						}
						else
						{	// seg was going down
							double dist = project_up(cc, cs);
							if (dist < cs.dy)
							{
								cc.y = cs.y + cs.dy;
							}
						}
					}
				}
			}
			cs.x += cs.dx;
			cs.y += cs.dy;
		}

		// now we can finally apply movement to collidables
		// most stuff down here is done for compatibility with fangame physics
		for (size_t i = 0; i < this->_collidableC; i++)
		{
			// if already dead or have to die, no hesitation
			if (place_solid(get_hitbox(this->_collidable[i]))) this->alive[i] = false;
			if (!this->alive[i]) continue;

			BBox& cc = this->_collidable[i];

			// see if our desired destination is clear
			if (!place_free(get_hitbox(rel(cc, cc.dx, cc.dy))))
			{	
				bool canX = false, canY = false;
				if (cc.dx < 0)
				{	// going left
					double dist = project_free_left(cc);
					if (dist < -cc.dx)
					{	// will hit a thing
						canX = false;
						cc.x -= round(dist);
						cc.dx = 0.0;
					}
					else
						canX = true;
				}
				else
				{	// going right perhaps
					double dist = project_free_right(cc);
					if (dist < cc.dx)
					{	// will hit a thing
						canX = false;
						cc.x += round(dist);
						cc.dx = 0.0;
					}
					else
						canX = true;
				}
				if (cc.dy < 0)
				{	// going up
					double dist = project_free_up(cc);
					if (dist < -cc.dy)
					{	// will hit a thing
						canY = false;
						cc.y -= round(dist);
						cc.dy = 0.0;
					}
					else
						canY = true;
				}
				else
				{	// going down probably
					double dist = project_free_down(cc);
					if (dist < cc.dy)
					{	// will hit a thing
						canY = false;
						cc.y += round(dist);
						cc.dy = 0.0;
					}
					else
						canY = true;
				}
				if (canX && canY)
				{	// corner collision. do only if solid
					if (place_solid(get_hitbox(rel(cc, cc.dx, cc.dy))))
					{
						cc.dx = 0;
						cc.y += cc.dy;
					}
					else
					{
						cc.x += cc.dx;
						cc.y += cc.dy;
					}
				}	
				else
				{
					cc.x += cc.dx;
					cc.y += cc.dy;
				}
			}
			else
			{	// all clear
				cc.x += cc.dx;
				cc.y += cc.dy;
			}
		}
	}
}
