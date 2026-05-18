###########
# General #
###########
RM = -rm -f


#########
# Flags #
#########
CFLAGS = -std=c11


###############
# Directories #
###############
DIRS =
DIRS += cli
DIRS += codegen
DIRS += dasm
DIRS += program
DIRS += PROGRAm-loader
DIRS += run
DIRS += testcases
DIRS += vm


#########
# Paths #
#########
MAIN_C = main.c
MAIN_O = main.o
MAIN_D = main.d

EXEC = vmr


#######################
# Generated Variables #
#######################
SRCS = $(MAIN_C)  $(wildcard $(DIRS:%=%/*.c))
DEPS = $(MAIN_D)  $(SRCS:%.c=%.d)
OBJS = $(MAIN_O)  $(SRCS:%.c=%.o)


###########
# Recipes #
###########
all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^


%.d: %.c
	$(CC) $(CFLAGS) -MM -MP -MF $@ $<


clean:
	$(RM) $(OBJS) $(DEPS) $(EXEC)


-include $(DEPS)
