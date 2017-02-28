include common.mk

include utilities/submake.mk
include mm/submake.mk
include jvm/submake.mk
include jvmclassfile/submake.mk

.PHONY: clean run

SRC+=main.cc

TMP_OBJ=$(patsubst %.cc,$(BIN_DIR)/%.o,$(SRC))
APP_OBJ=$(patsubst %.S,$(BIN_DIR)/%.o,$(TMP_OBJ))


include rules.mk
