############
# Commands #
############
RM = -rm -f


#########
# Flags #
#########
CFLAGS = -std=c11


#########
# Paths #
#########
VM_O = vm.o
VM_MAIN_O = vm-main.o
EXEC = vm


###########
# Recipes #
###########
$(EXEC): $(VM_O) $(VM_MAIN_O)
	$(CC) $(CFLAGS) -o $@ $^


clean:
	$(RM) $(VM_O) $(MAIN_O) $(EXEC)
