/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   metamorph.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pboucher <pboucher@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 18:53:05 by pboucher          #+#    #+#             */
/*   Updated: 2026/08/18 19:33:03 by pboucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#define _GNU_SOURCE
#include "death.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static t_mutation g_instruction_mutations[] = {
    {{0x48, 0x31, 0xC0}, 3,
     {0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00}, 7,
     "xor_mov"},
    
    {{0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00}, 7,
     {0x6A, 0x01, 0x58, 0x90, 0x90, 0x90, 0x90}, 7,
     "mov_pushpop"},
    
    {{0x48, 0x83, 0xC0, 0x01}, 4,
     {0x48, 0xFF, 0xC0, 0x90}, 4,
     "add_inc"},
    
    {{0x48, 0x83, 0xE8, 0x01}, 4,
     {0x48, 0xFF, 0xC8, 0x90}, 4,
     "sub_dec"},
    
    {{0x48, 0x85, 0xC0}, 3,
     {0x48, 0x83, 0xF8, 0x00}, 4,
     "test_cmp"},
    
    {{0x48, 0x8D, 0x45, 0xFC}, 4,
     {0x48, 0x8B, 0x45, 0xFC}, 4,
     "lea_mov"},
};

static t_junk g_junk_sequences[] = {
    {{0x50, 0x58, 0x51, 0x59, 0x52, 0x5A, 0x90, 0x90}, 8},
    {{0x48, 0x31, 0xC0, 0x48, 0x31, 0xC0, 0x90, 0x90}, 8},
    {{0x48, 0x83, 0xC0, 0x00, 0x48, 0x83, 0xE8, 0x00}, 8},
    {{0x48, 0x87, 0xC0, 0x48, 0x87, 0xC0, 0x90, 0x90}, 8},
    {{0x48, 0xFF, 0xC0, 0x48, 0xFF, 0xC8, 0x90, 0x90}, 8},
};

void mutate_instructions(uint8_t *data, size_t size)
{
    size_t num_mutations = sizeof(g_instruction_mutations) / sizeof(g_instruction_mutations[0]);
    size_t mutations_applied = 0;
    
    for (size_t i = 0; i < size - 16; i++) {
        for (size_t j = 0; j < num_mutations; j++) {
            if (memcmp(&data[i], g_instruction_mutations[j].orig, 
                      g_instruction_mutations[j].orig_len) == 0) {
                size_t idx = rand() % num_mutations;
                memcpy(&data[i], g_instruction_mutations[idx].repl, 
                       g_instruction_mutations[idx].repl_len);
                mutations_applied++;
                i += g_instruction_mutations[idx].repl_len;
                break;
            }
        }
    }
}

void insert_junk_code(uint8_t *data, size_t *size)
{
    size_t num_junk = sizeof(g_junk_sequences) / sizeof(g_junk_sequences[0]);
    size_t insert_positions = 5 + rand() % 10;
    
    for (size_t i = 0; i < insert_positions && *size > 20; i++) {
        size_t pos = rand() % (*size - 16);
        size_t idx = rand() % num_junk;
        if (pos < 4096) continue;
        
        size_t junk_len = g_junk_sequences[idx].len;
        memmove(&data[pos + junk_len], &data[pos], *size - pos);
        memcpy(&data[pos], g_junk_sequences[idx].bytes, junk_len);
        *size += junk_len;
    }
}

int metamorph_bin(const char *path)
{
    if (path == NULL)
        return (-1);
    
    struct stat stats;
    if (stat(path, &stats) == -1)
        return (-1);
    
    int fd = open(path, O_RDWR);
    if (fd == -1)
        return (-1);
    
    void *map = mmap(NULL, stats.st_size, PROT_READ | PROT_WRITE, 
                     MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        return (-1);
    }
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    srand((unsigned int)(ts.tv_nsec ^ ts.tv_sec ^ (ts.tv_nsec >> 32)));
    
    uint8_t *data = (uint8_t *)map;
    size_t size = stats.st_size;
    
    mutate_instructions(data, size);

    munmap(map, stats.st_size);
    close(fd);
    
    return (0);
}