#pragma once

#ifdef GAMING_EXPORTS
#define GAMING_API __declspec(dllexport)
#else
#define GAMING_API __declspec(dllimport)
#endif

#include "HeapIt.h"
#include <random>

constexpr float scr_width{ 800.0f };
constexpr float scr_height{ 700.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 650.0f };

constexpr uint8_t no_type{ 0b00000000 };
constexpr uint8_t evil1{ 0b00000001 };
constexpr uint8_t evil2{ 0b00000010 };
constexpr uint8_t evil3{ 0b00000100 };
constexpr uint8_t civilian{ 0b00001000 };
constexpr uint8_t hero{ 0b00010000 };
constexpr uint8_t supply{ 0b00100000 };
constexpr uint8_t bullet{ 0b01000000 };

enum class dirs { up = 0, down = 1, left = 2, right = 3, stop = 4 };

struct GAMING_API FPOINT
{
	float x{ 0 };
	float y{ 0 };
};

namespace dll
{
	class GAMING_API RandIt
	{
	private:
		std::random_device rd{};
		std::seed_seq* sq{ nullptr };
		std::mt19937* twister{ nullptr };

	public:
		RandIt();
		~RandIt();

		int operator() (int min, int max);
	};

	class GAMING_API PROTON
	{
	protected:
		float width{ 1.0f };
		float height{ 1.0f };

	public:
		FPOINT start{ 0,0 };
		FPOINT end{ 0,0 };
		FPOINT center{ 0,0 };

		PROTON(float sx = 0, float sy = 0, float s_width = 1.0f, float s_height = 1.0f);
		virtual ~PROTON() {};

		void SetEdges();
		void NewDims(float new_width, float new_height);

		float GetWidth() const;
		float GetHeight() const;
	};

	class GAMING_API BASE :public PROTON
	{
	protected:
		uint8_t type = no_type;

		float slope = 0;
		float intercept = 0;

		float move_sx = 0;
		float move_ex = 0;
		float move_sy = 0;
		float move_ey = 0;

		bool vert_line = false;
		bool hor_line = false;

		int frame = 0;
		int frame_delay = 0;
		int max_frames = 0;

		void SetPathInfo(float _where_x, float _where_y);

	public:
		dirs dir = dirs::stop;

		BASE(uint8_t _what, float _sx, float _sy);
		virtual ~BASE() {};

		uint8_t GetType()const;
		void SetType(uint8_t to_what);

		int GetFrame();

		virtual bool Move(float gear) = 0;
		virtual void Release() = 0;
	};

	class GAMING_API OBJECTS :public BASE
	{
	private:
		float speed = 0;
		
		OBJECTS(uint8_t what, float init_x, float init_y, float bull_dest_x = 0, float bull_dest_y = 0);
	
	public:
		int strenght = 0;


		friend GAMING_API OBJECTS* ObjectFactory(uint8_t which_object, float deploy_x, float deploy_y,
			float bullet_dest_x, float bullet_dest_y);

		bool Move(float gear) override;
		void Release() override;
	};

	class GAMING_API CREATURES :public BASE
	{
	private:
		float speed = 0;
		int attack_delay = 150;

		CREATURES(uint8_t what, float init_x, float init_y);

	public:
		int strenght = 0;
		int lifes = 0;

		float Distance(FPOINT myPoint, FPOINT refPoint);

		FPOINT Sort(cont::CONTAINER<FPOINT> refContainer);

		bool AIShoot(cont::CONTAINER<FPOINT> civs, FPOINT Hero_pos, FPOINT where_to_shoot);
		
		friend GAMING_API CREATURES* CreatureFactory(uint8_t which_object, float deploy_x, float deploy_y);

		bool Move(float gear) override;
		void Release() override;
	};

	////////////////////////////////////////////

	typedef OBJECTS* Asset;
	typedef CREATURES* Creature;

	////////////////////////////////////////////

	GAMING_API OBJECTS* ObjectFactory(uint8_t which_object, float deploy_x, float deploy_y,
		float bullet_dest_x = 0, float bullet_dest_y = 0);

	GAMING_API Creature CreatureFactory(uint8_t which_object, float deploy_x, float deploy_y);
}