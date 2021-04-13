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
// SPLIT NON-NITRO MODE
//
#if !defined(SPLITMERGE_NITRO_MODE)

#define WELCOME_MSG         SPLITMERGE_WELCOME_MSG
#define FILE_LIMIT          SPLITMERGE_FILE_LIMIT
#define MAX_FIRST_FILE_SIZE SPLITMERGE_MAX_FIRST_FILE_SIZE

#endif


//~~~~~~~~~~~~~~~~
//
// SPLIT NITRO MODE
//
#if defined(SPLITMERGE_NITRO_MODE)

#define WELCOME_MSG         SPLITMERGE_NITRO_WELCOME_MSG
#define FILE_LIMIT          SPLITMERGE_NITRO_FILE_LIMIT
#define MAX_FIRST_FILE_SIZE SPLITMERGE_NITRO_MAX_FIRST_FILE_SIZE

#endif


//~~~~~~~~~~~~~~~~
//
//
//
static i32
get_split_count(i64 file_size) {
    i32 result = 0;
    
    if(file_size >= MAX_FIRST_FILE_SIZE) {
        while(file_size > 0) {
            file_size -= FILE_LIMIT;
            result    += 1;
        }
    }
    
    return result;
}

static void
append_file_data(File_Data *file, u8 *data, i64 length) {
    if(file) {
        if(length >= file->capacity - file->length) {
            length = file->capacity - file->length;
        }
        
        For(i64, it_index, length) {
            file->data[file->length] = data[it_index];
            file->length += 1;
        }
    }
}


//~~~~~~~~~~~~~~~~
//
// MAIN
//
int
main(int arg_count, char **arg_data) {
    printf("%s <split>\n", WELCOME_MSG);
    printf("%d potential files to split.\n", arg_count - 1);
    
    u64 random_seed = os_set_random_seed();
    
    String file_name   = make_string(128);
    String output_path = make_string(128);
    String source_path;
    
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
        
        source_path = output_path;
        
        append_cstring(&output_path, UNPACK_NTSTRING("split_output/0x"));
    }
    
    for_range(i32, arg_index, 1, arg_count) {
        printf("---===##===---\n");
        
        file_name.length = 0;
        
        String arg = set_string_from_ntstring(arg_data[arg_index]);
        
        if(begins_with_cstring(arg, UNPACK_NTSTRING("..\\")) ||
           begins_with_cstring(arg, UNPACK_NTSTRING("../")))
        {
            append_string(&file_name, source_path);
            
            i32 up_count = count_instance_of_cstring(arg, UNPACK_NTSTRING("../"));
            up_count += count_instance_of_cstring(arg, UNPACK_NTSTRING("..\\"));
            
            while(up_count > 0) {
                i32 index = index_of_parent_path(file_name);
                
                if(index >= 0) {
                    file_name.length -= file_name.length - index - 1;
                }
                
                up_count -= 1;
            }
        } else {
            append_string(&file_name, arg);
        }
        
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
        
        File_Handle file_handle = os_open_file_for_reading(arg.data);
        
        if(os_is_handle_valid(file_handle)) {
            i64 file_size       = os_get_size_of_file(file_handle);
            i64 total_file_size = file_size;
            
            i32 split_count = get_split_count(total_file_size + file_name.length);
            
            if(split_count > 0) {
                File_Data file = make_file_data(FILE_LIMIT);
                
                String out_file_name = make_string(128);
                
                Shared_Header shared_header = {0};
                
                shared_header.validation_0 = SPLITMERGE_HEADER_VALIDATION[0];
                shared_header.validation_1 = SPLITMERGE_HEADER_VALIDATION[1];
                shared_header.validation_2 = SPLITMERGE_HEADER_VALIDATION[2];
                shared_header.version      = SPLITMERGE_FILE_VERSION;
                shared_header.unique_id    = (u32)os_get_random_u64(&random_seed);
                
                if(is_big_endian()) {
                    shared_header.flags |= Header_Flag__Big_Endian;
                }
                
                For(i32, it_index, split_count) {
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
                    
                    printf("Splitting file %d/%d\n", it_index + 1, split_count);
                    
                    if(os_read_file(&file, file_handle, file.capacity - file.length) > 0) {
                        out_file_name.length = 0;
                        
                        append_cstring(&out_file_name, output_path.data, output_path.length);
                        append_u32(&out_file_name, shared_header.unique_id, 16);
                        append_char(&out_file_name, '_');
                        append_u32(&out_file_name, shared_header.file_index, 10);
                        append_cstring(&out_file_name, SPLITMERGE_FILE_EXTENSION_CSTRING);
                        null_terminate(&out_file_name);
                        
                        File_Handle out_file_handle = os_open_file_for_writing(out_file_name.data);
                        
                        os_write_file(out_file_handle, file.data, file.length);
                        
                        os_close_file(out_file_handle);
                        
                        shared_header.file_index += 1;
                    }
                }
                
                SPLTMRG_FREE(file.data);
                SPLTMRG_FREE(out_file_name.data);
            } else {
                printf("%s is too small, minimum file size is %lld bytes\n",
                       arg.data, (MAX_FIRST_FILE_SIZE + 1));
            }
        } else {
            printf("Invalid file: \"%s\"\n", arg.data);
        }
        
        os_close_file(file_handle);
    }
    
    return 0;
}
