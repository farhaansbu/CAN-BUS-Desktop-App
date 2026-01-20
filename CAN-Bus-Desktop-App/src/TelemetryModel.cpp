#include "TelemetryModel.h"

void TelemetryModel::ingest(const CanFrame& frame)
{
	frames_.push_back(frame);
	if (frames_.size() > max_frames_) 
		frames_.pop_front();
	
	decodeAndUpdate(frame);

}


const VehicleSignals& TelemetryModel::signals() const
{
	return signals_;
}

const std::deque<CanFrame>& TelemetryModel::frames() const
{
	return frames_;
}

void TelemetryModel::setMaxFrames(size_t n)
{
	max_frames_ = std::max<size_t>(1,n);
	if (frames_.size() > max_frames_) 
	{
		frames_.erase(frames_.begin(), frames_.begin() + (frames_.size() - max_frames_));
	}
}

// Decoding loigc
void TelemetryModel::decodeAndUpdate(const CanFrame& f)
{
    // Adjust based on CAN ID
    switch (f.id)
    {
        // -------------------------------
        // Vehicle speed (OBD-II PID 0x0D)
        // -------------------------------
    case 0x0D:
    {
        if (f.dlc < 1) break;

        // f.data[0] = speed in km/h
        float speed_kph = static_cast<float>(f.data[0]);
        signals_.speed_mph = speed_kph * 0.621371f;
        break;
    }

    // -------------------------------
    // Engine RPM (OBD-II PID 0x0C)
    // Formula: RPM = ((A * 256) + B) / 4
    // -------------------------------
    case 0x0C:
    {
        if (f.dlc < 2) break;

        uint16_t raw = (static_cast<uint16_t>(f.data[0]) << 8)
            | static_cast<uint16_t>(f.data[1]);

        signals_.rpm = static_cast<float>(raw) / 4.0f;
        break;
    }

    // -------------------------------
    // Throttle position (PID 0x11)
    // Formula: A * 100 / 255
    // -------------------------------
    case 0x11:
    {
        if (f.dlc < 1) break;

        signals_.throttle_pct =
            static_cast<float>(f.data[0]) * 100.0f / 255.0f;
        break;
    }

    // -------------------------------
    // Coolant temperature (PID 0x05)
    // Formula: A - 40 (°C)
    // -------------------------------
    case 0x05:
    {
        if (f.dlc < 1) break;

        signals_.coolant_temp_c =
            static_cast<float>(f.data[0]) - 40.0f;
        break;
    }

    } // End of switch
}
