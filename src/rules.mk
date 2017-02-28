
all: $(APP)

clean:
	rm -f -r $(BIN_DIR)/*

$(APP): $(APP_OBJ)
	$(GCC) -o $(APP) -pthread $(APP_OBJ) $(LDFLAGS)

$(BIN_DIR)/%.o: %.S
	@mkdir -p $(@D)
	$(GCC) -c $(CXXFLAGS) -o $@ $<

$(BIN_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(GCC) -c $(CXXFLAGS) -o $@ $<
