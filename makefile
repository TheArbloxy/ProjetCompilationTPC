# Nom de l'exécutable
EXEC = bin/tpcc

# Dossiers
SRC = src
OBJ = obj
BIN = bin

# Fichiers sources
LEX = $(SRC)/tpc-2025-2026.lex
YACC = $(SRC)/tpc-2025-2026.y
TREE_C = $(SRC)/tree.c
TREE_H = $(SRC)/tree.h

SYMB_C = $(SRC)/symbol_table.c
SYMB_H = $(SRC)/symbol_table.h
SHANDLER_C = $(SRC)/symbol_handler.c
SHANDLER_H = $(SRC)/symbol_handler.h
NASM_HANDLER_C = $(SRC)/nasm_handler.c
NASM_HANDLER_H = $(SRC)/nasm_handler.h

# Fichiers générés
LEX_C = $(OBJ)/lex.yy.c
YACC_C = $(OBJ)/tpc-2025-2026.tab.c
YACC_H = $(OBJ)/tpc-2025-2026.tab.h

# Objets
OBJS = \
	$(OBJ)/tpc-2025-2026.tab.o \
	$(OBJ)/lex.yy.o \
	$(OBJ)/tree.o \
	$(OBJ)/symbol_table.o \
	$(OBJ)/symbol_handler.o \
	$(OBJ)/nasm_handler.o

# Compilateur
CC = gcc
CFLAGS = -Wall -g -I$(SRC) -I$(OBJ)

# Règle par défaut
all: $(BIN) $(OBJ) $(EXEC)

# Création des dossiers si absents
$(BIN) $(OBJ):
	mkdir -p $@

# Édition de liens
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lfl

# Compilation des .c en .o
$(OBJ)/%.o: $(OBJ)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/tree.o: $(TREE_C) $(TREE_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/symbol_table.o: $(SYMB_C) $(SYMB_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/symbol_handler.o: $(SHANDLER_C) $(SHANDLER_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/nasm_handler.o: $(NASM_HANDLER_C) $(NASM_HANDLER_H)
	$(CC) $(CFLAGS) -c $< -o $@

# Bison
$(YACC_C) $(YACC_H): $(YACC)
	bison -d -o $(YACC_C) $<

# Flex
$(LEX_C): $(LEX) $(YACC_H)
	flex -o $@ $<

# Nettoyage
clean:
	rm -f $(LEX_C) $(YACC_C) $(YACC_H) $(OBJS) $(EXEC)

.PHONY: all clean
