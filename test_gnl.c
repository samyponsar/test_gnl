#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../get_next_line.h"

typedef struct s_case
{
	const char	*what;
	const char	*path;
}	t_case;

char	*ref_getline(FILE *file, char **buf, size_t *cap)
{
	if (getline(buf, cap, file) == -1)
		return (NULL);
	return (*buf);
}

void	fail_diff(const t_case *c, size_t i, char *gnl, char *line)
{
	printf("%-24s\tKO\n", c->what);
	printf("  %s: line %zu differs\n", c->path, i);
	printf("  get_next_line : %s\n", gnl ? gnl : "(NULL)");
	printf("  getline       : %s", line ? line : "(NULL)\n");
	exit(1);
}

/* Reads one file twice: once through get_next_line(fd), once through getline
   on a separate FILE stream. Two independent opens mean two independent file
   offsets, so neither reader can steal bytes from the other. */
void	test_differential(const t_case *c)
{
	int		fd;
	FILE	*file;
	char	*gnl;
	char	*line;
	char	*buf;
	size_t	cap;
	size_t	i;

	fd = open(c->path, O_RDONLY);
	file = fdopen(open(c->path, O_RDONLY), "r");
	if (fd < 0 || file == NULL)
		exit(1);
	buf = NULL;
	cap = 0;
	i = 0;
	while (1)
	{
		gnl = get_next_line(fd);
		line = ref_getline(file, &buf, &cap);
		if ((gnl == NULL) != (line == NULL))
			fail_diff(c, i, gnl, line);
		if (gnl == NULL)
			break ;
		if (strcmp(gnl, line) != 0)
			fail_diff(c, i, gnl, line);
		free(gnl);
		i++;
	}
	assert(get_next_line(fd) == NULL);
	assert(ref_getline(file, &buf, &cap) == NULL);
	free(buf);
	close(fd);
	fclose(file);
}

void	check_line(char *gnl, const char *want)
{
	if ((gnl == NULL) != (want == NULL) || (gnl && strcmp(gnl, want) != 0))
	{
		printf("  get_next_line : %s\n", gnl ? gnl : "(NULL)");
		printf("  expected      : %s\n", want ? want : "(NULL)");
		exit(1);
	}
	free(gnl);
}

/* Two files open at once, calls interleaved: leftovers must never leak from
   one node to the other, and a drained fd must keep returning NULL. */
void	test_two_fds(void)
{
	int	fd1;
	int	fd2;

	fd1 = open("files/short.txt", O_RDONLY);
	fd2 = open("files/short2.txt", O_RDONLY);
	check_line(get_next_line(fd1), "line 1\n");
	check_line(get_next_line(fd2), "line 4\n");
	check_line(get_next_line(fd1), "line 2\n");
	check_line(get_next_line(fd2), "line 5\n");
	check_line(get_next_line(fd1), "line 3\n");
	check_line(get_next_line(fd1), NULL);
	check_line(get_next_line(fd1), NULL);
	check_line(get_next_line(fd2), "line 6");
	check_line(get_next_line(fd1), NULL);
	check_line(get_next_line(fd2), NULL);
	check_line(get_next_line(fd1), NULL);
	close(fd1);
	close(fd2);
}

/* Milliseconds on a monotonic clock, only ever used as a difference. */
long	monotonic_ms(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int	main(void)
{
	printf("\n");
	printf("\n");
	printf("BUFFER_SIZE=%d\n", BUFFER_SIZE);
	printf("\n");
	const t_case	cases[] = {
		{"Empty file", "files/empty.txt"},
		{"Single newline", "files/single_newline.txt"},
		{"Ten empty lines", "files/multiple_newlines.txt"},
		{"Line without newline", "files/no_newline.txt"},
		{"Three lines", "files/short.txt"},
		{"Three lines, no final newline", "files/short2.txt"},
		{"Windows format (CRLF)", "files/crlf.txt"},
		{"Line of 8191 bytes", "files/8191.txt"},
		{"Line of 8192 bytes", "files/8192.txt"},
		{"Line of 8193 bytes", "files/8193.txt"},
		{"100 KiB line, no newline", "files/huge_line.txt"},
		{"Candide de Voltaire", "files/candide.txt"},
		{"Binary, 4 NUL bytes", "files/random.bin"},
	};
	size_t	i;
	long	start;

	i = 0;
	while (i < sizeof(cases) / sizeof(*cases))
	{
		start = monotonic_ms();
		test_differential(&cases[i]);
		printf("%-24s\tOK %3ld ms\n", cases[i].what, monotonic_ms() - start);
		i++;
	}
	start = monotonic_ms();
	test_two_fds();
	printf("%-24s\tOK %3ld ms\n", "Two files, alternating", monotonic_ms() - start);
	return (0);
}
