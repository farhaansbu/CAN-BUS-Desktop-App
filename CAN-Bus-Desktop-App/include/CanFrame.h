#ifndef CANFRAME_H
#define CANFRAME_H

constexpr size_t CAN_FRAME_SIZE{ 13 };

#include <cstdint>
#include <array>

struct CanFrame
{
	uint32_t id{ 0 };
	uint8_t dlc{ 0 };
	std::array<uint8_t, 8> data{};
	double timestamp_s{ 0 };
};


#endif

