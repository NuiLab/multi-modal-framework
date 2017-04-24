DIR=.
INC=$(DIR)/include
SRC=$(DIR)/src
OBJ=./obj
ARG=-std=c++11 -fpermissive -o

clean:
	find $(DIR) -name '*.o' -delete
	find $(DIR) -name '*~' -delete
	find $(DIR) -name '*.sw*' -delete
