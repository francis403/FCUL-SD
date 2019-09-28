#Grupo 13
# Francisco de Almeida Araujo, 45701
# Nuno Burnay, 46406
# Robin Vassantlal, 46408

INC_DIR = include
OBJ_DIR = obj
SRC_DIR = src
GRP_DIR = grupo13

CC = gcc
CCFLAGS = -Wall #-g -ggdb

main: clean create_structure client server

testes: create_structure test_data test_entry test_list test_table test_message

#------------------------ Network ------------------------#

server: data.o entry.o list.o table.o message.o network_client.o table_skel.o setup_file.o primary_backup.o table-server.o
	$(CC) -pthread $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/message.o $(OBJ_DIR)/table.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/setup_file.o $(OBJ_DIR)/primary_backup.o $(OBJ_DIR)/table-server.o -o server

client: data.o entry.o list.o table.o message.o network_client.o table-client.o client_stub.o
	$(CC) $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o $(OBJ_DIR)/message.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/table-client.o $(OBJ_DIR)/client_stub.o -o client

#------------------------ Tests ------------------------#

test_message: data.o entry.o list.o table.o message.o test_message.o
	$(CC) $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o $(OBJ_DIR)/message.o $(OBJ_DIR)/test_message.o -o test_message 
	
test_table: data.o entry.o list.o table.o test_table.o
	$(CC) $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o $(OBJ_DIR)/test_table.o -o test_table

test_list: data.o entry.o list.o test_list.o
	$(CC) $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/test_list.o -o test_list

test_entry: data.o entry.o test_entry.o
	$(CC) $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/test_entry.o -o test_entry

test_data: data.o test_data.o
	$(CC) $(OBJ_DIR)/data.o $(OBJ_DIR)/test_data.o -o test_data

#------------------------ Tests (.o) ------------------------#

test_message.o: test_message.c
	$(CC) $(CCFLAGS) -c test_message.c -o $(OBJ_DIR)/test_message.o -I $(INC_DIR)/message -I $(INC_DIR)/table

test_table.o: test_table.c
	$(CC) $(CCFLAGS) -c test_table.c -o $(OBJ_DIR)/test_table.o -I $(INC_DIR)/table

test_list.o: test_list.c
	$(CC) $(CCFLAGS) -c test_list.c -o $(OBJ_DIR)/test_list.o -I $(INC_DIR)/table

test_entry.o: test_entry.c
	$(CC) $(CCFLAGS) -c test_entry.c -o $(OBJ_DIR)/test_entry.o -I $(INC_DIR)/table

test_data.o: test_data.c
	$(CC) $(CCFLAGS) -c test_data.c -o $(OBJ_DIR)/test_data.o -I $(INC_DIR)/table

#------------------------- Stubs (.o) -------------------------#

table_skel.o: $(SRC_DIR)/table_skel.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table_skel.c -o $(OBJ_DIR)/table_skel.o -I $(INC_DIR) -I $(INC_DIR)/table -I $(INC_DIR)/message

client_stub.o: $(SRC_DIR)/client_stub.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/client_stub.c -o $(OBJ_DIR)/client_stub.o -I $(INC_DIR) -I $(INC_DIR)/table -I $(INC_DIR)/message -I $(INC_DIR)/table_client

#------------------------ Network (.o) ------------------------#

table-server.o: $(SRC_DIR)/table_server/table-server.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table_server/table-server.c -o $(OBJ_DIR)/table-server.o -I $(INC_DIR) -I $(INC_DIR)/message -I $(INC_DIR)/table -I $(INC_DIR)/table_client

table-client.o: $(SRC_DIR)/table_client/table-client.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table_client/table-client.c -o $(OBJ_DIR)/table-client.o -I $(INC_DIR) -I $(INC_DIR)/message -I $(INC_DIR)/table -I $(INC_DIR)/table_client

primary_backup.o: $(SRC_DIR)/primary_backup.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/primary_backup.c -o $(OBJ_DIR)/primary_backup.o -I $(INC_DIR) -I $(INC_DIR)/table_client -I $(INC_DIR)/message -I $(INC_DIR)/table

network_client.o: $(SRC_DIR)/table_client/network_client.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table_client/network_client.c -o $(OBJ_DIR)/network_client.o -I $(INC_DIR) -I $(INC_DIR)/message -I $(INC_DIR)/table_client -I $(INC_DIR)/table

message.o: $(SRC_DIR)/message/message.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/message/message.c -o $(OBJ_DIR)/message.o -I $(INC_DIR)/message -I $(INC_DIR)/table

#------------------------ Table (.o) ------------------------#

table.o: $(SRC_DIR)/table/table.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table/table.c -o $(OBJ_DIR)/table.o -I $(INC_DIR)/table

list.o: $(SRC_DIR)/table/list.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table/list.c -o $(OBJ_DIR)/list.o -I $(INC_DIR)/table

entry.o: $(SRC_DIR)/table/entry.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table/entry.c -o $(OBJ_DIR)/entry.o -I $(INC_DIR)/table

data.o: $(SRC_DIR)/table/data.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/table/data.c -o $(OBJ_DIR)/data.o -I $(INC_DIR)/table
	
#------------------------ Outros (.o) ------------------------#

setup_file.o: $(SRC_DIR)/setup_file.c
	$(CC) $(CCFLAGS) -c $(SRC_DIR)/setup_file.c -o $(OBJ_DIR)/setup_file.o -I $(INC_DIR)

#------------------------ Gestao ------------------------#

create_structure:
	mkdir -p include obj src

clean: cleanObj cleanTemp cleanExec
	rm -f grupo13.txt

cleanExec:
	rm -f test_data test_entry test_list test_table test_message client server

cleanObj:
	rm -f $(OBJ_DIR)/*.o

cleanTemp:
	rm -f ./*~ $(OBJ_DIR)/*~ $(SRC_DIR)/*~ $(SRC_DIR)/message/*~ $(SRC_DIR)/table/*~ $(SRC_DIR)/table_client/*~ $(SRC_DIR)/table_server/*~ $(INC_DIR)/*~ $(INC_DIR)/message/*~ $(INC_DIR)/table/*~ $(INC_DIR)/table_client/*~

create_zip: 
	mkdir -p $(GRP_DIR) $(GRP_DIR)/src $(GRP_DIR)/src/message $(GRP_DIR)/src/table $(GRP_DIR)/src/table_client $(GRP_DIR)/src/table_server $(GRP_DIR)/include $(GRP_DIR)/include/message $(GRP_DIR)/include/table $(GRP_DIR)/include/table_client
	cp -r $(SRC_DIR)/message/*.c $(GRP_DIR)/src/message
	cp -r $(SRC_DIR)/table/*.c $(GRP_DIR)/src/table
	cp -r $(SRC_DIR)/table_client/*.c $(GRP_DIR)/src/table_client
	cp -r $(SRC_DIR)/table_server/*.c $(GRP_DIR)/src/table_server
	cp -r $(SRC_DIR)/*.c $(GRP_DIR)/src
	cp -r $(INC_DIR)/message/*.h $(GRP_DIR)/include/message
	cp -r $(INC_DIR)/table/*.h $(GRP_DIR)/include/table
	cp -r $(INC_DIR)/table_client/*.h $(GRP_DIR)/include/table_client
	cp -r $(INC_DIR)/*.h $(GRP_DIR)/include
	cp -r README.txt makefile $(GRP_DIR) 
	zip -r SD.zip  $(GRP_DIR) -o
	rm -rf $(GRP_DIR)
