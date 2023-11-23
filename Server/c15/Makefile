INCLUDE = -I ./*.h
TARGET1 = pro_pool_CGI

.PHONY: all
all: $(TARGET1) 
$(TARGET1) : $(TARGET1).o
	$(GCC) -o 

#build object
%.o : %.c
	$(GCC) -c $< -o $(INC_CLUDE)

.PHONY: clean
clean:
	-@rm -f $(TARGET1) $(TARGET1).o
