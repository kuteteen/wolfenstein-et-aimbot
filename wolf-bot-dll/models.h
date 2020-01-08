#include <Windows.h>
#include <stdio.h>

class BotCoordinate {
public:
	long x;
	long y;
	BotCoordinate(long x, long y) {
		this->x = x;
		this->y = y;
	}
	BotCoordinate diff(BotCoordinate coord2) {
		return BotCoordinate(x - coord2.x, y - coord2.y);
	}
	bool equals(BotCoordinate coord2) {
		return x == coord2.x && y == coord2.y;
	}
	bool lessThan(long i) {
		return abs(x) < i && abs(y) < i;
	}


};

// This class needs to be 3 bytes long. Do not add virtual functions since this will add a vtable to the struct
class Rgb {

public:
	BYTE r;
	BYTE g;
	BYTE b;
	bool rgbInRange(Rgb compare, long range) {
		return abs((long)r - (long)compare.r) <= range &&
			abs((long)g - (long)compare.g) <= range &&
			abs((long)b - (long)compare.b) <= range;

	}
	bool rgbEqualPlayer() {
		Rgb player = { 0, 255, 0 };
		return rgbInRange(player, 50);
	}
	char* format(char* buffer) {
		sprintf(buffer, "Rgb(%ld,%ld,%ld)", r, g, b);
		return buffer;
	}
};
