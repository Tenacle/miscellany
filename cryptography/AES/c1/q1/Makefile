CC = clang
CFLAGS = -g -Wall

# TODO: rename the target to something meaningful!
TARGET = q1

OBJDIR = object
HDRS = $(wildcard *.h)
SRCS = $(wildcard *.c)
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

.PHONY: all
all: $(TARGET)

# WARNING: *must* have a tab before each definition
$(TARGET): $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(OBJDIR)/%.o: %.c $(HDRS) $(OBJDIR)
	$(CC) $(CFLAGS) -c $*.c -o object/$*.o

$(OBJDIR):
	mkdir $(OBJDIR)

.PHONY: clean
clean:
	rm -rf $(TARGET) object
