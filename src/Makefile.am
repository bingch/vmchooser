
## Process this file with automake to produce Makefile.in

## Created by Anjuta


uidir = $(datadir)/vmchooser/ui
ui_DATA = vmchooser.ui \
	logo.jpg \
	vmchooser.conf


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(VMCHOOSER_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g\
	 -lldap\
	 -llber

bin_PROGRAMS = vmchooser

vmchooser_SOURCES = \
	vmchooser.h \
	vmchooser.c \
	rpm/vmchooser.spec 

vmchooser_LDFLAGS = \
	-Wl,--export-dynamic

vmchooser_LDADD = $(VMCHOOSER_LIBS)


EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(datadir)/vmchooser
