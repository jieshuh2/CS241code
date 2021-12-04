/**
 * finding_filesystems
 * CS 241 - Spring 2021
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string, "Free blocks: %zd\n"
                            "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    inode* node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        // perror("chmod no path:");
        return -1;
    }
    // int type = new_permissions >> RWX_BITS_NUMBER
    // int bit_mask = 1023;
    int type = node->mode >> RWX_BITS_NUMBER;
    node->mode = (type << RWX_BITS_NUMBER) | new_permissions;
    clock_gettime(CLOCK_REALTIME, &(node->ctim));
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        // perror("chown no path:");
        return -1;
    }
    if (owner != ((uid_t)-1)) {
        node->uid = owner;
        clock_gettime(CLOCK_REALTIME, &(node->ctim));
    }
    if (group != ((uid_t)-1)) {
        node->gid = group;
        clock_gettime(CLOCK_REALTIME, &(node->ctim));
    }
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode* nody = get_inode(fs, path);
    if (nody) {
        return NULL;
    }
    inode_number num = first_unused_inode(fs); 
    inode* init = fs->inode_root + num;
    const char* filename = NULL;//wait const
    inode*parent = parent_directory(fs, path, &filename);
    if (valid_filename(filename) && is_directory(parent)) {
        init_inode(parent, init);
        size_t blocknum = parent->size / sizeof(data_block);
        size_t idx = parent->size % sizeof(data_block);
        data_block_number datablock = -1;
        if (idx == 0) {
            //this means that there are no half filled block;
            if (blocknum < NUM_DIRECT_BLOCKS) {
                datablock = add_data_block_to_inode(fs, parent);
            }  else {
                if (parent->indirect == -1) {
                    int error = add_single_indirect_block(fs, parent);
                    if (error == -1) {
                        errno = ENOSPC;
                        // perror("fail single block");
                        return NULL;
                    }
                }
                data_block_number *block_array = (data_block_number*)(fs->data_root + parent->indirect);
                datablock = add_data_block_to_indirect_block(fs, block_array);
                if (datablock == -1) {
                    errno = ENOSPC;
                    // perror("cannot add block");
                    return NULL;
                }
            }
        } else {                                                           
            if (blocknum < NUM_DIRECT_BLOCKS) {
                datablock = parent->direct[blocknum];
            } else {
                data_block_number *block_array = (data_block_number*)(fs->data_root + parent->indirect);
                datablock = block_array[blocknum - NUM_DIRECT_BLOCKS];
            }
        }
        char *block = (char *)(fs->data_root + datablock) + idx; //confuse where to start;
        minixfs_dirent node;
        node.inode_num = num;
        node.name = strdup(filename);
        parent->size += FILE_NAME_ENTRY;
        make_string_from_dirent(block, node);
        free(node.name);
        return init;
    }
    return NULL;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        ssize_t numberinused = 0;
        //isn't that out of scope
        char* map = GET_DATA_MAP(fs->meta);
        for (size_t i = 0; i < fs->meta->dblock_count; i++) {
            if (map[i] != 0) {
                numberinused += 1;
            }
        }
        char* file  = block_info_string(numberinused);
        if ((size_t)*off > strlen(file)) {
            // free(file);
            return 0;
        }
        if (count + *off> strlen(file)) {//NULL terminate 
            count = strlen(file) - *off;
        }
        memcpy(buf, file + *off, count);
        *off += count;
        return count;
    }
    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    inode* node = get_inode(fs, path);
    if (node == NULL) {
        node = minixfs_create_inode_for_path(fs, path);
    }
    ssize_t blockstart = (*off)/sizeof(data_block);
    ssize_t shiftstart = (*off)%sizeof(data_block);
    ssize_t remain = (*off + count)%sizeof(data_block);
    ssize_t totalblock = (*off + count)/sizeof(data_block);
    if (remain != 0) {
        totalblock += 1;
    }
    int error = minixfs_min_blockcount(fs, path, totalblock);
    if (error == -1) {
        errno = ENOSPC;
        return -1;
    }
    //make sure threre are enoguh block
    data_block_number* block_array = node->direct;
    ssize_t copy = 0;
    if (totalblock <= NUM_DIRECT_BLOCKS) {
        ssize_t numcpy;
        if (count < sizeof(data_block) - shiftstart) {
            numcpy = count;
        } else {
            numcpy = sizeof(data_block) - shiftstart;
        }
        memcpy(((char*)(fs->data_root + block_array[blockstart])) + shiftstart, buf, numcpy);
        copy += numcpy;
        buf = buf + numcpy;
        count -= numcpy;
        for (int i = blockstart + 1; i < totalblock; i++) {
            if (count < sizeof(data_block)) {
                numcpy = count;
            } else {
                numcpy = sizeof(data_block);
            }
            memcpy(fs->data_root + block_array[i], buf, numcpy);
            copy += numcpy;
            buf = buf + numcpy;
            count -= numcpy;
        }
    } else {
        if (blockstart < NUM_DIRECT_BLOCKS) {
            size_t numcpy;
            if (count < sizeof(data_block) - shiftstart) {
                numcpy = count;
            } else {
                numcpy = sizeof(data_block) - shiftstart;
            }
            memcpy(((char*)(fs->data_root + block_array[blockstart])) + shiftstart, buf, numcpy);
            copy += numcpy;
            buf = buf + numcpy;
            count -= numcpy;
            for (ssize_t i = blockstart + 1; i < NUM_DIRECT_BLOCKS; i++) {
                if (count < sizeof(data_block)) {
                    numcpy = count;
                } else {
                    numcpy = sizeof(data_block);
                }
                memcpy(fs->data_root + block_array[i], buf, numcpy);
                copy += numcpy;
                buf = buf + numcpy;
                count -= numcpy;
            }
            block_array = (data_block_number *)(fs->data_root + node->indirect);
            totalblock -= NUM_DIRECT_BLOCKS;
            for (ssize_t i = 0; i < totalblock; i++) {
                if (count < sizeof(data_block)) {
                    numcpy = count;
                } else {
                    numcpy = sizeof(data_block);
                }
                memcpy(fs->data_root + block_array[i], buf, numcpy);
                copy += numcpy;
                buf = buf + numcpy;
                count -= numcpy;
            }
        } else {
            block_array = (data_block_number *)(fs->data_root + node->indirect);
            blockstart -= NUM_DIRECT_BLOCKS;
            totalblock -= NUM_DIRECT_BLOCKS;
            size_t numcpy;
            if (count < sizeof(data_block) - shiftstart) {
                numcpy = count;
            } else {
                numcpy = sizeof(data_block) - shiftstart;
            }
            memcpy(((char*)(fs->data_root + block_array[blockstart])) + shiftstart, buf, numcpy);
            copy += numcpy;
            buf = buf + numcpy;
            count -= numcpy;
            for (ssize_t i = blockstart + 1; i < totalblock; i++) {
                if (count < sizeof(data_block)) {
                    numcpy = count;
                } else {
                    numcpy = sizeof(data_block);
                }
                memcpy(fs->data_root + block_array[i], buf, numcpy);
                copy += numcpy;
                buf = buf + numcpy;
                count -= numcpy;
            }
        }
    }
    assert(count == 0);
    *off = *off + copy;
    node->size += copy;
    clock_gettime(CLOCK_REALTIME, &node->mtim);
    clock_gettime(CLOCK_REALTIME, &node->atim);
    return copy;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);

    inode* node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        return -1;
    }
    if ((ssize_t)node->size <= (*off)) {
        clock_gettime(CLOCK_REALTIME, &node->atim);
        return 0;
    }
    if (count + *off > node->size) {
        //min of count and node->size - *off
        count = node->size - *off;
    }
    ssize_t blockstart = (*off)/sizeof(data_block);
    ssize_t shiftstart = (*off)%sizeof(data_block);
    ssize_t remain = (*off + count)%sizeof(data_block);
    ssize_t totalblock = (*off + count)/sizeof(data_block);
    if (remain != 0) {
        totalblock += 1;
    }
    data_block_number* block_array = node->direct;
    ssize_t copy = 0;
    if (totalblock <= NUM_DIRECT_BLOCKS) {
        ssize_t numcpy;
        if (count < sizeof(data_block) - shiftstart) {
            numcpy = count;
        } else {
            numcpy = sizeof(data_block) - shiftstart;
        }
        memcpy(buf, ((char*)(fs->data_root + block_array[blockstart])) + shiftstart, numcpy);
        copy += numcpy;
        buf = buf + numcpy;
        count -= numcpy;
        for (int i = blockstart + 1; i < totalblock; i++) {
            if (count < sizeof(data_block)) {
                numcpy = count;
            } else {
                numcpy = sizeof(data_block);
            }
            memcpy(buf, fs->data_root + block_array[i], numcpy);
            copy += numcpy;
            buf = buf + numcpy;
            count -= numcpy;
        }
    } else {
        if (blockstart < NUM_DIRECT_BLOCKS) {
            size_t numcpy;
            if (count < sizeof(data_block) - shiftstart) {
                numcpy = count;
            } else {
                numcpy = sizeof(data_block) - shiftstart;
            }
            memcpy(buf, ((char*)(fs->data_root + block_array[blockstart])) + shiftstart, numcpy);
            copy += numcpy;
            buf = buf + numcpy;
            count -= numcpy;
            for (ssize_t i = blockstart + 1; i < NUM_DIRECT_BLOCKS; i++) {
                if (count < sizeof(data_block)) {
                    numcpy = count;
                } else {
                    numcpy = sizeof(data_block);
                }
                memcpy(buf, fs->data_root + block_array[i], numcpy);
                copy += numcpy;
                buf = buf + numcpy;
                count -= numcpy;
            }
            block_array = (data_block_number *)(fs->data_root + node->indirect);
            totalblock -= NUM_DIRECT_BLOCKS;
            for (ssize_t i = 0; i < totalblock; i++) {
                if (count < sizeof(data_block)) {
                    numcpy = count;
                } else {
                    numcpy = sizeof(data_block);
                }
                memcpy(buf, fs->data_root + block_array[i], numcpy);
                copy += numcpy;
                buf = buf + numcpy;
                count -= numcpy;
            }
        } else {
            block_array = (data_block_number *)(fs->data_root + node->indirect);
            blockstart -= NUM_DIRECT_BLOCKS;
            totalblock -= NUM_DIRECT_BLOCKS;
            size_t numcpy;
            if (count < sizeof(data_block) - shiftstart) {
                numcpy = count;
            } else {
                numcpy = sizeof(data_block) - shiftstart;
            }
            memcpy(buf, ((char*)(fs->data_root + block_array[blockstart])) + shiftstart, numcpy);
            copy += numcpy;
            buf = buf + numcpy;
            count -= numcpy;
            for (ssize_t i = blockstart + 1; i < totalblock; i++) {
                if (count < sizeof(data_block)) {
                    numcpy = count;
                } else {
                    numcpy = sizeof(data_block);
                }
                memcpy(buf, fs->data_root + block_array[i], numcpy);
                copy += numcpy;
                buf = buf + numcpy;
                count -= numcpy;
            }
        }
    }
    //null terminate
    assert(count == 0);
    *off = *off + copy;
    clock_gettime(CLOCK_REALTIME, &node->atim);
    return copy;
}
