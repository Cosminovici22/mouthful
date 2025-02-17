MFAPI_SRC_DIR := src/mfapi
MFGEN_SRC_DIR := src/mfgen

.PHONY: all mfapi mfgen clean mfapi.clean mfgen.clean

all: mfapi mfgen

mfapi:
	$(MAKE) -C $(MFAPI_SRC_DIR)

mfgen:
	$(MAKE) -C $(MFGEN_SRC_DIR)

clean: mfapi.clean mfgen.clean

mfapi.clean:
	$(MAKE) -C $(MFAPI_SRC_DIR) clean

mfgen.clean:
	$(MAKE) -C $(MFGEN_SRC_DIR) clean
