PYTHON ?= "python"

TEST_SCRIPTS = $(wildcard core/*/run.py)

$(TEST_SCRIPTS):
	cd $(dir $@) && PYTHONPATH=../../framework/ $(PYTHON) run.py

test: $(TEST_SCRIPTS)

.PHONY: $(TEST_SCRIPTS) test
