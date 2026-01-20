#ifndef TELEMETRYMODEL_H
#define TELEMETRYMODEL_H

#include <deque>
#include "CanFrame.h"

struct VehicleSignals {
	float speed_mph{ 0.0f };
	float rpm{ 0.0f };
	float throttle_pct{ 0.0f };
	float coolant_temp_c{ 0.0f };
	// add as needed
};

struct TelmetrySnapshot {
	VehicleSignals signals;
	std::deque<CanFrame> recent_frames;
};

class TelemetryModel {
public:

	void ingest(const CanFrame& f);
	const VehicleSignals& signals() const;
	const std::deque<CanFrame>& frames() const;
	void setMaxFrames(size_t n);

private:
	void decodeAndUpdate(const CanFrame& f);
	size_t max_frames_{ 20 };
	VehicleSignals signals_;
	std::deque<CanFrame> frames_;

};






#endif
