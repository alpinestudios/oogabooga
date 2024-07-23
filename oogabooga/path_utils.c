
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

string get_file_name_including_extension(string file_path) {
    if (file_path.count <= 0) return ZERO(string);

    s64 last_separator = -1;
    for (s64 i = file_path.count - 1; i >= 0; i--) {
        if (file_path.data[i] == '/' || file_path.data[i] == '\\' || file_path.data[i] == ':') {
            last_separator = i;
            break;
        }
    }

    string file_name = ZERO(string);
    if (last_separator != -1 && last_separator < file_path.count - 1) {
        file_name.data = file_path.data + last_separator + 1;
        file_name.count = file_path.count - last_separator - 1;
    } else {
        file_name = file_path; // If no separator was found, assume entire path is a file name.
    }

    return file_name;
}
string get_file_name_excluding_extension(string file_path) {
	string file_name = get_file_name_including_extension(file_path);
	
	for (s64 i = file_name.count-1; i >= 1; i--) {
		if (file_name.data[i] == '.') {
			return string_view(file_name, 0, i);
		}
	}
	return file_name;
}

string get_directory_of(string path) {
    if (path.count <= 0) return ZERO(string);
    
    for (u64 i = path.count; i >= 0; i--) {
        if (path.data[i] == '/' || path.data[i] == '\\' || path.data[i] == ':') {
            return string_view(path, 0, i);
        }
    }
    
    return ZERO(string);
}