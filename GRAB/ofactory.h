#pragma once

#ifdef OFACTORY_EXPORTS 
#define OFACTORY_API _declspec(dllexport)
#else 
#define OFACTORY_API _declspec(dllimport)
#endif

enum class types {
	big_gold = 0, big_silver = 1, mid_gold = 2, mid_silver = 3, sm_gold = 4, sm_silver = 5,
	bag = 6, no_type = 7
};

class OBJECT
{
	protected:
		float _width = 0;
		float _height = 0;

	public:
		float x = 0;
		float y = 0;
		float ex = 0;
		float ey = 0;
	
		OBJECT(float __x, float __y, float __width, float __height)
		{
			x = __x;
			y = __y;
			_width = __width;
			_height = __height;
			ex = x + _width;
			ey = y + _height;
		}

		virtual ~OBJECT(){}

		void SetNewDims(float __new_width, float __new_height)
		{
			_width = __new_width;
			_height = __new_height;
			ex = x + _width;
			ey = y + _height;
		}

		void SetEdges()
		{
			x += _width;
			y += _height;
		}
};
class BENEFIT :public OBJECT
{
	public:
		types type = types::no_type;
		int bonus = -1;
		int weight = 0;

		BENEFIT(types __type, float _bx,float _by):OBJECT(_bx,_by,1.0f,1.0f)
		{
			type = __type;

			switch (__type)
			{
				case types::big_gold:
					SetNewDims(40.0f, 37.0f);
					bonus = 20;
					weight = 40;
					break;

				case types::bag:
					SetNewDims(30.0f, 30.0f);
					weight = 10;
					break;

				case types::big_silver:
					SetNewDims(40.0f, 29.0f);
					bonus = 15;
					weight = 35;
					break;

				case types::mid_gold:
					SetNewDims(30.0f, 34.0f);
					bonus = 25;
					weight = 30;
					break;

				case types::mid_silver:
					SetNewDims(30.0f, 30.0f);
					bonus = 18;
					weight = 25;
					break;

				case types::sm_gold:
					SetNewDims(20.0f, 32.0f);
					bonus = 35;
					weight = 10;
					break;

				case types::sm_silver:
					SetNewDims(20.0f, 30.0f);
					bonus = 35;
					weight = 10;
					break;
			}
		}

		~BENEFIT() override
		{
			delete this;
		}
};
class HEAD :public OBJECT
{
	private:
		float lambda = 1.0f;
		float speed = 10.0f;
		int frame = 0;
		int frame_delay = 5;

	public:
		
		int max_heavy_delay = 0;
		int heavy_delay = 0;
		bool forward = true;
		bool moving = false;
		types cargo_attached = types::no_type;
		
		HEAD(float __hx, float __hy, float __speed = 10.0f) :OBJECT(__hx, __hy, 50.0f, 55.0f)
		{
			forward = true;
			lambda = 1.0f;
			speed = __speed;
		}

		~HEAD() override
		{
			delete this;
		}

		int GetFrame()
		{
			frame_delay--;
			if (frame_delay <= 0)
			{
				frame_delay = 5;
				frame++;
				if (frame > 75)frame = 0;
			}
			return frame;
		}

		float GetLambda(float __target_y)
		{
			if (x - 750.0f == 0)return 0.0f;
			else
			{
				lambda = (__target_y - y) / (750.0f - x);
				if (lambda < 0)lambda = -lambda;
			}

			if (y > __target_y)lambda = -lambda;
			return lambda;
		}

		bool Move()
		{
			if (!moving)return true; //COMMON CASE - MOVING OR IDLE
			if (forward)
			{
				if (ex + speed <= 800.0f && ey + lambda * speed <= 600 && y + lambda * speed > 50.0f)
				{
					x += speed;
					y += lambda * speed;
					SetEdges();
				}
				else
				{
					forward = false;
					return true;
				}
			}
			else
			{
				if (heavy_delay > 0)
				{
					heavy_delay--;
					return true;
				}
				else
				{
					heavy_delay = max_heavy_delay;
					if (x - speed >= 50.0f)
					{
						x -= speed;
						y += speed * lambda;
						SetEdges();
					}
					else
					{
						if (y > 270)
						{
							y -= speed;
							SetEdges();
						}
						if (y < 250)
						{
							y += speed;
							SetEdges();
						}
						if (y >= 250 && y <= 270)
						{
							moving = false;
							return false; //return in base
						}
					}
				}
			}
			return true; //COMMON CASE - MOVING OR IDLE
		}

};

typedef OBJECT* object_ptr;
typedef BENEFIT* benefit_ptr;
typedef HEAD* head_ptr;

extern OFACTORY_API benefit_ptr iCreate(types _what, float _where_x, float _where_y);