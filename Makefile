OUT = akucc
SOURCE_DIR = src
BIN_DIR = bin
BUILD_DIR = build
HEADERS = $(wildcard $(SOURCE_DIR)/include/*.hpp)
SOURCES = $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.cpp=.o)))
CC = g++
OUTCAP = $(shell echo '$(OUT)' | tr '[:lower:]' '[:upper:]')
CFLAGS = -g -O0 -Isrc/include

$(BIN_DIR)/$(OUT): $(OBJECTS)
	@printf "%8s %-40s %s\n" $(CC) $@ "$(CFLAGS)"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(HEADERS)
	@printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@mkdir -p $(BUILD_DIR)/
	@$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -r bin
	rm -r build