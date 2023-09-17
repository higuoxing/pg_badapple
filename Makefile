MODULES = badapple
EXTENSION = badapple
DATA = badapple.txt badapple--1.0.0.sql

PG_CONFIG := pg_config

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
