# name of your application
APPLICATION = default

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Uncomment these lines if you want to use platform support from external
# repositories:
#RIOTCPU ?= $(CURDIR)/../../RIOT/thirdparty_cpu
#RIOTBOARD ?= $(CURDIR)/../../RIOT/thirdparty_boards

# Uncomment this to enable scheduler statistics for ps:
#CFLAGS += -DSCHEDSTATISTICS

# If you want to use native with valgrind, you should recompile native
# with the target all-valgrind instead of all:
# make -B clean all-valgrind

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += config
# include and auto-initialize all available sensors
USEMODULE += saul_reg
USEMODULE += saul_default
USEMODULE += auto_init_saul

BOARD_PROVIDES_NETIF := airfy-beacon fox iotlab-m3 iotlab-a8-m3 mulle native nrf51dongle \
	nrf6310 pba-d-01-kw2x pca10000 pca10005 saml21-xpro samr21-xpro spark-core \
	yunjia-nrf51822

ifneq (,$(filter $(BOARD),$(BOARD_PROVIDES_NETIF)))
  # Use modules for networking
  # gnrc is a meta module including all required, basic gnrc networking modules
  USEMODULE += gnrc
  # use the default network interface for the board
  USEMODULE += gnrc_netdev_default
  # automatically initialize the network interface
  USEMODULE += auto_init_gnrc_netif
  # the application dumps received packets to stdout
  USEMODULE += gnrc_pktdump

  # We use only the lower layers of the GNRC network stack, hence, we can
  # reduce the size of the packet buffer a bit
  CFLAGS += -DGNRC_PKTBUF_SIZE=512
endif

FEATURES_OPTIONAL += config
FEATURES_OPTIONAL += periph_rtc

ifneq (,$(filter msb-430,$(BOARD)))
  USEMODULE += sht11
endif
ifneq (,$(filter msba2,$(BOARD)))
  USEMODULE += sht11
  USEMODULE += mci
  USEMODULE += random
endif

# Set a custom 802.15.4 channel if needed
DEFAULT_CHANNEL ?= 26
CFLAGS += -DDEFAULT_CHANNEL=$(DEFAULT_CHANNEL)

include $(RIOTBASE)/Makefile.include
