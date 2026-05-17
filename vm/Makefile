###########
# General #
###########
RM = -rm -f


#######
# CLI #
#######
CLI_DIR = cli
CLI_H   = $(CLI_DIR)/cli.h
CLI_C   = $(CLI_DIR)/cli.c


###########
# Codegen #
###########
CODEGEN_DIR = codegen
CODEGEN_H   = $(CODEGEN_DIR)/vm-codegen.h


#######
# Run #
#######
RUN_DIR    = run
RUN_H      = $(RUN_DIR)/run.h
RUN_C      = $(RUN_DIR)/run.c
RUN_CLI_H  = $(RUN_DIR)/cli.h
RUN_CLI_C  = $(RUN_DIR)/cli.c
RUN_PROG_H = $(RUN_DIR)/program.h
RUN_PROG_C = $(RUN_DIR)/program.c
RUN_LOAD_H = $(RUN_DIR)/program-loader.h
RUN_LOAD_C = $(RUN_DIR)/program-loader.c


######
# VM #
######
VM_DIR = vm
VM_H   = $(VM_DIR)/vm.h
VM_C   = $(VM_DIR)/vm.c


##########
# C Code #
##########
MAIN_C = main.c

SRCS_C =
SRCS_C += $(MAIN_C)
SRCS_C += $(CLI_C)
SRCS_C += $(RUN_C)
SRCS_C += $(RUN_CLI_C)
SRCS_C += $(RUN_PROG_C)
SRCS_C += $(RUN_LOAD_C)
SRCS_C += $(VM_C)

OBJS   = $(SRCS_C:%.c=%.o)

EXEC = vmrun


###########
# Recipes #
###########
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^


##########################
# No-Action Dependencies #
##########################
# Main
$(MAIN_C): $(VM_H) $(CODEGEN_H)

# Top-level CLI
$(CLI_C): $(CLI_H) $(RUN_H) $(RUN_CLI_H)

# Run Subsystem
$(RUN_C): $(RUN_H) $(RUN_CLI_H) $(RUN_LOAD_H) $(RUN_PROG_H) $(VM_H)
$(RUN_CLI_C): $(RUN_CLI_H)
$(RUN_PROG_C): $(RUN_PROG_H) $(VM_H)
$(RUN_LOAD_C): $(RUN_CLI_H) $(RUN_LOAD_H) $(RUN_PROG_H) $(VM_H)

# Virtual Machine
$(VM_C): $(VM_H)


clean:
	$(RM) $(OBJS)
	$(RM) $(EXEC)
