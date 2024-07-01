
// in "dir/file.ext" this returns "ext". Returns an empty string if there is no extension
string get_file_extension(string path) {
	
	if (path.count <= 0) return ZERO(string);
	
	for (s64 i = path.count-1; i >= 0; i--) {
	
		if (path.data[i] == '/' || path.data[i] == '\\' || path.data[i] == ':') {
			return ZERO(string);
		}
		
		if (path.data[i] == '.') {
			string ext = ZERO(string);
			ext.count = path.count-i;
			if (ext.count > 0) ext.data = path.data+i;
			return ext;
		}
	}
	
	return ZERO(string);
}