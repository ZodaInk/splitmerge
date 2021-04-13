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
// TYPES
//
typedef struct Merge_Bundle {
    String *files;
    u32     file_count;
    u32     file_capacity;
    
    u32    unique_id;
    String out_file_name;
} Merge_Bundle;

typedef struct Merge_Bundle_Array {
    Merge_Bundle *data;
    i32 count;
    i32 capacity;
} Merge_Bundle_Array;


//~~~~~~~~~~~~~~~~
//
// HEADER
//
static bool
is_valid_header(Shared_Header header) {
    if(header.validation_0 == SPLITMERGE_HEADER_VALIDATION[0] &&
       header.validation_1 == SPLITMERGE_HEADER_VALIDATION[1] &&
       header.validation_2 == SPLITMERGE_HEADER_VALIDATION[2])
    {
        return true;
    }
    return false;
}


//~~~~~~~~~~~~~~~~
//
// ENDIAN
//
static u16
swap_endian_u16(u16 value) {
    u16 result = ((value << 8) & 0xff00 |
                  (value >> 8) & 0x00ff);
    
    return result;
}

static u32
swap_endian_u32(u32 value) {
    u32 result = ((value << 24) & 0xff000000 |
                  (value <<  8) & 0x00ff0000 |
                  (value >>  8) & 0x0000ff00 |
                  (value >> 24) & 0x000000ff);
    
    return result;
}

static u64
swap_endian_u64(u64 value) {
    u64 result = ((value << 32) & 0xffffffff00000000 |
                  (value >> 32) & 0x00000000ffffffff);
    
    result = ((result << 16) & 0xffff0000ffff0000 |
              (result >> 16) & 0x0000ffff0000ffff);
    
    result = ((result << 8) & 0xff00ff00ff00ff00 |
              (result >> 8) & 0x00ff00ff00ff00ff);
    
    return result;
}

static bool
should_swap_endian(u8 flags) {
    if(is_big_endian() != (flags & Header_Flag__Big_Endian)) {
        return true;
    }
    return false;
}

static Merge_Bundle
make_merge_bundle(u32 unique_id) {
    Merge_Bundle result = {0};
    
    result.unique_id     = unique_id;
    result.file_capacity = 128;
    result.files         = SPLTMRG_ALLOC(String, result.file_capacity);
    
    return result;
}


//~~~~~~~~~~~~~~~~
//
// BUNDLE
//
static void
append_bundle(Merge_Bundle_Array *array, Merge_Bundle bundle) {
    if(array) {
        if(array->count + 1 >= array->capacity) {
            array->capacity += 4;
            array->data = SPLTMRG_REALLOC(Merge_Bundle, array->data, array->capacity);
        }
        
        array->data[array->count] = bundle;
        array->count += 1;
    }
}

static void
append_file_name(Merge_Bundle *bundle, String file_name, u32 file_index) {
    if(bundle) {
        if(file_index >= bundle->file_capacity) {
            while(file_index >= bundle->file_capacity) {
                bundle->file_capacity += 128;
            }
            bundle->files = SPLTMRG_REALLOC(String, bundle->files, bundle->file_capacity);
        }
        
        bundle->files[file_index] = file_name;
        bundle->file_count += 1;
    }
}


