CC      = $(CROSS_COMPILE)gcc
AR      = $(CROSS_COMPILE)ar
RANLIB  = $(CROSS_COMPILE)ranlib

PROGS   = system_manager

RM      = rm -f
RM_CMD  = $(RM) *.BAK *.bak *.d *.o *.so ,* *~ *.a *.orig *.rej *.out $(PROGS)

# Libraries
ifdef BUILD_NOLIBM
  LIBS =
else
  LIBS = -lm
endif

# Stripping
ifdef BUILD_STRIPPING
  STRIPFLAGS = -Wl,-s
else
  STRIPFLAGS =
endif

# Debug
ifdef BUILD_DEBUG
  CFLAGS += -g
endif

# ==========================
# Compile flags
# ==========================
CFLAGS += -I. \
    -I$(STAGING_DIR)/usr/include \
    -I$(STAGING_DIR)/usr/include/libubox \
    -I../../libgps/src \
    -I../../libcheck_tx_power/src \
    -O2 -Wall -pthread   # pthread 포함

# ==========================
# Linker flags
# ==========================
LDFLAGS += -L$(STAGING_DIR)/usr/lib \
    $(STRIPFLAGS) \
    -lhpa \
    -lcheck_tx_power \
    -lgps \
    -ltemp \
    -lubus \
    -lblobmsg_json \
    -lubox -ljson-c \
    -lpthread          # pthread 링크

# ==========================
# Object files
# ==========================
SYSTEM_MANAGER_OBJS = \
	system_manager.o \
	master.o \
	slave.o \
	slave_module/alive_client.o \
	slave_module/image_client.o \
	master_module/image_server.o \
  master_module/alive_server.o \
  config/config_reader.o \
  config/uci_reader.o \
  config/uci_cmd.o \
	ubus/ubus.o

# ==========================
# Targets
# ==========================
all: $(PROGS)

# 링크
system_manager: $(SYSTEM_MANAGER_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

# Compile
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	$(RM_CMD)

realclean: clean
	$(RM) $(PROGS)
