#pragma once

struct UpdateEvent {
	float dt;

	operator const float&() const { return dt; }
	operator float& () { return dt; }
};