// really basic type for unsigned byte colors
#pragma once

// intentionally a POD (plain old data) type
// can have constructors & member functions, but nothing virtual
template <typename T>
struct Color {
	typedef T value_type;	// external typedef for template type

	T r, g, b;				// public access to color data
};

typedef Color<unsigned char> uColor;
typedef Color<float>         fColor;
