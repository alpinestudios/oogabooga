
// #Cleanup

// usage example: hex_to_rgba(0x2a2d3aff);
Vector4 hex_to_rgba(s64 hex) {
	u8 r = (hex>>24) & 0x000000FF;
	u8 g = (hex>>16) & 0x000000FF;
	u8 b = (hex>>8) & 0x000000FF;
	u8 a = (hex>>0) & 0x000000FF;
	return (Vector4){r/255.0, g/255.0, b/255.0, a/255.0};
}

// todo - hsv conversion stuff when it's needed