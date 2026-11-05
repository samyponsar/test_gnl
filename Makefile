SRCS := ../get_next_line.c ../get_next_line_utils.c test_gnl.c
HDRS := ../get_next_line.h
SIZES := 1 42 8192 9999 10000000
OBJS := $(foreach s,$(SIZES),$(s)_get_next_line.o $(s)_get_next_line_utils.o $(s)_test_gnl.o)
BINS := $(addprefix gnl_,$(SIZES))
CC := cc
CFLAGS := -Wall -Wextra -Werror

all: $(BINS)

%_get_next_line.o: ../get_next_line.c $(HDRS)
	$(CC) $(CFLAGS) -D BUFFER_SIZE=$* -c $< -o $@

%_get_next_line_utils.o: ../get_next_line_utils.c $(HDRS)
	$(CC) $(CFLAGS) -D BUFFER_SIZE=$* -c $< -o $@

%_test_gnl.o: test_gnl.c $(HDRS)
	$(CC) $(CFLAGS) -D BUFFER_SIZE=$* -c $< -o $@

$(BINS): gnl_%: %_get_next_line.o %_get_next_line_utils.o %_test_gnl.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(BINS)

re: fclean all

.PHONY: all clean fclean re
