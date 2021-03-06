//~~~~~~~~~~~~~~~~
// MIT License
//
// Copyright (c) 2021 Patrik Johansson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


//~~~~~~~~~~~~~~~~
//
// INCLUDES
//
#include "splitmerge.h"

#if defined(SPLITMERGE_WIN32)
#  include "win32_splitmerge.c"
#else
#  include "crt_splitmerge.c"
#endif


//~~~~~~~~~~~~~~~~
//
// PROCEDURES
//
static bool
is_big_endian() {
	union {int i; char c[sizeof(int)];} x;
    
	x.i = 1;
	if(x.c[0] == 1) {
		return false;
	}
    
	return true;
}

#define is_little_endian() (!is_big_endian())


static File_Data
make_file_data(i64 capacity) {
    File_Data result = {0};
    
    result.capacity = capacity;
    result.data     = SPLTMRG_ALLOC(u8, result.capacity);
    
    return result;
}


//~~~~~~~~~~~~~~~~
//
// STRING
//
static i32
get_length_of_ntstring(char *data) {
    i32 result = 0;
    
    if(data) {
		while(*data) {
			data   += 1;
			result += 1;
		}
	}
    
    return result;
}

static char
digit_value_to_char(i32 value) {
    char result = 0;
    
    if(value >= 0 && value <= 9) {
        result = '0' + (char)value;
    } else if(value >= 10 && value < 36) {
        result = 'A' + (char)(value - 10);
    }
    
    return result;
}

static String
set_string_from_ntstring(char *str) {
    String result = {0};
    
    result.data     = str;
    result.capacity = get_length_of_ntstring(result.data);
    result.length   = result.capacity;
    
    return result;
}

static String
make_string(i32 capacity) {
    String result = {0};
    
    result.capacity = capacity;
    result.data     = SPLTMRG_ALLOC(char, result.capacity);
    
    return result;
}

static i32
find_index_of_last(String str, char c) {
    i32 result = -1;
    
    if(str.data) {
		rfor(i32, it_index, str.length) {
			if(str.data[it_index] == c) {
				return it_index;
			}
		}
	}
    
    return result;
}

static bool
begins_with_char(String a, char b) {
    if(a.data && a.data[0] == b) {
        return true;
    }
    return false;
}

