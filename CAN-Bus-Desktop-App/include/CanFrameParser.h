#ifndef CANFRAMEPARSER_H
#define CANFRAMEPARSER_H

#include <vector>
#include "CanFrame.h"


class CanFrameParser {

public:
	void pushBytes(const uint8_t* data, size_t size);
	bool tryPop(CanFrame& out, double timestamp);


private:
	std::vector<uint8_t> buffer_;
	size_t read_offset_{ 0 };

};



#endif
