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
#include "splitmerge.c"


//~~~~~~~~~~~~~~~~
//
//
//
static u32
get_unique_id() {
	u32 result = 0;
    
	u16 n0 = (u16)rand();
	u16 n1 = (u16)rand();
    
	result = ((u32)n0 | ((u32)n1 << 16));
    
	return result;
}

static void
append_file_data(File_Data *file, u8 *data, i64 length) {
    For(i64, it_index, length) {
        file->data[file->length] = data[it_index];
        file->length += 1;
    }
}


//~~~~~~~~~~~~~~~~
//
// MAIN
//
int
main(int arg_count, char **arg_data) {
    printf("SPLITMERGE <split>\n");
    printf("%d potential files to split.\n", arg_count - 1);
    
    srand((unsigned int)time(NULL));
    
    String output_path = make_string(128);
    
    {
        append_ntstring(&output_path, arg_data[0]);
        
        i32 index = find_index_of_last(output_path, '/');
        
        if(index >= 0) {
            output_path.length -= output_path.length - index - 1;
        } else {
            index = find_index_of_last(output_path, '\\');
            
            if(index >= 0) {
                output_path.length -= output_path.length - index - 1;
            }
        }
        
        append_cstring(&output_path, UNPACK_NTSTRING("split_output/0x"));
    }
    
    for_range(i32, arg_index, 1, arg_count) {
        printf("---===##===---\n");
        
        char *arg = arg_data[arg_index];
        
        String file_name = {0};
        file_name.data   = arg;
        file_name.length = get_length_of_ntstring(arg);
        
        {
            i32 index = find_index_of_last(file_name, '/');
            
            if(index >= 0) {
                advance_string(&file_name, index + 1);
            } else {
                index = find_index_of_last(file_name, '\\');
                
                if(index >= 0) {
                    advance_string(&file_name, index + 1);
                }
            }
        }
        
        File_Handle file_handle = open_file_for_reading(arg);
        
        if(is_file(file_handle)) {
            i64 file_size       = get_size_of_file(file_handle);
            i64 total_file_size = file_size;
            
            if(file_size > SPLITMERGE_MAX_FIRST_FILE_SIZE) {
                File_Data file = make_file_data(SPLITMERGE_FILE_LIMIT);
                
                String out_file_name = make_string(128);
                
                Shared_Header shared_header = {0};
                shared_header.validation_0 = 'S';
                shared_header.validation_1 = '+';
                shared_header.validation_2 = 'M';
                shared_header.version      = SPLITMERGE_FILE_VERSION;
                shared_header.unique_id    = get_unique_id();
                if(is_big_endian()) {
                    shared_header.flags |= Header_Flag__Big_Endian;
                }
                
                while(file_size > 0) {
                    file.length = 0;
                    
                    if(shared_header.file_index == 0) {
                        First_Header header = {0};
                        header.shared           = shared_header;
                        header.file_name_length = (u16)file_name.length;
                        
                        u8 null_byte = 0;
                        
                        append_file_data(&file, (u8*)&header, sizeof(First_Header));
                        append_file_data(&file, (u8*)file_name.data, file_name.length);
                        append_file_data(&file, &null_byte, 1);
                    } else {
                        append_file_data(&file, (u8*)&shared_header, sizeof(Shared_Header));
                    }
                    
                    if(read_file(&file, file_handle, file.capacity - file.length) > 0) {
                        out_file_name.length = 0;
                        
                        append_cstring(&out_file_name, output_path.data, output_path.length);
                        append_u32(&out_file_name, shared_header.unique_id, 16);
                        append_char(&out_file_name, '_');
                        append_u32(&out_file_name, shared_header.file_index, 10);
                        append_cstring(&out_file_name, UNPACK_NTSTRING(".spltmrg"));
                        null_terminate(&out_file_name);
                        
                        File_Handle out_file_handle = open_file_for_writing(out_file_name.data);
                        
                        write_data_to_file(out_file_handle, file.data, file.length);
                        
                        close_file(out_file_handle);
                        
                        shared_header.file_index += 1;
                    }
                    
                    file_size = get_remaining_size_of_file(file_handle);
                    
                    {
                        float p = (float)file_size / (float)total_file_size;
                        printf("Splitting file %.2f%%\n", (1.0f - p) * 100.0f);
                    }
                }
                
                ZI_FREE(file.data);
            }
        }
        
        close_file(file_handle);
    }
    
    return 0;
}