static bool
begins_with_cstring(String a, char *b_data, i32 b_length) {
    if(a.data && b_data && a.length >= b_length) {
        For(i32, it_index, b_length) {
            if(a.data[it_index] != b_data[it_index]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

static bool
begins_with_string(String a, String b) {
    return begins_with_cstring(a, b.data, b.length);
}

static bool
begins_with_ntstring(String a, char *b) {
    return begins_with_cstring(a, b, get_length_of_ntstring(b));
}

static bool
ends_with_char(String a, char b) {
    if(a.data && a.data[a.length - 1] == b) {
        return true;
    }
    return false;
}

static bool
ends_with_cstring(String a, char *b_data, i32 b_length) {
    if(a.data && b_data && a.length >= b_length) {
        rfor(i32, it_index, b_length) {
            if(a.data[a.length - (b_length - it_index)] != b_data[it_index]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

static bool
ends_with_string(String a, String b) {
    return ends_with_cstring(a, b.data, b.length);
}

static bool
ends_with_ntstring(String a, char *b) {
    return ends_with_cstring(a, b, get_length_of_ntstring(b));
}

static void
advance_string(String *str, i32 amount) {
    if(str) {
        str->data   += amount;
        str->length -= amount;
    }
}

static i32
count_instance_of_char(String a, char b) {
    i32 result = 0;
    
    if(a.data) {
        For(i32, it_index, a.length) {
            if(a.data[it_index] == b) {
                result += 1;
            }
        }
    }
    
    return result;
}

static i32
count_instance_of_cstring(String a, char *b_data, i32 b_length) {
    i32 result = 0;
    
    if(a.data && b_data) {
        while(a.length > 0) {
            if(begins_with_cstring(a, b_data, b_length)) {
                advance_string(&a, b_length);
                result += 1;
            } else {
                advance_string(&a, 1);
            }
        }
    }
    
    return result;
}

static i32
count_instance_of_string(String a, String b) {
    return count_instance_of_cstring(a, b.data, b.length);
}

static i32
count_instance_of_ntstring(String a, char *b) {
    return count_instance_of_cstring(a, b, get_length_of_ntstring(b));
}

static i32
index_of_parent_path(String str) {
    i32 result = -1;
    
    if(ends_with_char(str, '/') || ends_with_char(str, '\\')) {
        str.length -= 1;
    } else {
        i32 index = find_index_of_last(str, '/');
        
        if(index >= 0) {
            str.length -= str.length - index - 1;
        } else {
            index = find_index_of_last(str, '\\');
            
            if(index >= 0) {
                str.length -= str.length - index - 1;
            }
        }
    }
    
    result = find_index_of_last(str, '/');
    
    if(result < 0) {
        result = find_index_of_last(str, '\\');
    }
    
    return result;
}

static void
reverse_string(String str) {
    i32 count = str.length / 2;
    
    For(i32, it_index, count) {
        i32  reverse_index = str.length - 1 - it_index;
        char tmp           = str.data[it_index];
        
        str.data[it_index]      = str.data[reverse_index];
        str.data[reverse_index] = tmp;
    }
}

static void
replace_char(String *str, char a, char b) {
    if(str) {
        For(i32, it_index, str->length) {
            if(str->data[it_index] == a) {
                str->data[it_index] = b;
            }
        }
    }
}

static void
maybe_grow_string(String *str, i32 new_length) {
    if(str) {
        if(new_length > str->capacity) {
            while(new_length > str->capacity) {
                str->capacity += 64;
            }
            
            str->data = SPLTMRG_REALLOC(char, str->data, str->capacity);
        }
    }
}

static void
null_terminate(String *a) {
    if(a && a->data) {
        maybe_grow_string(a, a->length + 1);
        
        a->data[a->length] = 0;
    }
}

static void
append_char(String *a, char b) {
    if(a && a->data) {
        maybe_grow_string(a, a->length + 1);
        
        a->data[a->length]  = b;
        a->length          += 1;
    }
}

static void
append_cstring(String *a, char *b_data, i32 b_length) {
    if(a && a->data && b_data) {
        maybe_grow_string(a, a->length + b_length);
        
        For(i32, b_index, b_length) {
            a->data[a->length]  = b_data[b_index];
            a->length          += 1;
        }
    }
}

static void
append_string(String *a, String b) {
    append_cstring(a, b.data, b.length);
}

static void
append_ntstring(String *a, char *b) {
    append_cstring(a, b, get_length_of_ntstring(b));
}

static void
append_bits(String *str, u64 value, i32 base, bool is_signed) {
	if(str) {
		bool is_negative = false;
		i32 starting_length = str->length;
        
		if(is_signed) {
			i64 i = *(i64 *)&value;
			if(i < 0) {
				is_negative = true;
				value = -i;
			}
		}
        
		while(value >= base) {
			char digit = digit_value_to_char((i32)(u32)(value % base));
			append_char(str, digit);
			value /= base;
		}
        
		char digit = digit_value_to_char((i32)(u32)(value % base));
		append_char(str, digit);
        
		if(is_negative) {
			append_char(str, '-');
		}
        
        String s = *str;
        advance_string(&s, starting_length);
		reverse_string(s);
	}
}

static void
append_u16(String *str, u16 value, i32 base) {
    append_bits(str, (u64)value, base, false);
}

static void
append_u32(String *str, u32 value, i32 base) {
    append_bits(str, (u64)value, base, false);
}

static void
append_u64(String *str, u64 value, i32 base) {
    append_bits(str, value, base, false);
}
