CC = clang
CFLAGS = -g -Wall

# TODO: rename the target to something meaningful!
TARGET = q1

OBJDIR = object
HDRS = $(wildcard *.h)
SRCS = $(wildcard *.c)
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
LNKS = $(wildcard *.o)

.PHONY: all
all: $(TARGET)

# WARNING: *must* have a tab before each definition
$(TARGET): $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) -lpthread $(OBJS) $(LNKS) -o $(TARGET)

$(OBJDIR)/%.o: %.c $(HDRS) $(OBJDIR)
	$(CC) $(CFLAGS) -c $*.c -o object/$*.o

$(OBJDIR):
	mkdir $(OBJDIR)

.PHONY: print
print:
	@echo 
	@echo $(CC) $(CFLAGS) -lpthread $(OBJS) -o $(TARGET)
	@echo $(CC) $(CFLAGS) -c $*.c -o object/$*.o

.PHONY: clean
clean:
	rm -rf $(TARGET) object
