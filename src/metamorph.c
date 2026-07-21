#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

int metamorph_bin(const char *path)
{
    int	fd = open(path, O_RDWR);
    if (fd == -1)
    {
        return (-1);
    }

    struct stat	stats;
    if (fstat(fd, &stats) == -1)
    {
        close(fd);
        return (-1);
    }

    size_t	size = stats.st_size;
    void	*map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        close(fd);
        return (-1);
    }

    uint8_t random_junk0[8] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    uint8_t random_junk1[8] = {0x50, 0x58, 0x51, 0x59, 0x90, 0x90, 0x90, 0x90};
    uint8_t random_junk2[8] = {0x53, 0x5B, 0x52, 0x5A, 0x90, 0x90, 0x90, 0x90};
    uint8_t random_junk3[8] = {0x41, 0x57, 0x41, 0x5F, 0x90, 0x90, 0x90, 0x90};
    uint8_t random_junk4[8] = {0x87, 0xC0, 0x87, 0xC0, 0x90, 0x90, 0x90, 0x90};

    uint8_t  *mutations[5] = {
        random_junk0,
        random_junk1,
        random_junk2,
        random_junk3,
        random_junk4
    };

    struct timespec	ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        munmap(map, size);
        close(fd);
		return (-1);
    }

    uint64_t    seed = ts.tv_nsec + ts.tv_sec; 

    srand(seed);

    size_t  i = 0;
    while (i < size)
    {
        uint8_t  *addr = &((uint8_t*)map)[i];

        for (int mut = 0; mut < 5; mut++)
        {
            if (i + 8 <= size &&
                memcmp(addr, mutations[mut], 8) == 0)
            {
                int rdm = rand() % 5;
                memcpy(addr, mutations[rdm], 8);

                i += 8 - 1;
                break ;
            }
        }
        i++;
    }

    munmap(map, size);
    close(fd);
    return (0);
}