//~~~~~~~~~~~~~~~~
//
// MAIN
//
int
main(int arg_count, char **arg_data) {
    Merge_Bundle_Array master_list = {0};
    master_list.capacity = 4;
    master_list.data     = SPLTMRG_ALLOC(Merge_Bundle, master_list.capacity);
    
    printf("SPLITMERGE <merge>\n");
    printf("%d potential split files.\n", arg_count - 1);
    
    String source_path = set_string_from_ntstring(arg_data[0]);
    
    for_range(int, arg_index, 1, arg_count) {
        String arg = set_string_from_ntstring(arg_data[arg_index]);
        
        if(ends_with_cstring(arg, SPLITMERGE_FILE_EXTENSION_CSTRING)) {
            File_Handle file_handle = os_open_file_for_reading(arg.data);
            
            if(os_is_handle_valid(file_handle)) {
                File_Data file = make_file_data(SPLITMERGE_MAX_FILE_NAME_AND_HEADER_SIZE);
                
                if(os_read_file(&file, file_handle, sizeof(First_Header)) > 0) {
                    Shared_Header *shared_header = (Shared_Header*)file.data;
                    
                    if(is_valid_header(*shared_header)) {
                        Merge_Bundle *bundle = 0;
                        
                        if(should_swap_endian(shared_header->flags)) {
                            shared_header->version    = swap_endian_u16(shared_header->version);
                            shared_header->unique_id  = swap_endian_u32(shared_header->unique_id);
                            shared_header->file_index = swap_endian_u16(shared_header->file_index);
                        }
                        
                        For(i32, it_index, master_list.count) {
                            Merge_Bundle *it = master_list.data + it_index;
                            
                            if(it->unique_id == shared_header->unique_id) {
                                bundle = it;
                                break;
                            }
                        }
                        
                        if(!bundle) {
                            Merge_Bundle new_bundle = make_merge_bundle(shared_header->unique_id);
                            append_bundle(&master_list, new_bundle);
                            bundle = &master_list.data[master_list.count - 1];
                        }
                        
                        if(shared_header->file_index == 0) {
                            First_Header *header = (First_Header*)file.data;
                            
                            if(should_swap_endian(shared_header->flags)) {
                                header->file_name_length = swap_endian_u16(header->file_name_length);
                            }
                            
                            char *file_name_pos = (char*)file.data + file.length;
                            
                            bundle->out_file_name = make_string(64);
                            
                            append_string(&bundle->out_file_name, source_path);
                            
                            {
                                i32 index = find_index_of_last(bundle->out_file_name, '/');
                                
                                if(index >= 0) {
                                    index = bundle->out_file_name.length - index - 1;
                                    bundle->out_file_name.length -= index;
                                } else {
                                    index = find_index_of_last(bundle->out_file_name, '\\');
                                    
                                    if(index >= 0) {
                                        index = bundle->out_file_name.length - index - 1;
                                        bundle->out_file_name.length -= index;
                                    }
                                }
                            }
                            
                            append_cstring(&bundle->out_file_name, UNPACK_NTSTRING("merged_output/"));
                            
                            if(os_read_file(&file, file_handle, header->file_name_length) > 0) {
                                append_cstring(&bundle->out_file_name,
                                               file_name_pos, header->file_name_length);
                                
                                null_terminate(&bundle->out_file_name);
                            }
                        }
                        
                        append_file_name(bundle, arg, shared_header->file_index);
                    } else {
                        printf("%s has an invalid header\n", arg.data);
                    }
                }
                
                SPLTMRG_FREE(file.data);
            }
            
            os_close_file(file_handle);
        } else {
            printf("%s is not a split file\n", arg.data);
        }
	}
    
    if(master_list.count > 0) {
        File_Data file_buffer = make_file_data(SPLITMERGE_NITRO_FILE_LIMIT);
        File_Data header_data = make_file_data(sizeof(First_Header));
        File_Data read_data   = {0};
        
        For(i32, bundle_index, master_list.count) {
            printf("---===##===---\n");
            
            Merge_Bundle *bundle = master_list.data + bundle_index;
            
            File_Handle dest_handle = os_open_file_for_writing(bundle->out_file_name.data);
            
            if(dest_handle) {
                For(u32, file_index, bundle->file_count) {
                    File_Handle source_handle = os_open_file_for_reading(bundle->files[file_index].data);
                    
                    header_data.length = 0;
                    
                    if(source_handle) {
                        if(file_index == 0) {
                            if(os_read_file(&header_data, source_handle, sizeof(First_Header)) > 0)
                            {
                                First_Header *header = (First_Header*)header_data.data;
                                
                                u16 file_name_length = header->file_name_length;
                                if(should_swap_endian(header->shared.flags)) {
                                    file_name_length = swap_endian_u16(file_name_length);
                                }
                                
                                os_move_file_pointer(source_handle, file_name_length + 1);
                            }
                        } else {
                            os_move_file_pointer(source_handle, sizeof(Shared_Header));
                        }
                        
                        i64 remaining_length = os_get_remaining_size_of_file(source_handle);
                        while(remaining_length > 0) {
                            read_data.length   = 0;
                            read_data.data     = file_buffer.data     + file_buffer.length;
                            read_data.capacity = file_buffer.capacity - file_buffer.length;
                            
                            if(os_read_file(&read_data, source_handle, read_data.capacity)) {
                                file_buffer.length += read_data.length;
                                
                                if(file_buffer.length == file_buffer.capacity) {
                                    printf("Merging file %d/%d - chunk %u/%u\n",
                                           bundle_index + 1, master_list.count,
                                           file_index, bundle->file_count);
                                    
                                    os_write_file(dest_handle, file_buffer.data, file_buffer.length);
                                    
                                    file_buffer.length = 0;
                                }
                            }
                            
                            remaining_length = os_get_remaining_size_of_file(source_handle);
                        }
                    }
                    
                    os_close_file(source_handle);
                }
                
                if(file_buffer.length > 0) {
                    printf("Merging file %d/%d - chunk %u/%u\n",
                           bundle_index + 1, master_list.count,
                           bundle->file_count, bundle->file_count);
                    
                    os_write_file(dest_handle, file_buffer.data, file_buffer.length);
                    
                    file_buffer.length = 0;
                }
            }
            
            os_close_file(dest_handle);
        }
        
        SPLTMRG_FREE(file_buffer.data);
        SPLTMRG_FREE(header_data.data);
    }
    
	return 0;
}
