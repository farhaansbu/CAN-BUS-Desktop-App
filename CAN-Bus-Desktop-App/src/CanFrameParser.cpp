#include <iostream>
#include "CanFrameParser.h"


void CanFrameParser::pushBytes(const uint8_t* data, size_t size)
{
	for (size_t i{ 0 }; i < size; ++i)
	{
		buffer_.push_back(data[i]);
	}
}

bool CanFrameParser::tryPop(CanFrame& out, double timestamp)
{
	if (buffer_.size() - read_offset_ < CAN_FRAME_SIZE)
	{
		return false;
	}

	const uint8_t* p = buffer_.data() + read_offset_;

	// parse id big-endian
	out.id = static_cast<uint32_t>(p[3]) |
			(static_cast<uint32_t>(p[2]) << 8) |
			(static_cast<uint32_t>(p[1]) << 16) |
			(static_cast<uint32_t>(p[0]) << 24);

	// copy data length code
	out.dlc = p[4];

	// copy data
	std::copy(p + 5, p + 13, out.data.begin());
	out.timestamp_s = timestamp;

	read_offset_ += CAN_FRAME_SIZE;

	// Compact buffer occasionally
	if (read_offset_ > 1024)
	{
		buffer_.erase(buffer_.begin(), buffer_.begin() + read_offset_);
		read_offset_ = 0;
	}

	return true;

}